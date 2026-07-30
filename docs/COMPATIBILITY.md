# Compatibility Policy

Bengal follows semantic versioning beginning with `1.0.0`. Before `1.0`,
minor releases may change source contracts; patch releases remain limited to
compatible fixes and documentation.

## Stable Surface

For a stable major release, compatibility covers:

- public names and documented semantics in [API.md](API.md);
- direct component headers and `<bengal/bengal.hpp>`;
- the `Bengal::Core` CMake target and `Core` package component;
- the C++20 minimum language level; and
- documented platform and concurrency contracts.

Within one major release, Bengal will not remove or rename a public API,
narrow accepted inputs, weaken ownership or synchronization guarantees, or
silently change overflow behavior. Additive APIs, defect fixes, stronger
diagnostics, and support for additional platforms may ship in minor releases.

## Not Covered

Compatibility does not cover:

- `bengal::detail`;
- repository test, benchmark, fuzz, example, or CMake helper internals;
- object layout, symbol names, or ABI across compiler and standard-library
  versions;
- exact performance, allocation sizes caused by implementation padding, or
  native thread handle types across platforms; or
- behavior outside documented preconditions, including unsupported queue
  topologies and invalid object lifetimes.

Bengal is header-only and template-heavy. Consumers should rebuild when
updating Bengal, the compiler, or the standard library.

## Deprecation and Removal

Public APIs targeted for removal are documented and deprecated for at least
one minor release. Removal occurs only in a new major release unless the API
cannot be retained without a correctness or security defect. Such exceptional
changes require explicit release notes and migration guidance.

## Version Checks

Consumers may inspect `<bengal/version.hpp>` at compile time. Build systems
should prefer CMake version selection:

```cmake
find_package(Bengal 1 CONFIG REQUIRED COMPONENTS Core)
```

The source header version and CMake project version must match in every
published archive. Before `1.0`, generated CMake package files accept only the
same minor release; stable releases accept compatible versions in the same
major release.
