//#include <stdio.h>
//#include <time.h>

int printf();
/*
int IMG_SIZE = 512; // 512x512 image
int KERNEL_SIZE = 3; // 3x3 kernel
int IMG_AREA = 512 * 512;
int ITERATIONS = 10;
*/
#define IMG_SIZE 512    // 512x512 image
#define KERNEL_SIZE 3   // 3x3 kernel
#define IMG_AREA 262144 //512 * 512
#define ITERATIONS 10

// Image data (stored as a 1D array for better cache performance)
int image[IMG_AREA/*512 * 512*/]; 
int output[IMG_AREA/*512 * 512*/]; 

// 3x3 Sharpening kernel (Fixed-point scale 1000)
int kernel[KERNEL_SIZE/*3*/][KERNEL_SIZE/*3*/] = {
  {0, -1000, 0},
  {-1000, 5000, -1000},
  {0, -1000, 0}
};
//int KERNEL_SCALE = 1000;
#define KERNEL_SCALE 1000

// 2D Convolution operation
void convolve(int size) {
  for (int it = 0; it < ITERATIONS; it++) {
    // Output becomes input for the next iteration (image is updated)
    for (int i = 1; i < size - 1; i++) { // Row
      for (int j = 1; j < size - 1; j++) { // Column
	long sum = 0; // Use 64-bit for intermediate calculation
	
	// Apply kernel (High spatial locality access)
	for (int ki = 0; ki < KERNEL_SIZE; ki++) {
	  for (int kj = 0; kj < KERNEL_SIZE; kj++) {
	    // Map 2D access to 1D index
	    int row = i + ki - 1;
	    int col = j + kj - 1;
            
	    // Multiply and accumulate
	    sum += (long)image[row * size + col] * (long)kernel[ki][kj];
	  }
	}
	// Store result in the output array, scaled down
	output[i * size + j] = (int)(sum / /*(long)*/KERNEL_SCALE);
      }
    }
    
    // Copy output to image for the next iteration
    for (int i = 0; i < IMG_AREA; i++) {
      image[i] = output[i];
    }
  }
}

int main() {
  // Data initialization (simple gradient)
  for (int i = 0; i < IMG_AREA; i++) {
    image[i] = i % 256; 
    output[i] = 0;
  }
  
  convolve(IMG_SIZE);
    
  // Checksum: Sum of all pixels in the final output image
  long checksum = 0;
  for (int i = 0; i < IMG_AREA; i++) {
    checksum += output[i];
  }
  
  printf("Image Convolution, ");
  printf("Output Sum Checksum: %lld\n", checksum); 
  
  return 0;
}
