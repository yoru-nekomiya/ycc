CXX=g++
CXXFLAGS=-g -static -std=c++20
SRCS=$(wildcard src/*.cpp)
OBJS=$(patsubst src/%.cpp,build/%.o,$(SRCS))
TARGET=build/ycc

$(shell mkdir -p build)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

build/%.o: src/%.cpp
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
	find . -name "*.lir" -delete

%::
	@:

.PHONY: test bench clean
