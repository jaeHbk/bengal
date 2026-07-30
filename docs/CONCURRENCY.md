# Concurrency Contract

`bengal::spsc_queue<T, N>` is a fixed-capacity, non-blocking queue for exactly
one producer thread and one consumer thread. It is intended for bounded event
handoff where overload and shutdown policy remain visible to the application.

## Capacity and Allocation

- `N` is the usable capacity and must be greater than zero.
- Slot storage and indices are embedded in the queue object.
- Construction and steady-state queue operations do not allocate.
- `try_push()` and `try_emplace()` return `false` when all slots are occupied.
- There is no heap fallback, overwrite, blocking wait, or silent drop.

The implementation uses `N + 1` internal slots so full and empty states remain
distinct. `indices_are_always_lock_free` reports the standard library's
compile-time guarantee for the atomic index type. Applications requiring
lock-free indices should check it for their target toolchain.

## Ownership

The following calls belong to the producer thread:

- `try_push()`;
- `try_emplace()`; and
- `full()`.

The following calls belong to the consumer thread:

- `try_pop()`;
- `consume_one()`; and
- `empty()`.

`size_approx()` is an observational snapshot. Concurrent progress can make it
stale immediately, so it must not control correctness or reserve capacity.
Calling producer operations from multiple threads, consumer operations from
multiple threads, or destroying the queue during an operation is unsupported.

## Publication and Reuse

The producer constructs an element before publishing its index with release
ordering. The consumer acquires that index before reading the element. The
consumer destroys the element before releasing its read index, and the
producer acquires that index before reusing the slot.

These acquire/release relationships cover the element handoff. They do not
make unrelated application state thread-safe.

## Overload and Shutdown

The queue intentionally does not choose an overload policy. A producer can
retry, yield, count backpressure, drop explicitly, or stop upstream work after
`try_push()` returns `false`.

The queue also has no implicit closed state. Applications must use an explicit
protocol, such as an end-of-stream event, or coordinate an external stop token
while defining whether queued items are drained or abandoned. The queue must
outlive both participating threads.

## Values and Exceptions

Values need not be default-constructible. `try_pop()` move-constructs a result;
`consume_one()` invokes a callback with `T&` and avoids that extra move.

If value construction fails, the write index is not published. If a
`consume_one()` callback throws, the read index is not advanced and the value
remains queued. A throwing move constructor can leave the queued value in its
type-defined moved-from state. Nothrow event types are recommended for
predictable-latency pipelines.

## Scope

This is an SPSC primitive, not a general task queue. It provides no MPSC or
MPMC safety, fairness, wake-up mechanism, CPU affinity, real-time guarantee,
or end-to-end latency claim. The 64-byte index alignment is a conservative
false-sharing reduction, not a declaration of the host cache-line size.
