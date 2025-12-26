//#include <stdio.h>
//#include <time.h>

int printf();

int tarai(int x, int y, int z) {
  if (x <= y) {
    return y;
  }
  
  return tarai(
	       tarai(x - 1, y, z), 
	       tarai(y - 1, z, x), 
	       tarai(z - 1, x, y)
	       );
}

int main() {
  int X = 12;
  int Y = 6; 
  int Z = 0;
  /*
    int X = 14;
    int Y = 7;
    int Z = 0;
  */
  int result = tarai(X, Y, Z);
  
  printf("Result (Checksum): %d\n", result);
  return 0;
}
