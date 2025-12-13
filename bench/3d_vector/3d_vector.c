//#include <stdio.h>
//#include <time.h>

int printf();

// Structure for a 3D Vector
struct Vec3 {
  long x; // 64-bit integer fixed-point
  long y;
  long z;
};
/*
int V_COUNT = 500000; // Number of vectors
int ITERATIONS = 100; // Number of calculation iterations
int SCALE = 1000; // Fixed-point scale factor
*/
#define V_COUNT 500000
#define ITERATIONS 100
#define SCALE 1000

struct Vec3 vectors[V_COUNT/*500000*/];

// Calculates dot product (A . B) and updates A: A = A + (A . B)
void vector_op(int n) {
  for (int it = 0; it < ITERATIONS; it++) {
    for (int i = 0; i < n; i++) {
      // Use the next vector as B (sequential memory access pattern)
      int j = (i + 1) % n; 
      
      // Dot Product (Fixed-Point, 64-bit intermediate)
      long dot_prod = (vectors[i].x * vectors[j].x + 
		       vectors[i].y * vectors[j].y + 
		       vectors[i].z * vectors[j].z) / SCALE;
      
      // Add result to A
      vectors[i].x += dot_prod;
      vectors[i].y += dot_prod;
      vectors[i].z += dot_prod;
    }
  }
}

// Approximate calculation of the norm (length) for checksum
long calculate_norm_approx(struct Vec3* v) {
  // Norm^2 (V.x^2 + V.y^2 + V.z^2)
  long norm_sq = v->x * v->x + v->y * v->y + v->z * v->z;
  
  // Simple bit shift count to evaluate the order (avoids float sqrt)
  long approx_sqrt = 0;
  long temp = norm_sq;
  
  // Count bits (log2 approximation)
  while (temp > 0) {
    //temp >>= 1;
    temp = temp >> 1;
    approx_sqrt++;
  }
  // Returns the bit length of the result
  return approx_sqrt;
}

int main() {
  // Data initialization
  for (int i = 0; i < V_COUNT; i++) {
    vectors[i].x = i * SCALE / 1000 + 1;
    vectors[i].y = (i + 1) * SCALE / 1000 + 1;
    vectors[i].z = (i + 2) * SCALE / 1000 + 1;
  }
  
  vector_op(V_COUNT);
    
  long checksum = calculate_norm_approx(&vectors[V_COUNT - 1]);
    
  printf("3D Vector, ");
  printf("Checksum (Last Vector Norm Approx): %ld\n", checksum); 
  
  return 0;
}
