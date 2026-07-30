# Bengal 0.3 Validation

## Scope

This record validates the `0.3` concurrency and platform-support exit gate. It
does not establish application, network, trading, or market performance.

Evidence was produced by
[CI run 30566115260](https://github.com/jaeHbk/bengal/actions/runs/30566115260)
at commit `16eac8def858bfa2fd9c733f4ab019ee462f5c95`.

## Concurrent Pipeline

The sustained test sends 1,000,000 sequential `std::uint64_t` values through a
`bengal::spsc_queue<std::uint64_t, 1024>` from one producer to one consumer.
It verifies:

- every value is consumed once and in order;
- the consumed count is 1,000,000;
- the checksum is `499999500000`;
- the queue is empty after join; and
- the worker completed Bengal's startup handshake.

The same test passed in release builds and under GCC ThreadSanitizer. The TSan
job ran both the unit suite and sustained test with `halt_on_error=1`; the
sustained test completed in 0.55 seconds under instrumentation. That elapsed
time is recorded only to identify the run and is not a throughput baseline.

## Platform Matrix

The hosted run passed:

- GCC 13.3 on Ubuntu 24.04;
- Clang on Ubuntu 24.04;
- Apple Clang on ARM macOS 15;
- AddressSanitizer and UndefinedBehaviorSanitizer;
- GCC ThreadSanitizer;
- bounded short-string and allocator fuzzing;
- installed-package consumers; and
- native, Boost.StaticString, and Google Benchmark baseline jobs.

## Exit-Gate Decisions

- `spsc_queue` is retained as a dependency-free, fixed-capacity SPSC primitive
  with explicit full, ownership, overload, and shutdown contracts.
- Queue waiting and close policy remain application responsibilities.
- Structured thread startup status distinguishes unsupported QoS from setup
  failure without preventing the worker from running.
- Platform capability reporting describes Bengal support and performs no
  runtime probe.
- The concurrent bounded-pipeline example uses an explicit end event and
  accesses consumer-owned results only after join.

## Limitations

- TSan does not prove freedom from all concurrency defects.
- The sustained test covers one producer and one consumer, which is the only
  supported topology.
- CI runners are virtualized and unsuitable for stable latency claims.
- No CPU affinity, frequency control, blocking wait strategy, or real-time
  scheduler is used.
