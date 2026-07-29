# Changelog

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

