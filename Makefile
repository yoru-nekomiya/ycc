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

clean:
	rm -rf build/
	rm -rf tests/out/ 

.PHONY: test clean
