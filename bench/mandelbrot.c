//Mandelbrot (Fixed-Point)

//#include <stdio.h>
//#include <time.h>

int printf();

int main() {
  int SCALE = 4096; 
  int X_MIN = -8192; 
  int X_MAX = 4096;  
  int Y_MIN = -4096; 
  int Y_MAX = 4096;  
  int WIDTH = 800; 
  int HEIGHT = 400;
  int MAX_ITER = 255;
  int ESCAPE_SQUARED = 67108864; // 4.0 * SCALE * SCALE
  
  int x;
  int y;
  int step_x = (X_MAX - X_MIN) / WIDTH;
  int step_y = (Y_MAX - Y_MIN) / HEIGHT;
  long total_iterations = 0; 
  
  for (y = Y_MIN; y < Y_MAX; y += step_y) {
    for (x = X_MIN; x < X_MAX; x += step_x) {
      int zr = 0; int zi = 0;
      int tr = 0; int ti = 0; 
      int n;
      for (n = 0; n < MAX_ITER; n++) {
	if (tr + ti > ESCAPE_SQUARED) { 
	  break;
	}
        
	zi = (2 * zr * zi) / SCALE + y;
	zr = (tr - ti) / SCALE + x;
	
	tr = zr * zr;
	ti = zi * zi;
      }
      total_iterations += n;
    }
  }
    printf("Total Iterations: %ld\n", total_iterations);    
    return 0;
}
