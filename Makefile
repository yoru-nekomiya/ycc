CXX=g++
CXXFLAGS=-g -static -std=c++20
SRCS=$(wildcard src/*.cpp) $(wildcard src/optimization/*.cpp) $(wildcard src/optimization/global/*.cpp) $(wildcard src/optimization/local/*.cpp)
OBJS=$(patsubst src/%.cpp,build/%.o,$(SRCS))
TARGET=build/ycc

$(shell mkdir -p build)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TARGET)
	sh run_tests.sh

bench: $(TARGET)
	sh run_bench.sh $(filter-out $@,$(MAKECMDGOALS))

clean:
	rm -rf build/
	rm -rf tests/out/ tests/*.s
	find bench -mindepth 2 -maxdepth 2 -type d -name out -exec rm -rf {} +
	rm -f bench/report.txt
	find . -type f \( -name "*.lir" -o -name "*.dot" -o -name "*.svg" \) -delete

%::
	@:

.PHONY: test bench clean
