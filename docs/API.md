# Bengal Public API

This page inventories the intended stable surface. Behavioral details remain
in the linked contract documents. Names under `bengal::detail` and all files
outside `include/bengal` are implementation details.

## Library and Version

The umbrella header is `<bengal/bengal.hpp>`. Each component may also be
included directly. `<bengal/version.hpp>` provides:

- `BENGAL_VERSION_MAJOR`, `BENGAL_VERSION_MINOR`,
  `BENGAL_VERSION_PATCH`, and `BENGAL_VERSION_STRING`;
- `bengal::version_major`, `version_minor`, and `version_patch`; and
- `bengal::version` as a `std::string_view`.

The CMake package exports `Bengal::Core` and requires C++20.

## Type Composition

`<bengal/meta/type_map.hpp>` provides:

- `type_map<Ts...>`;
- `make_type_map(values...)`; and
- `get<T>(map)`.

Types in a map must be unique. Access is by exact, unqualified stored type.
Storage is the underlying `std::tuple`; Bengal adds no allocation. Construction
and callable exceptions propagate. Concurrent access follows the usual C++
rule: reads may overlap, but mutation requires external synchronization.

`<bengal/meta/type_set.hpp>` provides:

- `type_set<Ts...>`;
- `type_set_union`, `type_set_union_t`;
- `type_set_intersection`, `type_set_intersection_t`; and
- `for_each_type(set, callable)`.

Type sets also require unique types.
They contain no runtime state and perform no allocation.

## Bounded Primitives

`<bengal/text/short_string.hpp>` provides
`basic_short_string<Capacity>` and the `short_string` alias. Mutating methods
either report overflow, throw `std::length_error`, or explicitly truncate.
Storage is embedded and null-terminated without allocation. `front()` and
`back()` require a non-empty string, and `operator[]` requires a valid
position. Concurrent mutation requires external synchronization.

`<bengal/memory/static_buffer_resource.hpp>` provides
`static_buffer_resource<Capacity, Alignment>`. It is a monotonic PMR resource
with deterministic exhaustion and no upstream allocator. See
[MEMORY.md](MEMORY.md).

`<bengal/concurrency/spsc_queue.hpp>` provides
`spsc_queue<T, Capacity>`. It is non-blocking and supports exactly one producer
and one consumer. See [CONCURRENCY.md](CONCURRENCY.md).

## Threading and Platform

`<bengal/thread/qos_jthread.hpp>` provides:

- `stop_token` and `stop_source`;
- `qos_class` and `qos_outcome`;
- `thread_startup_status`;
- `set_current_thread_qos()`;
- `qos_available`; and
- `qos_jthread`.

QoS is currently implemented only on Apple platforms. Unsupported requests
remain observable and do not prevent the worker callable from running. See
[THREADING.md](THREADING.md).

`<bengal/platform/capabilities.hpp>` provides:

- `operating_system`;
- `thread_qos_backend`;
- `platform_capabilities`; and
- `current_platform_capabilities()`.

These are compile-time Bengal capabilities, not runtime host probes. See
[PLATFORM.md](PLATFORM.md).

## General Guarantees

- Public headers are self-contained and warning-clean on supported CI
  compilers.
- Header-only definitions are safe to include in multiple translation units.
- No function is async-signal-safe unless explicitly documented otherwise.
- No API provides hard real-time, scheduling, end-to-end latency, or
  profitability guarantees.
