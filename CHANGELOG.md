# Changelog

## 0.3.0 - 2026-07-30

Concurrency and platform-support milestone.

### Added

- Fixed-capacity `spsc_queue<T, N>` with explicit producer, consumer,
  backpressure, shutdown, allocation, and exception contracts.
- Compile-time operating-system and Bengal QoS-backend capability reporting.
- Structured worker-startup outcomes for default, applied, unsupported, and
  failed QoS setup.
- One-million-event sustained SPSC validation and GCC ThreadSanitizer CI.
- End-product, concurrency, platform, and `0.3` validation documentation.

### Changed

- The bounded-pipeline example now uses a concurrent queue, explicit
  end-of-stream event, observable backpressure, and post-join result access.
- Installed-package consumers now validate queue and platform headers.

### Validation

- GCC and Clang on Linux.
- Apple Clang on ARM macOS, including non-default utility QoS.
- AddressSanitizer and UndefinedBehaviorSanitizer.
- ThreadSanitizer over unit and sustained-load tests.
- Bounded fuzzing, installed consumers, and established benchmark baselines.

Sustained-load timing is diagnostic and host-specific. Bengal makes no
universal throughput, real-time, application-latency, or profitability claims.

## 0.2.0 - 2026-07-29

Performance-evidence milestone.

### Added

- Native p50/p95/p99/p99.9 sample-percentile and throughput reporting.
- `qos_jthread` versus `std::thread` startup baselines.
- Independent Google Benchmark cross-check suite.
- Reproducible CI artifacts containing hardware metadata and raw benchmark
  output.
- Published `0.2` performance report with scope and limitations.

### Changed

- Benchmark methodology now distinguishes sampled run-average percentiles from
  individual-operation latency distributions.
- Established baseline CI runs 1,001 native samples and compares with
  Boost.StaticString.

## 0.1.0 - 2026-07-29

Initial Bengal release.

### Added

- Tuple-backed `type_map` with unique type keys and const-safe iteration.
- Compile-time `type_set` union, intersection, and iteration.
- `basic_short_string<N>` with explicit overflow and truncation behavior.
- Bounded `static_buffer_resource<N>` with deterministic exhaustion and
  high-water metrics.
- Portable stoppable QoS workers with Apple scheduling hints and observable
  startup status.
- Installable `Bengal::Core` CMake package.
- Property tests, bounded fuzzing, sanitizer coverage, benchmarks, and an
  integrated bounded-pipeline example.

### Validation

- GCC and Clang on Linux.
- Apple Clang on ARM macOS, including non-default utility QoS.
- AddressSanitizer and UndefinedBehaviorSanitizer.
- Installed-package consumer builds.
- Standard library and Boost.StaticString benchmark baselines.

Performance results remain workload- and hardware-specific. Bengal does not
claim universal superiority over established alternatives.
