//#include <stdio.h>
//#include <stdlib.h>

int atoi();
int printf();

long move_count = 0;

void hanoi(int n, int from, int to, int via) {
  if (n == 0) return;
  
  hanoi(n - 1, from, via, to);
  
  move_count++; 
  
  hanoi(n - 1, via, to, from);
}

int main(int argc, char** argv) {
  int n = (argc >= 2) ? atoi(argv[1]) : 25;
  hanoi(n, 0, 2, 1);
  printf("Hanoi(%d) moves (Checksum): %ld\n", n, move_count);
  return 0;
}
