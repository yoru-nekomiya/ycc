//#include <stdio.h>

int printf();

// Fixed Point Configuration
int FP_BITS = 10;  // 10 bits for the fractional part (Scale factor = 1024)
int SCALE = 1024;  // 2^FP_BITS

// Scene Configuration
int WIDTH = 128;
int HEIGHT = 128;
int MAX_ITERATIONS = 100;  
int NUM_SPHERES = 3;

long NO_HIT_DIST = 9223372036854775807;

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

// Fixed Point Square Root (Approximation)
long fp_sqrt_approx(long n) {
  long a = (n > 0) ? (n + SCALE) >> 1 : 0;
  for (int i = 0; i < 5; i++) {
    long next_a = fp_div(n, a);
    a = (a + next_a) >> 1;
  }
  return a;
}


// ----------------------------------------------------
// Vector/Sphere Structures
// ----------------------------------------------------

struct Vec3 {
  long x;
  long y;
  long z;
};

struct Sphere {
  struct Vec3 center;
  long radius_sq; 
  int color_r;
  int color_g;
  int color_b;
};

// Global Scene Definition
struct Sphere scene[] = {
  {{0, 0, 0}, /*fp_mul(3 * SCALE, 3 * SCALE)*/0, 255, 0, 0},        
  {{10000, 10000, 10000}, /*fp_mul(2 * SCALE, 2 * SCALE)*/0, 0, 255, 0}, 
  {{-10000, 5000, 5000}, /*fp_mul(4 * SCALE, 4 * SCALE)*/0, 0, 0, 255} 
};

void initialize_scene_data() {
    long r1 = 3 * SCALE;
    scene[0].radius_sq = fp_mul(r1, r1);

    long r2 = 2 * SCALE;
    scene[1].radius_sq = fp_mul(r2, r2);
    
    long r3 = 4 * SCALE;
    scene[2].radius_sq = fp_mul(r3, r3);
}

// ----------------------------------------------------
// Core Ray Tracing Functions
// ----------------------------------------------------

// Vector Subtraction: *out = *a - *b
void vec_sub(struct Vec3 *a, struct Vec3 *b, struct Vec3 *out) {
  out->x = a->x - b->x;
  out->y = a->y - b->y;
  out->z = a->z - b->z;
}

// Vector Dot Product: A . B
long vec_dot(struct Vec3 *a, struct Vec3 *b) {
  return fp_mul(a->x, b->x) + fp_mul(a->y, b->y) + fp_mul(a->z, b->z);
}

// Vector Length Squared
long vec_len_sq(struct Vec3 *a) {
  return vec_dot(a, a);
}

// Ray-Sphere Intersection
long intersect_sphere(struct Vec3 *ray_origin, struct Vec3 *ray_dir, struct Sphere *s) {
  struct Vec3 L; 
  vec_sub(&(s->center), ray_origin, &L); 
  
  // L . D (Fixed Point)
  long tca = vec_dot(&L, ray_dir); 
  
  if(tca < 0){
    return NO_HIT_DIST;
  }
  // L^2
  long d_sq = vec_len_sq(&L);
  
  // tca^2
  long tca_sq = fp_mul(tca, tca);
  
  // d_sq - tca_sq
  long thc_sq = d_sq - tca_sq;
  
  if (thc_sq > s->radius_sq){
    return NO_HIT_DIST;
  }
  
  long thc = fp_sqrt_approx(s->radius_sq - thc_sq); 
  
  long t = tca - thc;
  
  //return (t > 0) ? t : NO_HIT_DIST;
  if(t > 0){
    return t;
  } else {
    return NO_HIT_DIST;
  }
}

// Main trace function
int trace_ray(struct Vec3 *ray_origin, struct Vec3 *ray_dir, long *hit_t) {
  long closest_t = NO_HIT_DIST;
  int hit_sphere_index = -1;
    
  for (int i = 0; i < NUM_SPHERES; i++) {
    long t = intersect_sphere(ray_origin, ray_dir, &scene[i]);
    //printf("t=%ld, ", t);
    if (t > 0 && t < closest_t) {
      closest_t = t;
      hit_sphere_index = i;
    }
  }
  
  *hit_t = closest_t; 
  
  return hit_sphere_index;
}

// ----------------------------------------------------
// Main Render Loop
// ----------------------------------------------------

int image_buffer[/*WIDTH*/128 * /*HEIGHT*/128 * 3];

void render_scene() {
  // Camera settings 
  struct Vec3 cam_origin = {0, 0, -20000}; 
  long fov = 1 * SCALE; 
  
  int x; int y;
  
  for (y = 0; y < HEIGHT; y++) {
    for (x = 0; x < WIDTH; x++) {
      
      // --- 1. Calculate Ray Direction ---
      
      long dx = (long)((x - WIDTH / 2) * SCALE / 16);
      long dy = (long)((HEIGHT / 2 - y) * SCALE / 16);
      
      // struct Vec3 ray_dir = {dx, dy, fov};
      struct Vec3 ray_dir;
      ray_dir.x = dx;
      ray_dir.y = dy;
      ray_dir.z = fov;
      
      // --- 2. Trace the Ray ---
      long hit_t = 0;
      int hit_index = trace_ray(&cam_origin, &ray_dir, &hit_t);
      
      // --- 3. Shade the Pixel ---
      int r = 0; int g = 0; int b = 0;
      if (hit_index != -1) {
	r = scene[hit_index].color_r;
	g = scene[hit_index].color_g;
	b = scene[hit_index].color_b;
      } 
      
      int pixel_index = (y * WIDTH + x) * 3;
      image_buffer[pixel_index] = r;
      image_buffer[pixel_index + 1] = g;
      image_buffer[pixel_index + 2] = b;
    }
  }
}

// ----------------------------------------------------
// Main Function and Checksum
// ----------------------------------------------------

int main() {
  long total_checksum = 0;
  int i;

  initialize_scene_data();
  
  for (i = 0; i < MAX_ITERATIONS; i++) {
    render_scene();
  }
  
  for (i = 0; i < WIDTH * HEIGHT * 3; i++) {
    total_checksum += image_buffer[i];
  }

  printf("Simple Ray Tracer (Fixed-Point), ");
  printf("Checksum (Total RGB Sum): %ld\n", total_checksum);

  return 0;
}
