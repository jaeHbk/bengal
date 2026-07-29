# Bengal

[![CI](https://github.com/jaeHbk/bengal/actions/workflows/ci.yml/badge.svg)](https://github.com/jaeHbk/bengal/actions/workflows/ci.yml)

Bengal is a C++20 library for predictable-latency software. It provides small,
explicit primitives for type-indexed dispatch, bounded text, bounded memory,
and platform-aware worker threads.

Bengal is an independent successor project inspired by Tiger's goals. Its
focus is correctness, portability, explicit failure behavior, and measured
performance rather than platform-specific performance claims.

## Initial Components

- `bengal::type_map`: unique, type-indexed values backed by `std::tuple`.
- `bengal::type_set`: compile-time set union, intersection, and iteration.
- `bengal::basic_short_string<N>`: fixed-capacity text with no silent overflow.
- `bengal::static_buffer_resource<N>`: bounded PMR storage with usage metrics.
- `bengal::qos_jthread`: stoppable RAII worker with observable Apple QoS
  startup.

The library is header-only in the initial release.

## Build

With CMake 3.20 or newer:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

A dependency-free convenience build is also available:

```sh
make test
make benchmark
make example
```

Clang-based fuzz targets are available through
`-DBENGAL_BUILD_FUZZERS=ON`.

## Design Rules

- No allocation in bounded primitives after construction.
- No silent truncation or hidden heap fallback.
- No platform scheduling claim stronger than the operating-system contract.
- No optimization is accepted without a reproducible comparison.
- Domain logic belongs in downstream applications, not Bengal core.

## Status

Bengal is pre-release software. APIs may change before `1.0`, published
performance claims do not yet exist, and benchmark results remain
workload-specific.

See [docs/ROADMAP.md](docs/ROADMAP.md) and
[docs/APPLICATIONS.md](docs/APPLICATIONS.md). Benchmark methodology and
interpretation rules are in [docs/BENCHMARKING.md](docs/BENCHMARKING.md).
Runtime contracts are documented in [docs/MEMORY.md](docs/MEMORY.md) and
[docs/THREADING.md](docs/THREADING.md).
Release changes are recorded in [CHANGELOG.md](CHANGELOG.md).

## License

Bengal is available under the [MIT License](LICENSE).
