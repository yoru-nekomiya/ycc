//#include <stdio.h>

int printf();

// --- Configuration ---
/*
int DATA_SIZE = 1000000;
int ITERATIONS = 50; 
int ALPHABET_SIZE = 256;
int ADLER_MOD = 65521; // The modulus for Adler-32 (largest prime less than 2^16)
*/
#define DATA_SIZE 1000000
#define ITERATIONS 50
#define ALPHABET_SIZE 256
#define ADLER_MOD 65521  // The modulus for Adler-32 (largest prime less than 2^16)

// --- Global Data and Checksum ---
int input_data[DATA_SIZE/*1000000*/]; 
long total_adler_checksum = 0;

// --- 1. Adler-32 Checksum Implementation ---
// Tests modulo arithmetic, data access, and loop unrolling/optimization.
int adler32(int *data, int len) { 
  // S1 is the sum, S2 is the weighted sum
  // Use int for S1 and S2 to respect the constraint, though performance might be affected
  int s1 = 1;
  int s2 = 0;
  int k;
  
  // Loop through the data in chunks
  while (len > 0) {
    // Process up to 5552 bytes at a time for efficient modulo operations
    int block_len = len > 5552 ? 5552 : len;
    len -= block_len;
    
    // Core processing loop
    for (k = 0; k < block_len; k++) {
      // if data[k] were negative, but data is initialized as positive.
      s1 += data[k];
      s2 += s1;
    }
    data += block_len;
    
    // Apply modulo operation
    s1 = s1 % ADLER_MOD; //s1 %= ADLER_MOD;
    s2 = s2 % ADLER_MOD; //s2 %= ADLER_MOD;
    
    // Ensure s1 and s2 remain positive after modulo if the result might be negative (though ADLER_MOD is large)
    if (s1 < 0) s1 += ADLER_MOD;
    if (s2 < 0) s2 += ADLER_MOD;
  }
  
  // Final result: (s2 * 65536) + s1
  // Cast to long for the intermediate multiplication to avoid int overflow
  return (int)(((long)s2 << 16) | s1);
}

// --- 2. Huffman Frequency Calculation ---
void calculate_frequency(int *data, int len, int *freq) {
  int i;
  
  // Initialize frequencies to zero
  for (i = 0; i < ALPHABET_SIZE; i++) {
    freq[i] = 0;
  }
  
  // Count the occurrences of each byte (0-255)
  for (i = 0; i < len; i++) {
    int symbol = data[i]; 
    // The index must be checked to ensure it's within bounds (0-255)
    if (symbol >= 0 && symbol < ALPHABET_SIZE) {
      freq[symbol]++; 
    }
  }
}

// --- Main Logic ---

// Input frequency array (Alphabet size 256)
int frequency_table[ALPHABET_SIZE/*256*/];

void initialize_data() {
  int i;
  // Fill the buffer with pseudo-random data to ensure varied frequencies and complex Adler results
  for (i = 0; i < DATA_SIZE; i++) {
    input_data[i] = (i * 37 + (i >> 5)) % 256; 
  }
}

void benchmark_miniz_core() {
  int i;
  
  for (i = 0; i < ITERATIONS; i++) {
    // 1. Calculate Adler-32 checksum
    int adler = adler32(input_data, DATA_SIZE); 
    
    // 2. Calculate Huffman Frequencies
    calculate_frequency(input_data, DATA_SIZE, frequency_table);
    
    // Accumulate Adler checksum for final verification
    total_adler_checksum += (long)adler;
    
    // Accumulate frequency sum to ensure frequency table calculation is not optimized away
    long freq_sum = 0;
    int j;
    for (j = 0; j < ALPHABET_SIZE; j++) {
      freq_sum += frequency_table[j];
    }
    total_adler_checksum += freq_sum;
  }
}

int main() {
  initialize_data();
  
  // Execute the benchmark
  benchmark_miniz_core();
  
  printf("Miniz Core (Adler32 & Huffman Freq), ");
  printf("Input Data Size: %d, ", DATA_SIZE);
  printf("Total Checksum (Adler + Freq Sum): %ld\n", total_adler_checksum); 
  
  return 0;
}
