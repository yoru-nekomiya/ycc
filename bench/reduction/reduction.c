//#include <stdio.h>

int printf();

// --- Configuration ---
/*
int ARRAY_SIZE = 1048576; // 2^20 elements (large enough to test memory and SIMD)
int ITERATIONS = 100;     // Run several times for stability
*/
#define ARRAY_SIZE 1048576 // 2^20 elements (large enough to test memory and SIMD)
#define ITERATIONS 100     // Run several times for stability

// Global array to hold the input data
int data[ARRAY_SIZE/*1048576*/];

// --- Core Reduction Function ---
// Calculates the sum of squares of all elements in the array.
// The primary goal is to see how efficiently the compiler handles the accumulation (sum) variable.
long reduction_sum_of_squares() {
  // The accumulator must be 'long' to prevent overflow and to test 64-bit arithmetic
  long sum = 0; 
  int i;
  
  // Loop through the entire array
  for (i = 0; i < ARRAY_SIZE; i++) {
    long element = (long)data[i];
    
    // Operation: Sum += Element * Element
    // This requires two operations inside the loop (multiplication and addition),
    // which the compiler should ideally map to SIMD instructions efficiently.
    sum += element * element; 
  }
  
  return sum;
}

void initialize_data() {
  // Initialize array with simple data
  for (int i = 0; i < ARRAY_SIZE; i++) {
    data[i] = i % 1000; // Keep values small to prevent immediate overflow during calculation
  }
}

int main() {
  long final_checksum = 0;
  
  initialize_data();
  
  // Execute the benchmark multiple times
  for (int i = 0; i < ITERATIONS; i++) {
    // Accumulate the result from each iteration
    final_checksum += reduction_sum_of_squares();
  }
  
  printf("Reduction Calculation (Sum of Squares), ");
  printf("Array Size: %d, ", ARRAY_SIZE);
  printf("Total Checksum: %ld\n", final_checksum); 
  
  return 0;
}
