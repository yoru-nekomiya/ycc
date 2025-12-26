//#include <stdio.h>

int printf();

// --- Define Configuration ---
#define NUM_PARTICLES 1024
#define ITERATIONS 50
#define FP_BITS 10
#define SCALE 1024

// ----------------------------------------------------
// Fixed Point Math Helpers 
// ----------------------------------------------------

// Fixed Point Multiplication: (A * B) / SCALE
long fp_mul(long a, long b) {
  return (a * b) >> FP_BITS; 
}

// Fixed Point Division: (A * SCALE) / B
long fp_div(long a, long b) {
  if (b == 0) return 0;
  return (a << FP_BITS) / b;
}

// Fixed Point Square Root (Approximation, 5 iterations)
long fp_sqrt_approx(long n) {
  // Initial guess
  long a = (n > 0) ? (n + SCALE) >> 1 : 0; 
  // 5 iterations of the Babylonian method
  for (int i = 0; i < 5; i++) {
    long next_a = fp_div(n, a);
    a = (a + next_a) >> 1;
  }
  return a;
}

// ----------------------------------------------------
// Data Structures
// ----------------------------------------------------

// 3D Vector for position, velocity, and force
struct Vec3 {
  long x;
  long y;
  long z;
};

// Global particle arrays
struct Vec3 position[NUM_PARTICLES];
struct Vec3 velocity[NUM_PARTICLES];
struct Vec3 force[NUM_PARTICLES];

// --- Initialization ---
void init_particles() {
  for (int i = 0; i < NUM_PARTICLES; i++) {
    // Initial positions (scaled integer values)
    position[i].x = (long)i * 10 * SCALE;
    position[i].y = (long)(i * 2) * 10 * SCALE;
    position[i].z = 0;
    
    // Clear initial velocity and force
    velocity[i].x = 0;
    velocity[i].y = 0;
    velocity[i].z = 0;
    
    force[i].x = 0;
    force[i].y = 0;
    force[i].z = 0;
  }
}

// ----------------------------------------------------
// Core N-Body Simulation
// ----------------------------------------------------

// Calculates forces between all particle pairs and accumulates them in force[i]
void calculate_forces(int n) {
  int i; int j;
  // Softening parameter (avoids singularity at r=0)
  long softening_sq = fp_mul(1 * SCALE, 1 * SCALE); 
  
  // Clear forces for this step
  for (i = 0; i < n; i++) {
    force[i].x = 0;
    force[i].y = 0;
    force[i].z = 0;
    }
  
  // O(N^2) loop: Calculate interaction for every unique pair (j > i)
  for (i = 0; i < n; i++) {
    for (j = i + 1; j < n; j++) { 
      
      // 1. Distance vector (r = pos[j] - pos[i])
      long dx = position[j].x - position[i].x;
      long dy = position[j].y - position[i].y;
      long dz = position[j].z - position[i].z;
      
      // 2. Distance squared (dist_sq = dx^2 + dy^2 + dz^2 + softening^2)
      long dist_sq = fp_mul(dx, dx) + fp_mul(dy, dy) + fp_mul(dz, dz) + softening_sq;
      
      // 3. Inverse distance cubed (inv_dist_cube = 1 / (dist^3))
      long dist = fp_sqrt_approx(dist_sq);
      long inv_dist = fp_div(1 * SCALE, dist);
      long inv_dist_cube = fp_mul(fp_mul(inv_dist, inv_dist), inv_dist);
      
      // 4. Calculate force components (F = 1/dist^3 * r)
      long fx = fp_mul(dx, inv_dist_cube);
      long fy = fp_mul(dy, inv_dist_cube);
      long fz = fp_mul(dz, inv_dist_cube);
      
      // 5. Accumulate force (F_ji = -F_ij)
      force[i].x += fx;
      force[i].y += fy;
      force[i].z += fz;
      
      force[j].x -= fx;
      force[j].y -= fy;
      force[j].z -= fz;
    }
  }
}

// Updates particle positions and velocities (using Euler integration)
void integrate(int n) {
  long dt = 1 * SCALE / 100; // Time step dt = 0.01 (FP)
  
  for (int i = 0; i < n; i++) {
    // Update velocity: velocity += force * dt 
    velocity[i].x += fp_mul(force[i].x, dt);
    velocity[i].y += fp_mul(force[i].y, dt);
    velocity[i].z += fp_mul(force[i].z, dt);
    
    // Update position: position += velocity * dt
    position[i].x += fp_mul(velocity[i].x, dt);
    position[i].y += fp_mul(velocity[i].y, dt);
    position[i].z += fp_mul(velocity[i].z, dt);
  }
}

// ----------------------------------------------------
// Main Function and Checksum
// ----------------------------------------------------

int main() {
  int it;
  long total_checksum = 0;
  int i;
  
  init_particles();
  
  // Benchmark Loop
  for (it = 0; it < ITERATIONS; it++) {
    calculate_forces(NUM_PARTICLES);
    integrate(NUM_PARTICLES);
  }
  
  // Calculate Checksum (Sum of final particle positions)
  for (i = 0; i < NUM_PARTICLES; i++) {
    total_checksum += position[i].x + position[i].y + position[i].z;
  }
  
  printf("Benchmark: N-Body Simulation (O(N^2), Fixed-Point), ");
  printf("Particles: %d, Iterations: %d, ", NUM_PARTICLES, ITERATIONS);
  printf("Checksum (Final Position Sum): %ld\n", total_checksum);
  return 0;
}
