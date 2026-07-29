# Threading Contract

`bengal::qos_jthread` wraps `std::jthread` and preserves its RAII stop and join
behavior.

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

Callables may accept a leading `std::stop_token`, matching `std::jthread`, or
omit it. Unhandled callable exceptions terminate the process, as with
`std::thread`.

