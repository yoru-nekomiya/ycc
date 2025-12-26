//#include <stdio.h>
int printf();

// --- Define Configuration ---
#define ARRAY_SIZE 16384 // 1024 * 16 = 16384 elements (16K elements)
#define OUTER_ITERATIONS 500
#define INNER_REPEATS 10

// Global data array
int data[ARRAY_SIZE];
// Global char array to alias with
char buffer[ARRAY_SIZE * sizeof(int)];

// Function to perform the aliasing test
// The compiler must determine if 'p' (int*) and 'c' (char*) point to overlapping memory.
void alias_test_kernel(int *p, char *c) {
  int local_val = p[0];
  
  for (int i = 0; i < ARRAY_SIZE; i++) {
    // 1. Indirect store via 'c' (char*): Only affects the first byte of p[i]
    c[i * sizeof(int)] = (char)(i & 255); 
    
    // 2. Load via 'p' (int*): The compiler must decide if the value loaded
    // is affected by the store from c[...].
    int p_val = p[i];
    
    // 3. Simple non-vectorizable dependency chain using the loaded value
    for (int j = 0; j < INNER_REPEATS; j++) {
      p_val = p_val * 13 + (i * 7);
      p_val /= 2;
    }
    
    // 4. Store via 'p' (int*):
    p[i] = p_val;
  }
  
  // Use local_val to prevent the initial load from being eliminated entirely
  p[ARRAY_SIZE - 1] += local_val; 
}

// ----------------------------------------------------
// Main Function and Checksum
// ----------------------------------------------------

int main() {
  int i;
  long total_checksum = 0;
  
  // Initialize data array
  for (i = 0; i < ARRAY_SIZE; i++) {
    data[i] = i;
  }
  
  // Initialize buffer as well (though its initial content is less critical)
  for (i = 0; i < ARRAY_SIZE * sizeof(int); i++) {
    buffer[i] = (char)i;
  }
  
  // Benchmark Execution Loop
  for (int it = 0; it < OUTER_ITERATIONS; it++) {
    // Pass the same memory block using two different pointer types (aliasing)
    alias_test_kernel(data, (char *)data);
  }
  
  // Perform a test run on the explicitly aliased buffer memory
  alias_test_kernel((int *)buffer, buffer);
  
  // Calculate Checksum (Sum of final values)
  for (i = 0; i < ARRAY_SIZE; i++) {
    total_checksum += (long)data[i];
    total_checksum += (long)((int *)buffer)[i]; // Include aliased buffer for robustness
  }
  
  printf("Benchmark: Pointer Aliasing (Strict Aliasing Test), ");
  printf("Array Size: %d, Outer Iterations: %d, ", ARRAY_SIZE, OUTER_ITERATIONS);
  printf("Checksum (Final Data Sum): %ld\n", total_checksum);
  
  return 0;
}
