//#include <stdio.h>
int printf();

// --- Define Configuration ---
#define DATA_SIZE 65536    // 64K elements (must be even)
#define ROUNDS 32          // Standard TEA uses 32 rounds
#define ITERATIONS 30      // Total runs to increase load
#define TEA_DELTA 2654435769 // 0x9e3779b9

// Global data buffers
int data_buffer[DATA_SIZE];
int key[4]; // 128-bit key (4 x 32-bit)

// --- Initialization ---
void init_tea() {
  // Initialize data with simple pattern
  for (int i = 0; i < DATA_SIZE; i++) {
    data_buffer[i] = i * 12345;
  }
  // Initialize fixed key
  key[0] = 12345;
  key[1] = 23456;
  key[2] = 34567;
  key[3] = 45678;
}

// --- Core TEA Encryption Kernel ---
// Encrypts two 32-bit integers at a time
void tea_encrypt(int *v) {
  int v0 = v[0];
  int v1 = v[1];
  long sum = 0;
  int i;

  // Fixed 32 rounds loop - High potential for unrolling
  for (i = 0; i < ROUNDS; i++) {
    sum += TEA_DELTA;
    
    // v0 += ((v1 << 4) + key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + key[1])
    // We use int casts to ensure 32-bit wrap-around behavior if necessary
    v0 += (int)(((v1 << 4) + key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + key[1]));
    
    // v1 += ((v0 << 4) + key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + key[3])
    v1 += (int)(((v0 << 4) + key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + key[3]));
  }
  
  v[0] = v0;
  v[1] = v1;
}

int main() {
  long total_checksum = 0;
  
  init_tea();
  
  // Benchmark Execution Loop
  for (int it = 0; it < ITERATIONS; it++) {
    // Process buffer in chunks of 2 integers (64 bits)
    for (int i = 0; i < DATA_SIZE; i = i + 2) {
      // Passing a pointer to 2 elements
      tea_encrypt(&data_buffer[i]);
    }
  }

    // Calculate Checksum (Sum of encrypted buffer)
  for (int i = 0; i < DATA_SIZE; i++) {
    total_checksum += (long)data_buffer[i];
  }
  
  printf("Benchmark: Tiny Encryption Algorithm (TEA), ");
  printf("Data Size: %d, Rounds: %d, ", DATA_SIZE, ROUNDS);
  printf("Checksum (Encrypted Sum): %ld\n", total_checksum);
  
  return 0;
}
