# Bengal Benchmarking

## Purpose

Bengal benchmarks determine whether a primitive provides useful latency,
allocation, or predictability characteristics. They do not measure network
latency, trading outcomes, or market-beating performance.

## Current Workloads

- Type lookup: `bengal::type_map` compared with `std::tuple`.
- Short symbol assignment: `basic_short_string<15>` compared with an
  SSO-capable `std::string`.
- Bounded batches: Bengal PMR storage compared with a heap `std::vector` and
  `std::pmr::monotonic_buffer_resource` backed by local storage.
- Allocation audit: upstream allocation count and bytes for repeated batches.

The standard PMR comparison is the most important allocator baseline. Beating
heap allocation alone is not sufficient evidence that Bengal needs a custom
resource. Conversely, bounded storage may remain useful when it is slower than
the heap baseline if deterministic exhaustion and zero upstream allocation are
requirements.

## Running

```sh
make benchmark
./build/bengal_benchmarks --iterations=1000000 --samples=21
```

For CMake builds:

```sh
cmake -S . -B build -DBENGAL_BUILD_BENCHMARKS=ON
cmake --build build --config Release
./build/bengal_benchmarks
```

Use a release build. Record the compiler version, flags, CPU model, operating
system, power mode, and whether other workloads were active.

## Interpretation

- Compare median and p95, not a single elapsed duration.
- Treat differences below run-to-run variance as equivalent.
- Repeat runs in fresh processes and reverse comparison order when validating
  a claim.
- Inspect generated assembly or use a profiler when a result is surprising.
- Confirm publication-quality results with an established framework such as
  Google Benchmark.
- Do not add a feature solely because it wins a synthetic microbenchmark.

No benchmark is a test: results must not fail a build based on a timing
threshold. Functional invariants remain in the test suite.

## Acceptance Criteria

A performance-specific primitive remains in Bengal only when it offers at
least one demonstrated advantage over the closest established alternative:

- lower tail latency under a representative workload;
- fewer or no dynamic allocations;
- explicit bounded failure behavior;
- materially better throughput without weaker semantics; or
- observability unavailable from the baseline.

All claims must identify the workload and hardware. Bengal does not use a
microbenchmark result as evidence of end-to-end application performance.

## Current Design Decisions

- `type_map` is expected to match `std::tuple` lookup rather than outperform
  it; Bengal's value is the constrained type-indexed API.
- `basic_short_string` remains a candidate for bounded identifiers, subject to
  cross-platform comparison with established fixed-string implementations.
- `static_buffer_resource` is retained for explicit exhaustion and high-water
  observability. It is not currently presented as faster than standard PMR.
