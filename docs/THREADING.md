# Threading Contract

`bengal::qos_jthread` provides RAII stop and join behavior over `std::thread`.
It uses `bengal::stop_source` and `bengal::stop_token` because Apple libc++
does not currently provide the C++20 `std::jthread` family.

## QoS Startup

Construction waits until the new worker completes Bengal's startup handshake.
`startup_status()` reports:

- whether the worker reached the handshake;
- the requested QoS class;
- a QoS outcome of `not_requested`, `applied`, `unsupported`, or `failed`; and
- the operating-system error, when present.

`platform_default` produces `not_requested` without an error. A non-default
class on an unsupported platform produces `unsupported` with
`std::errc::operation_not_supported`. Successful Apple setup produces
`applied`; other Apple setup errors produce `failed`.

`qos_status()` remains the compatibility view that returns only the error
code.

The worker still runs when QoS configuration fails. Callers that require a
specific policy must inspect startup status and decide whether to continue.
Supported Bengal backends are documented in [PLATFORM.md](PLATFORM.md).

## Scheduling

Apple QoS is a scheduler hint. It does not pin a worker to a performance or
efficiency core, provide hard real-time scheduling, or guarantee latency.

## Callable Behavior

Callables may accept a leading `bengal::stop_token` or omit it. Destruction
requests stop and joins a joinable worker. Unhandled callable exceptions
terminate the process, as with `std::thread`.
