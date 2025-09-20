CXX=g++
CXXFLAGS=-g -static -std=c++20
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

ycc: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o ycc

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: ycc
	sh test.sh

clean:
	rm -f ycc *.o *.s *~ tmp

.PHONY: test clean
