//#define N 64 //size of matrix (N*N)
//#define ITERATIONS 100
int N = 64;
int ITERATIONS = 100;
/*
int A[N][N];
int B[N][N];
int C[N][N];
*/
int A[64][64];
int B[64][64];
int C[64][64];

int printf();

void init_matrix() {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      A[i][j] = i + j;
      B[i][j] = i - j;
      C[i][j] = 0;
    }
  }
  return;
}

void matrix_multiply() {
  for (int it = 0; it < ITERATIONS; it++) {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
	int sum = 0;
	for (int k = 0; k < N; k++) {
	  sum += A[i][k] * B[k][j];
	}
	C[i][j] = sum;
      }
    }
  }
  return;
}

int main() {
  init_matrix();
  matrix_multiply();
  printf("C[N-1][N-1] = %d\n", C[N-1][N-1]);
  return 0;
}
