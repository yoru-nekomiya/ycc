//3D rotate - Fixed Point

//#include <stdio.h>
//#include <time.h>
//#include <math.h> 

int printf();
/*
int SCALE = 10000; 
int V_COUNT = 100000;
int ITERATIONS = 100;
*/
#define SCALE 10000
#define V_COUNT 100000
#define ITERATIONS 100

// cos(0.5 rad) ≈ 0.87758, sin(0.5 rad) ≈ 0.47942
// cos_val = 8776, sin_val = 4794
/*
int COS_VAL = 8776; 
int SIN_VAL = 4794; 
*/
#define COS_VAL 8776 
#define SIN_VAL 4794

long x[V_COUNT/*100000*/];
long y[V_COUNT/*100000*/];
long z[V_COUNT/*100000*/];

void rotate_z(int n, int cos_val, int sin_val) {
  for (int it = 0; it < ITERATIONS; it++) {
    for (int i = 0; i < n; i++) {
      // X' = X*cos - Y*sin
      long new_x = ((x[i] * cos_val) / SCALE) - ((y[i] * sin_val) / SCALE);
      
      // Y' = X*sin + Y*cos
      long new_y = ((x[i] * sin_val) / SCALE) + ((y[i] * cos_val) / SCALE);
      
      // Z' = Z
      
      x[i] = new_x;
      y[i] = new_y;
    }
  }
  return;
}

int main() {
  for (int i = 0; i < V_COUNT; i++) {
    x[i] = (i * 10) / 1000 * SCALE;
    y[i] = (i * 5) / 1000 * SCALE;
    z[i] = (i * 2) / 1000 * SCALE;
  }
  
  rotate_z(V_COUNT, COS_VAL, SIN_VAL);
  
  printf("3D Rotation (Fixed-Point), ");
  printf("Checksum (X+Y+Z): %lld\n", x[V_COUNT-1] + y[V_COUNT-1] + z[V_COUNT-1]); 
  
  return 0;
}
