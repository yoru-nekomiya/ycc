//#include <stdio.h>
//#include <stdlib.h>

int printf();
int exit();

void assert(long expected, long actual, char* desc){
  if(expected != actual){
    printf("-- Test failed: %s (expected %ld, but got %ld)\n", desc, expected, actual);
    exit(1);
  } 
}
