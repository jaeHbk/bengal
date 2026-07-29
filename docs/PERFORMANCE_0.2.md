# Bengal 0.2 Performance Evidence

## Scope

This report validates Bengal's performance-oriented APIs against their closest
standard or established alternatives. It does not establish application,
network, trading, or market performance.

Evidence was produced by
[CI run 30477395816](https://github.com/jaeHbk/bengal/actions/runs/30477395816)
at commit `6679197b3c1c82732752e84c5c761bf7cf22b5a9`.

## Environment

- Ubuntu 24.04.4 LTS GitHub-hosted virtual machine
- AMD EPYC 9V74, 4 virtual CPUs, Microsoft hypervisor
- GCC 13.3.0
- C++20 release build
- Boost 1.83
- 100,000 base iterations and 1,001 native samples
- Five Google Benchmark repetitions

The CI artifact contains the complete hardware report, native output, Google
Benchmark text, and Google Benchmark JSON.

## Native Harness

Values are percentiles of mean nanoseconds per operation from independent
samples. They are not percentiles of individual-operation latency.

| Workload | p50 ns/op | p95 | p99 | p99.9 | Median ops/s |
|---|---:|---:|---:|---:|---:|
| `type_map/get` | 0.86 | 0.94 | 1.04 | 1.27 | 1.162e9 |
| `std::tuple/get` | 0.70 | 0.78 | 1.13 | 1.39 | 1.435e9 |
| `basic_short_string/assign` | 3.48 | 3.73 | 5.36 | 5.63 | 2.870e8 |
| `std::string/assign` | 4.18 | 4.28 | 4.50 | 4.89 | 2.391e8 |
| `boost::static_string/assign` | 3.83 | 3.95 | 4.27 | 5.16 | 2.610e8 |
| Bengal PMR batch | 46.70 | 51.83 | 53.44 | 61.78 | 2.141e7 |
| Standard monotonic PMR batch | 49.83 | 54.63 | 55.61 | 63.75 | 2.007e7 |
| Default allocator batch | 60.63 | 65.65 | 67.34 | 81.79 | 1.649e7 |
| `qos_jthread` start/join | 31,549.90 | 39,509.80 | 52,199.60 | 65,361.10 | 3.170e4 |
| `std::thread` start/join | 33,720.10 | 43,564.70 | 60,830.40 | 104,709.60 | 2.966e4 |

The allocation audit processed 1,000 batches:

- Bengal resource upstream allocations: `0`
- Standard local monotonic upstream allocations: `0`
- PMR heap allocations: `1,000`
- PMR heap allocated bytes: `512,000`

## Google Benchmark Cross-Check

Median wall-clock results:

| Workload | Median |
|---|---:|
| `type_map_get` | 0.355 ns |
| `tuple_get` | 0.352 ns |
| `short_string_assign` | 3.52 ns |
| `standard_string_assign` | 4.22 ns |
| `boost_static_string_assign` | 3.87 ns |
| Bengal PMR batch | 29.7 ns |
| Standard monotonic PMR batch | 38.0 ns |
| Default allocator batch | 41.3 ns |
| `qos_thread` start/join | 31,952 ns |
| `standard_thread` start/join | 34,813 ns |

## Decisions

- `type_map` remains for its constrained type-indexed API, not a speed claim.
  Google Benchmark shows lookup equivalent to `std::tuple`.
- `basic_short_string` remains because it provides explicit bounded behavior
  while matching or improving on the tested fixed-string baseline.
- `static_buffer_resource` remains for deterministic exhaustion, no upstream
  allocation, and high-water observability. Both harnesses show competitive
  batch performance, but Bengal does not claim universal superiority.
- `qos_jthread` remains for cancellation and observable QoS setup. Thread
  creation is OS-dominated, and these results do not establish a general
  startup advantage.

## Limitations

- Results come from one virtualized x86_64 runner and one compiler.
- CI runners are not isolated benchmark machines and may vary between runs.
- No CPU affinity, frequency control, or hardware performance counters were
  used.
- Native tail columns describe sampled run averages, not individual events.
- These microbenchmarks do not predict end-to-end application latency.

