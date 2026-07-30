# Provenance

Bengal is a new implementation informed by the public behavior and limitations
of the Tiger project. No Tiger source files are included in this repository.

The inspected Tiger repository did not contain a license file. Bengal must not
intentionally port Tiger source unless the relevant ownership and licensing
rights are confirmed.

Bengal is independently distributed under the MIT License. That license
applies only to Bengal's original implementation and does not grant rights to
Tiger source.

## SPSC Queue Evaluation

The `0.3` queue design evaluated established bounded SPSC interfaces and
implementation tradeoffs in:

- [Boost.Lockfree `spsc_queue`](https://www.boost.org/doc/libs/release/doc/html/lockfree/reference.html);
- [Folly `ProducerConsumerQueue`](https://github.com/facebook/folly);
- [Rigtorp `SPSCQueue`](https://github.com/rigtorp/SPSCQueue); and
- the classic acquire/release single-producer/single-consumer ring-buffer
  protocol.

Bengal does not vendor or copy source from those projects. Its implementation
uses original code with compile-time embedded storage because Bengal requires
no runtime queue allocation, no core dependency, a usable capacity equal to
the template argument, and explicit full/empty behavior.

The tradeoff is intentionally narrow scope: exactly one producer and one
consumer, no blocking wait strategy, and no built-in close operation. Boost,
Folly, or Rigtorp may be preferable when their dependency, allocation, API, or
platform tradeoffs fit an application better. Bengal makes no universal
throughput claim over those implementations.
