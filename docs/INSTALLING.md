# Installing and Consuming Bengal

Bengal is a header-only C++20 library. Consumers link the `Bengal::Core`
interface target so include paths, language requirements, and thread flags
propagate correctly.

## Installed Package

```sh
cmake -S . -B build -DBENGAL_BUILD_TESTS=OFF \
  -DBENGAL_BUILD_EXAMPLES=OFF
cmake --build build
cmake --install build --prefix /path/to/bengal
```

Consume the installation with:

```cmake
find_package(Bengal 1 CONFIG REQUIRED COMPONENTS Core)
target_link_libraries(my_application PRIVATE Bengal::Core)
```

Pass the installation prefix through `CMAKE_PREFIX_PATH` when it is outside a
standard location. Unknown required package components are rejected.

## Pinned FetchContent

Applications should pin an immutable release tag:

```cmake
include(FetchContent)
FetchContent_Declare(
  Bengal
  GIT_REPOSITORY https://github.com/jaeHbk/bengal.git
  GIT_TAG v1.0.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Bengal)

target_link_libraries(my_application PRIVATE Bengal::Core)
```

When Bengal is nested through `FetchContent` or `add_subdirectory`, its tests,
examples, and installation rules default off. A parent may opt into them with
`BENGAL_BUILD_TESTS`, `BENGAL_BUILD_EXAMPLES`, and `BENGAL_INSTALL`.

## Downstream Candidate Validation

The repository consumer fixture accepts a local source checkout:

```sh
cmake -S tests/consumer -B build/source-consumer \
  -DBENGAL_SOURCE_DIR=/path/to/bengal
cmake --build build/source-consumer
ctest --test-dir build/source-consumer --output-on-failure
```

Downstream projects may use the same cache variable pattern to test a Bengal
worktree before changing their pinned tag. Release evidence must test both the
source override and an installed package. Published applications should use a
tag, not a branch or moving commit.

## Supported Toolchains

CI validates current hosted GCC and Clang on Linux, Apple Clang on macOS, and
MSVC on Windows. The minimum supported configuration is CMake 3.20 with a
C++20 compiler and standard library. Older compiler versions may work but are
not part of the release contract.
