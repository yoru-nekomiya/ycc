//Ackermann Function

//#include <stdio.h>
//#include <time.h>

int printf();

int ackermann(int m, int n) {
  if (m == 0) {
    return n + 1;
  } else if (n == 0) {
    return ackermann(m - 1, 1);
  } else {
    return ackermann(m - 1, ackermann(m, n - 1));
  }
}

int main() {
  int M = 3;
  int N = 10; 
  int result = ackermann(M, N);
  printf("Result: %d\n", result);
  return 0;
}
