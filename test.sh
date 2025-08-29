#!/bin/sh

set -eu

g++ main.cpp -o ycc
./ycc '3+5-7' > test.s
gcc test.s -o test
./test
echo $?
