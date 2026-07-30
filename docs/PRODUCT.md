# Bengal End Product

## Product Boundary

Bengal 1.0 should be a small, installable C++20 library for building bounded
event pipelines. It should make memory limits, overload, thread startup,
ownership, and shutdown visible in code without becoming a networking,
serialization, storage, or trading framework.

The core distribution should remain header-only unless measured workloads
justify a compiled component. Consumers should be able to install one tagged
release, link `Bengal::Core`, include `<bengal/bengal.hpp>`, and use individual
headers when compile-time isolation matters.

## Stable Core

The intended 1.0 surface is:

- type-indexed composition through `type_map` and `type_set`;
- fixed-capacity text through `basic_short_string`;
- bounded monotonic allocation through `static_buffer_resource`;
- bounded one-producer/one-consumer handoff through `spsc_queue`;
- stoppable RAII workers with observable scheduling setup; and
- explicit compile-time platform capability reporting.

Every API must document capacity, allocation, ownership, thread safety,
exception behavior, and unsupported platforms. Bengal should not add generic
containers or wrappers that do not improve one of those contracts.

## Developer Experience

A production-shaped Bengal pipeline should have:

1. fixed-capacity event types and queues sized from observed load;
2. one named owner for every producer, consumer, and memory resource;
3. an explicit response to a full queue;
4. an explicit end-of-stream or stop-and-drain protocol;
5. startup status checked before accepting work; and
6. counters for backpressure, drops, high-water memory, and processing latency.

The bounded-pipeline example is the minimum reference shape. It is not an
application framework and deliberately leaves waiting and telemetry policy to
the downstream repository.

## Practical Applications

Applications remain in separate repositories and pin Bengal releases:

| Repository | Purpose | Bengal contracts validated |
|---|---|---|
| `bengal-market` | Read-only market capture and deterministic replay | bounded ingestion, gap/drop counters, p99 stage latency |
| `bengal-telemetry` | Local metrics and structured-event collector | batching, overload, persistence handoff |
| `bengal-gateway` | Protocol-neutral event router and load generator | typed dispatch, queue backpressure, mixed workloads |
| `bengal-midi` | macOS MIDI monitor and router | Apple QoS status, jitter, graceful cancellation |

`bengal-market` is the first proving application after `0.3`. Its first
release should use a public read-only feed, record raw input, replay it
deterministically, and compare an equivalent standard C++ pipeline. It should
not place orders or claim profitability, market-beating returns, or lower
network latency than a commercial broker.

## Release Bar

Bengal reaches 1.0 only when:

- GCC, Clang, and Apple Clang builds remain warning-clean;
- ASan, UBSan, TSan, fuzzing, install, and downstream-consumer jobs pass;
- published benchmarks include raw output and hardware metadata;
- one independent application pins a release and passes sustained replay; and
- names, compatibility policy, and package-manager integration are stable.

The practical end product is therefore Bengal plus independently versioned
applications, not one repository containing a market strategy and core
primitives.
