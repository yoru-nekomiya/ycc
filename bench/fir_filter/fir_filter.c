//#include <stdio.h>

int printf();

// --- Configuration ---
/*
int SIGNAL_SIZE = 100000; // Size of the input signal
int COEFF_SIZE = 32;      // Number of filter coefficients (taps)
int ITERATIONS = 200;     // Number of times to run the filter
*/
#define SIGNAL_SIZE 100000 // Size of the input signal
#define COEFF_SIZE 32      // Number of filter coefficients (taps)
#define ITERATIONS 200     // Number of times to run the filter

// Input signal and filter coefficients
int signal[SIGNAL_SIZE + COEFF_SIZE /*100000 + 32*/]; 
int coeffs[COEFF_SIZE/*32*/];
int output[SIGNAL_SIZE/*100000*/];

// --- Initialization ---
void init_data() {
    int i;
    // Initialize signal with a simple repeating pattern
    for (i = 0; i < SIGNAL_SIZE + COEFF_SIZE; i++) {
        signal[i] = (i % 100) - 50;
    }
    // Initialize coefficients (e.g., a simple low-pass filter pattern)
    for (i = 0; i < COEFF_SIZE; i++) {
        coeffs[i] = (i % 5) + 1;
    }
}

// --- Core FIR Filter Calculation ---
// This is where GCC/Clang will perform heavy loop unrolling and SIMD vectorization.
void apply_fir_filter() {
  int i;
  int j;
    
    // Process the signal
    for (i = 0; i < SIGNAL_SIZE; i++) {
        long sum = 0;
        
        // Inner loop: This is the hotspot for optimization.
        // It's a "Multiply-Accumulate" (MAC) operation.
        for (j = 0; j < COEFF_SIZE; j++) {
            // sum += signal[i + j] * coeffs[j]
            // We use long long for sum to prevent overflow during accumulation.
            sum += (long)signal[i + j] * coeffs[j];
        }
        
        // Store the lower 31 bits of the result (simple scaling/clamping simulation)
        output[i] = (int)(sum % 2147483647);
    }
}

int main() {
    int it;
    long total_checksum = 0;

    init_data();

    // Benchmark Execution
    for (it = 0; it < ITERATIONS; it++) {
        apply_fir_filter();
    }

    // Calculate Checksum
    for (it = 0; it < SIGNAL_SIZE; it++) {
        total_checksum += output[it];
    }

    printf("Integer FIR Filter (DSP Core), ");
    printf("Signal Size: %d, Taps: %d, ", SIGNAL_SIZE, COEFF_SIZE);
    printf("Checksum (Output Sum): %ld\n", total_checksum);

    return 0;
}
