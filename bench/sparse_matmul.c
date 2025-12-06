//#include <stdio.h>
//#include <time.h>

int printf();

// CSR (Compressed Sparse Row)
int NUM_ROWS = 4096; 
int NUM_NON_ZEROS = 16384; 
int ITERATIONS = 100; 

int values[/*NUM_NON_ZEROS*/16384];      
int col_indices[/*NUM_NON_ZEROS*/16384]; 
int row_pointers[/*NUM_ROWS + 1*/4096+1]; 

int X[/*NUM_ROWS*/4096]; 
int Y[/*NUM_ROWS*/4096]; 

void init_sparse_matrix(int n, int nnz) {
  int i;
  int j;
  int current_nnz = 0;
  
  row_pointers[0] = 0;
  for (i = 0; i < n; i++) {
    int elements_in_row = (i % 4) + 1; 
    row_pointers[i + 1] = row_pointers[i] + elements_in_row;
    
    for (j = 0; j < elements_in_row; j++) {
      values[current_nnz] = (i * j + 1) % 10 + 1; 
      col_indices[current_nnz] = (i + j) % n; 
      current_nnz++;
    }
  }
  
  for (i = 0; i < n; i++) {
    X[i] = (i * 3 + 1) % 5 + 1; 
    Y[i] = 0;
  }
}

// Y = A * X
void spmv(int n) {
  int i;
  int j;
  for (int it = 0; it < ITERATIONS; it++) {
    for (i = 0; i < n; i++) {
      int row_start = row_pointers[i];
      int row_end = row_pointers[i + 1];
      int sum = 0;
      
      for (j = row_start; j < row_end; j++) {
	int col = col_indices[j];
	int val = values[j];
        
	sum += val * X[col]; 
      }
      Y[i] = sum;
    }
  }
}

int main() {
  int N_ROWS = 4096;
  init_sparse_matrix(N_ROWS, NUM_NON_ZEROS);
  
  spmv(N_ROWS);
  
  long checksum = 0;
  int i;
  for (i = 0; i < N_ROWS; i++) {
    checksum += Y[i];
  }
  
  printf("Sparse Matrix-Vector Mul, ");
  printf("Y Vector Checksum: %ld\n", checksum); 
  
  return 0;
}
