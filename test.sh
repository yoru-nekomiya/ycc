#!/bin/sh

try(){
    expected="$1"
    input="$2"

    ./ycc "$input" > tmp.s
    gcc -z noexecstack -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
	echo "$input => $actual"
    else
	echo "$input => $expected expected, but got $actual."
	exit 1
    fi   
}

try 0 'return 0;'
try 42 'return 42;'
try 2 'return 1+1;'
try 9 'return 10-1;' 
try 15 'return 3*5;' 
try 5 'return 10/2;'
try 3 'return (1+2) * (8/4) / (3-1);'
try 1 'return 1<2;'
try 0 'return 2<1;'
try 1 'return 2>1;'
try 0 'return 1>2;'
try 1 'return 3<=5;'
try 1 'return 3<=3;'
try 0 'return 5<=3;'
try 1 'return 5>=3;'
try 1 'return 5>=5;'
try 0 'return 5>=8;'
try 1 'return 3==3;'
try 0 'return 3==7;'
try 1 'return 5!=6;'
try 0 'return 9!=9;'
try 4 'return +5+(-1);'
try 5 'a=2+3; return a;'

echo 'Tests are passed!'

