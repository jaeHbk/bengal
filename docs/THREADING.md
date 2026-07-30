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

## Lifetime and Ownership

A joinable `qos_jthread` owns its native thread. Destruction and move
assignment request stop and join before releasing the current thread. A moved
object remains valid but does not own the transferred thread.

`detach()` deliberately gives up RAII joining. A detached callable may outlive
the `qos_jthread`, so every captured object must outlive the worker. Callers
that need to stop a detached worker must retain a copied `stop_source`.

Calling `join()` or `detach()` when the object is not joinable follows
`std::thread` and throws `std::system_error`. Destroying a joinable
`qos_jthread` on its own worker thread cannot join safely and terminates
through the noexcept destructor path; ownership must prevent that situation.

## Stop State

`stop_source` and `stop_token` share a stop state. Constructing a source or the
`qos_jthread` startup handshake can therefore throw `std::bad_alloc`; native
thread creation can throw `std::system_error`.
`request_stop()` is idempotent: exactly one caller observes `true`.
Exceptions from copying or moving the callable and its arguments propagate
during construction.

Copies of a source or token may be queried from multiple threads. The stop bit
is atomic. Operations that mutate the same `qos_jthread` object, including
`join()`, `detach()`, move assignment, and destruction, require external
synchronization.

## Scheduling

Apple QoS is a scheduler hint. It does not pin a worker to a performance or
efficiency core, provide hard real-time scheduling, or guarantee latency.

## Callable Behavior

Callables may accept a leading `bengal::stop_token` or omit it. Destruction
requests stop and joins a joinable worker. Unhandled callable exceptions
terminate the process, as with `std::thread`.
