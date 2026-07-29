# Bengal Roadmap

## Project Thesis

Bengal provides predictable-latency C++ primitives with explicit memory,
overflow, lifetime, and platform behavior. It remains domain-neutral.

## 0.1: Portable Foundation

- Establish installable CMake and dependency-free local builds.
- Implement type map, type set, fixed-capacity text, bounded PMR storage, and
  QoS-aware RAII threads.
- Cover known correctness failures and boundary conditions.
- Run warning-clean tests on GCC, Clang, Apple Clang, Linux, and macOS ARM.
- Run AddressSanitizer and UndefinedBehaviorSanitizer in CI.
- Maintain MIT license and source provenance documentation.

Exit gate: all tests and sanitizers pass on supported platforms.

## 0.2: Performance Evidence

- Maintain a dependency-free benchmark harness and cross-check important
  results with Google Benchmark before publication.
- Compare against standard C++ and established fixed-capacity alternatives.
- Measure allocation count, throughput, and p50/p95/p99/p99.9 latency.
- Maintain bounded fuzzing for fixed-capacity text and allocator sequences.
- Retain architecture-specific acceleration only when it improves a published
  workload without changing semantics.

Exit gate: each performance-oriented API has a reproducible baseline.

## 0.3: Concurrency and Platform Support

- Evaluate an established bounded SPSC queue implementation.
- Extend thread-startup status reporting and platform capability discovery.
- Add sustained-load and ThreadSanitizer coverage.
- Document ownership, thread-safety, and real-time limitations for every API.

Exit gate: a downstream application can run a bounded pipeline without hidden
allocation or ambiguous shutdown behavior.

## 1.0: Stable Core

- Stabilize names and compatibility policy.
- Publish package-manager integration and API documentation.
- Complete cross-platform CI and downstream compatibility tests.
- Publish benchmark methodology and results with hardware details.

Exit gate: at least one independent downstream application validates Bengal's
APIs and operational behavior.

## Non-Goals

- Market strategies, exchange adapters, and order execution.
- Custom TLS, WebSocket, JSON, or financial protocol engines.
- Claims of core pinning or real-time scheduling without OS support.
- Market-beating or profitability claims.
