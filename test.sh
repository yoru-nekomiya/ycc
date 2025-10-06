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

try 0 'main(){return 0;}'
try 42 'main(){return 42;}'
try 2 'main(){return 1+1;}'
try 9 'main(){return 10-1;}' 
try 15 'main(){return 3*5;}' 
try 5 'main(){return 10/2;}'
try 3 'main(){return (1+2) * (8/4) / (3-1);}'
try 1 'main(){return 1<2;}'
try 0 'main(){return 2<1;}'
try 1 'main(){return 2>1;}'
try 0 'main(){return 1>2;}'
try 1 'main(){return 3<=5;}'
try 1 'main(){return 3<=3;}'
try 0 'main(){return 5<=3;}'
try 1 'main(){return 5>=3;}'
try 1 'main(){return 5>=5;}'
try 0 'main(){return 5>=8;}'
try 1 'main(){return 3==3;}'
try 0 'main(){return 3==7;}'
try 1 'main(){return 5!=6;}'
try 0 'main(){return 9!=9;}'
try 4 'main(){return +5+(-1);}'
try 5 'main(){a=2+3; return a;}'
try 3 'main(){a=4-1; b=a; return b;}'
try 20 'main(){x = 0; y = 0; for(;x < 10; x = x + 1) y = y + 2; return y;}'
try 20 'main(){b = 0; while(b < 20) b = b + 1; return b;}'
try 5 'main(){a = 0; b = 1; if(b > 5) a = 30; else a = 5; return a;}'
try 10 'main(){x=0; y=0; for(; x<10; ){x=x+1; y=y+1;} return y;}'
try 0 'main(){x=-10; y=10; while(x<10){x=x+1; y=y-1;} return x+y;}'
try 7 'main(){x=0; y=1; if(y<10){x=x+2; y=y+4;}else{x=x+1;} return x+y;}'
try 120 'kaijo(a){ if(a == 0) return 1; b = a * kaijo(a-1); return b;} main(){return kaijo(5);}'
try 55 'fib(a){if(a == 0) return 0; else if(a == 1) return 1; else return (fib(a-1)+fib(a-2));} main(){return fib(10);}'

echo 'Tests are passed!'

