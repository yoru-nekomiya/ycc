#!/bin/sh

try(){
    expected="$1"
    input="$2"

    ./ycc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
	echo "$input => $actual"
    else
	echo "$input => $expected expected, but got $actual."
	exit 1
    fi   
}

try 0 0
try 42 42
try 2 '1+1'
try 9 '10-1' 
try 15 '3*5' 
try 5 '10/2'
try 12 '(1+2) * 4'
try 1 '1<2'
try 0 '2<1'
try 1 '2>1'
try 0 '1>2'
try 1 '3<=5'
try 1 '3<=3'
try 0 '5<=3'
try 1 '5>=3'
try 1 '5>=5'
try 0 '5>=8'
try 1 '3==3'
try 0 '3==7'
try 1 '5!=6'
try 0 '9!=9'
try 4 '+5+(-1)'

echo 'Tests are passed!'

