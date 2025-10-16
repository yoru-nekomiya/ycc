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

try 0 'int main(){return 0;}'
try 42 'int main(){return 42;}'
try 2 'int main(){return 1+1;}'
try 9 'int main(){return 10-1;}' 
try 15 'int main(){return 3*5;}' 
try 5 'int main(){return 10/2;}'
try 3 'int main(){return (1+2) * (8/4) / (3-1);}'
try 1 'int main(){return 1<2;}'
try 0 'int main(){return 2<1;}'
try 1 'int main(){return 2>1;}'
try 0 'int main(){return 1>2;}'
try 1 'int main(){return 3<=5;}'
try 1 'int main(){return 3<=3;}'
try 0 'int main(){return 5<=3;}'
try 1 'int main(){return 5>=3;}'
try 1 'int main(){return 5>=5;}'
try 0 'int main(){return 5>=8;}'
try 1 'int main(){return 3==3;}'
try 0 'int main(){return 3==7;}'
try 1 'int main(){return 5!=6;}'
try 0 'int main(){return 9!=9;}'
try 4 'int main(){return +5+(-1);}'
try 5 'int main(){int a; a=2+3; return a;}'
try 3 'int main(){int a; a=4-1; int b; b=a; return b;}'
try 20 'int main(){int x; x = 0; int y; y = 0; for(;x < 10; x = x + 1) y = y + 2; return y;}'
try 20 'int main(){int b; b = 0; while(b < 20) b = b + 1; return b;}'
try 5 'int main(){int a; a = 0; int b; b = 1; if(b > 5) a = 30; else a = 5; return a;}'
try 10 'int main(){int x; x=0; int y; y=0; for(; x<10; ){x=x+1; y=y+1;} return y;}'
try 0 'int main(){int x; x=-10; int y; y=10; while(x<10){x=x+1; y=y-1;} return x+y;}'
try 7 'int main(){int x; x=0; int y; y=1; if(y<10){x=x+2; y=y+4;}else{x=x+1;} return x+y;}'
try 120 'int kaijo(int a){ if(a == 0) return 1; int b; b = a * kaijo(a-1); return b;} int main(){return kaijo(5);}'
try 55 'int fib(int a){if(a == 0) return 0; else if(a == 1) return 1; else return (fib(a-1)+fib(a-2));} int main(){return fib(10);}'
try 100 'int main(){int a; a = 100; int* b; b = &a; return *b;}'
try 100 'int main(){int a; a = 100; int* b; b = &a; int** c; c = &b; return **c;}'
try 4 'int main(){int a; return sizeof(a);}'

echo 'Tests are passed!'

