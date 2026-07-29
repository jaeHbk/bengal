# Bengal Application Portfolio

Applications live in separate repositories and consume tagged Bengal releases.
They provide realistic workloads without putting domain dependencies into the
core library.

## 1. bengal-market

Primary proving application for high-rate market data.

Initial scope:

- Connect to one officially supported, read-only WebSocket feed.
- Timestamp receipt, decoding, dispatch, and publication.
- Normalize trades and quotes into bounded event types.
- Record raw input and provide deterministic replay.
- Detect sequence gaps, reconnect safely, and report dropped events.
- Compare a Bengal pipeline with an equivalent standard C++ baseline.

Order execution, strategy claims, and private Robinhood endpoints are excluded
from the first version. Paper trading is considered only after capture and
replay pass sustained-load tests.

## 2. bengal-telemetry

A local metrics and structured-event collector. It validates bounded batching,
short identifiers, predictable overload behavior, and background persistence
without the licensing constraints of financial data.

## 3. bengal-gateway

A protocol-neutral event routing and load-testing tool. It validates
type-indexed handlers, bounded queues, backpressure, and p99 latency under
mixed message sizes.

## 4. bengal-midi

A macOS-focused MIDI event monitor and router. It validates QoS hints,
low-jitter event handling, fixed-capacity messages, and graceful cancellation.
It must not claim hard real-time behavior.

## Repository Contract

- Bengal never depends on an application repository.
- Applications pin a Bengal release rather than tracking `main`.
- Generic microbenchmarks remain in Bengal.
- End-to-end and domain benchmarks remain with their application.
- Features move into Bengal only after at least two workloads need the same
  abstraction or the primitive is independently useful.

## Start Criteria

`bengal-market` starts after Bengal `0.1` is sanitizer-clean and has baseline
microbenchmarks. The other applications remain planned until they test a
specific unresolved Bengal design question.

