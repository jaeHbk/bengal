# Bengal

[![CI](https://github.com/jaeHbk/bengal/actions/workflows/ci.yml/badge.svg)](https://github.com/jaeHbk/bengal/actions/workflows/ci.yml)

Bengal is a C++20 library for predictable-latency software. It provides small,
explicit primitives for type-indexed dispatch, bounded text, bounded memory,
and platform-aware worker threads.

Bengal is an independent successor project inspired by Tiger's goals. Its
focus is correctness, portability, explicit failure behavior, and measured
performance rather than platform-specific performance claims.

## Components

- `bengal::type_map`: unique, type-indexed values backed by `std::tuple`.
- `bengal::type_set`: compile-time set union, intersection, and iteration.
- `bengal::basic_short_string<N>`: fixed-capacity text with no silent overflow.
- `bengal::static_buffer_resource<N>`: bounded PMR storage with usage metrics.
- `bengal::spsc_queue<T, N>`: fixed-capacity one-producer/one-consumer
  handoff with explicit backpressure.
- `bengal::qos_jthread`: stoppable RAII worker with observable Apple QoS
  startup.
- `bengal::current_platform_capabilities()`: compile-time discovery of Bengal's
  supported platform backends.

The library is header-only.

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

Installed consumers use the exported target:

```cmake
find_package(Bengal CONFIG REQUIRED COMPONENTS Core)
target_link_libraries(my_application PRIVATE Bengal::Core)
```

Pinned `FetchContent`, source-tree integration, install layouts, and package
validation are covered in [docs/INSTALLING.md](docs/INSTALLING.md).

## Design Rules

- No allocation in bounded primitives after construction.
- No silent truncation or hidden heap fallback.
- No platform scheduling claim stronger than the operating-system contract.
- No optimization is accepted without a reproducible comparison.
- Domain logic belongs in downstream applications, not Bengal core.

## Status

Bengal 1.x maintains the public source and package contracts documented in
[docs/COMPATIBILITY.md](docs/COMPATIBILITY.md). Published performance evidence
is workload- and hardware-specific and does not imply end-to-end application
performance.

See [docs/ROADMAP.md](docs/ROADMAP.md) and
[docs/APPLICATIONS.md](docs/APPLICATIONS.md). Benchmark methodology and
interpretation rules are in [docs/BENCHMARKING.md](docs/BENCHMARKING.md).
The intended end product is defined in [docs/PRODUCT.md](docs/PRODUCT.md).
The public surface is listed in [docs/API.md](docs/API.md), and release
compatibility rules are in
[docs/COMPATIBILITY.md](docs/COMPATIBILITY.md).
Runtime contracts are documented in [docs/MEMORY.md](docs/MEMORY.md),
[docs/CONCURRENCY.md](docs/CONCURRENCY.md),
[docs/THREADING.md](docs/THREADING.md), and
[docs/PLATFORM.md](docs/PLATFORM.md).
Published `0.2` evidence is in
[docs/PERFORMANCE_0.2.md](docs/PERFORMANCE_0.2.md).
Published `0.3` validation is in
[docs/VALIDATION_0.3.md](docs/VALIDATION_0.3.md).
Release changes are recorded in [CHANGELOG.md](CHANGELOG.md).

## License

Bengal is available under the [MIT License](LICENSE).
