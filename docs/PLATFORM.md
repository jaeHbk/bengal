# Platform Capability Contract

`bengal::current_platform_capabilities()` reports the operating system family
and thread-QoS backend implemented by the compiled Bengal headers.

## Current Matrix

| Platform | `operating_system` | `thread_qos_backend` |
|---|---|---|
| Apple | `macos` | `apple` |
| Linux | `linux_os` | `none` |
| Windows | `windows` | `none` |
| Other | `unknown` | `none` |

These values describe Bengal support, not every feature exposed by the host
operating system. For example, Linux scheduling APIs may exist even though
Bengal does not currently provide a Linux QoS backend.

The function is `constexpr`; it performs no runtime probe, syscall, or
allocation. A binary moved to another operating-system family must be rebuilt.

## Worker Startup

Successful `qos_jthread` construction means the native thread was created and
the startup handshake completed. `startup_status()` reports:

- `worker_started`: whether the worker reached Bengal's startup handshake;
- `requested_qos`: the requested class;
- `qos`: `not_requested`, `applied`, `unsupported`, or `failed`; and
- `error`: the operating-system error, when present.

`platform_default` produces `not_requested` with no error. A non-default class
on a platform without a backend produces `unsupported` and
`std::errc::operation_not_supported`. Apple setup success produces `applied`.
Other Apple failures produce `failed` with the native error code.

The worker callable still runs when QoS is unsupported or fails. Applications
that require a scheduling hint must inspect startup status and decide whether
to proceed. `qos_status()` remains as the error-code-only compatibility view.

No status implies CPU pinning, priority elevation, hard real-time scheduling,
or a latency guarantee.
