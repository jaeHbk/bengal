CXX ?= c++
CXXFLAGS ?= -std=c++20 -O2 -g -Wall -Wextra -Wpedantic -Werror
BENCHMARK_CXXFLAGS ?= -std=c++20 -O3 -DNDEBUG -Wall -Wextra -Wpedantic -Werror
CPPFLAGS ?= -Iinclude

TEST_BINARY := build/bengal_tests
TEST_SOURCE := tests/bengal_tests.cpp
BENCHMARK_BINARY := build/bengal_benchmarks
BENCHMARK_SOURCE := benchmarks/bengal_benchmarks.cpp
EXAMPLE_BINARY := build/bengal_bounded_pipeline
EXAMPLE_SOURCE := examples/bounded_pipeline.cpp
HEADERS := $(wildcard include/bengal/*.hpp include/bengal/*/*.hpp)

.PHONY: all test benchmark example clean

all: $(TEST_BINARY) $(EXAMPLE_BINARY)

$(TEST_BINARY): $(TEST_SOURCE) $(HEADERS)
	@mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -pthread $< -o $@

$(BENCHMARK_BINARY): $(BENCHMARK_SOURCE) $(HEADERS)
	@mkdir -p build
	$(CXX) $(CPPFLAGS) $(BENCHMARK_CXXFLAGS) -pthread $< -o $@

$(EXAMPLE_BINARY): $(EXAMPLE_SOURCE) $(HEADERS)
	@mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -pthread $< -o $@

test: $(TEST_BINARY)
	./$(TEST_BINARY)

benchmark: $(BENCHMARK_BINARY)
	./$(BENCHMARK_BINARY)

example: $(EXAMPLE_BINARY)
	./$(EXAMPLE_BINARY)

clean:
	rm -rf build
