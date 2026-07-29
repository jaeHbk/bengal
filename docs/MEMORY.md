# Bounded Memory Contract

`bengal::static_buffer_resource<N>` is a monotonic, bounded
`std::pmr::memory_resource`.

## Lifetime

The resource must outlive every container and object using its allocations.
Calling `release()` invalidates all outstanding allocations. Call it only
after those objects have been destroyed or have stopped using their storage.

## Allocation

- Allocation advances through the fixed internal buffer.
- Individual deallocation is intentionally a no-op.
- Exhaustion throws `std::bad_alloc`.
- There is no hidden upstream resource or heap fallback.
- Alignment requests supported by `std::pmr::memory_resource` are honored while
  sufficient space remains.

Repeated container growth can retain superseded blocks until `release()`.
Reserve known capacity when predictable memory use matters.

## Concurrency

The resource is not thread-safe. A resource should be owned by one thread or
protected externally.

## Observability

- `used()` includes alignment padding.
- `remaining()` reports currently available trailing storage.
- `high_water_mark()` survives `release()` for workload sizing.
- `reset_high_water_mark()` sets the high-water value to current usage.

These metrics and deterministic exhaustion are Bengal's reason for providing
the resource. It is not claimed to be faster than
`std::pmr::monotonic_buffer_resource`.

