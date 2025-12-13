//#include <stdio.h>
//#include <time.h>

int printf();

#define N_MAX 10000000
#define N 10000000
int is_prime[N_MAX/*10000000*/ + 1]; 

void sieve(int n) {
  int i;
  for (i = 0; i <= n; i++) {
    is_prime[i] = 0;
  }
  
  is_prime[0] = 1;
  is_prime[1] = 1;
  
  int p;
  for (p = 2; p * p <= n; p++) {
    if (is_prime[p] == 0) {
      int i;
      for (i = p * p; i <= n; i += p) {
	is_prime[i] = 1;
      }
    }
  }
}

int main() {
  //int N = 10000000;
  long prime_sum = 0; // 結果の総和
  
  sieve(N);

  int i;
  for (i = 2; i <= N; i++) {
    if (is_prime[i] == 0) {
      prime_sum += i;
    }
  }
  
  printf("Eratosthenes Sieve (N=%d), ", N);
  printf("Prime Sum Checksum: %ld\n", prime_sum);
  return 0;
}
