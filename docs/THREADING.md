# Threading Contract

`bengal::qos_jthread` provides RAII stop and join behavior over `std::thread`.
It uses `bengal::stop_source` and `bengal::stop_token` because Apple libc++
does not currently provide the C++20 `std::jthread` family.

## QoS Startup

Construction waits until the new worker attempts QoS configuration.
`qos_status()` reports that result:

- success for `platform_default`;
- the operating-system error when Apple QoS setup fails; or
- `std::errc::operation_not_supported` for non-default QoS on unsupported
  platforms.

The worker still runs when QoS configuration fails. Callers that require a
specific policy must inspect `qos_status()` and decide whether to continue.

## Scheduling

Apple QoS is a scheduler hint. It does not pin a worker to a performance or
efficiency core, provide hard real-time scheduling, or guarantee latency.

## Callable Behavior

Callables may accept a leading `bengal::stop_token` or omit it. Destruction
requests stop and joins a joinable worker. Unhandled callable exceptions
terminate the process, as with `std::thread`.
