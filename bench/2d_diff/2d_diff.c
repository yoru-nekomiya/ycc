//#include <stdio.h>

int printf();

// --- Define Configuration ---
#define GRID_SIZE 512
#define ITERATIONS 200
#define FP_BITS 10
#define SCALE 1024       // Fixed-point scale (2^10)
#define DIFF_COEFF 256   // Diffusion coefficient (0.25 * SCALE)

// Two grids for Double Buffering (Current and Next state)
int grid_a[GRID_SIZE * GRID_SIZE];
int grid_b[GRID_SIZE * GRID_SIZE];

// --- Initialization ---
void init_grid() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            int idx = i * GRID_SIZE + j;
            // Initialize with a heat source in the center
            if (i > GRID_SIZE / 4 && i < 3 * GRID_SIZE / 4 && 
                j > GRID_SIZE / 4 && j < 3 * GRID_SIZE / 4) {
                grid_a[idx] = 100 * SCALE; // High initial temperature
            } else {
                grid_a[idx] = 0;
            }
            grid_b[idx] = 0; // Clear second grid
        }
    }
}

// --- Core Diffusion Kernel (Stencil Computation) ---
// Next = Current + D * (Up + Down + Left + Right - 4*Current)
void diffuse(int *current, int *next) {
    // The loop bounds skip the edges (i=0, i=GRID_SIZE-1, etc.)
    for (int i = 1; i < GRID_SIZE - 1; i++) {
        for (int j = 1; j < GRID_SIZE - 1; j++) {
            int idx = i * GRID_SIZE + j;
            
            // Get values from the 5-point stencil (Center, Up, Down, Left, Right)
            long center = (long)current[idx];
            long up     = (long)current[idx - GRID_SIZE];
            long down   = (long)current[idx + GRID_SIZE];
            long left   = (long)current[idx - 1];
            long right  = (long)current[idx + 1];
            
            // Calculate Laplacian: a measure of local curvature/difference
            long laplacian = up + down + left + right - (center * 4);
            
            // Fixed-point update: next = center + (DIFF_COEFF * laplacian) / SCALE
            // The intermediate long long is necessary for the fixed-point multiplication
            long delta = (laplacian * DIFF_COEFF) / SCALE;
            next[idx] = (int)(center + delta);
        }
    }
}

int main() {
    int it;
    int *src = grid_a;
    int *dst = grid_b;
    long total_checksum = 0;
    int i;

    init_grid();

    // Benchmark Loop
    for (it = 0; it < ITERATIONS; it++) {
        diffuse(src, dst);
        
        // Swap buffers for the next iteration
        int *temp = src;
        src = dst;
        dst = temp;
    }

    // Calculate Checksum (Sum of the final grid state)
    for (i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        total_checksum += (long)src[i];
    }

    printf("Benchmark: 2D Diffusion (5-point Stencil), ");
    printf("Grid: %dx%d, Iterations: %d, ", GRID_SIZE, GRID_SIZE, ITERATIONS);
    printf("Checksum (Total Energy Sum): %ld\n", total_checksum);

    return 0;
}
