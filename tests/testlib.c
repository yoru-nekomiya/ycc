#include <stdio.h>
#include <stdlib.h>

void assert(int expected, int actual/*, char* desc*/){
  if(expected != actual){
    //fprintf(stderr, "Test failed: %s\n", desc);
    printf("-- Test failed: expected %d, but got %d\n", expected, actual);
    exit(1);
  } /*else {
    printf("Test passed: %s\n", desc);
    }*/
}
