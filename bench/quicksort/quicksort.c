//#include <stdio.h>
//#include <stdlib.h> 

int printf();

#define SIZE 100000 
int data[SIZE];

void swap(int* a, int* b) {
  int t = *a;
  *a = *b;
  *b = t;
}

int partition(int* arr, int low, int high) {
  int pivot = arr[high];
  int i = low - 1;
  
  for (int j = low; j <= high - 1; j++) {
    if (arr[j] <= pivot) {
      i++;
      swap(&arr[i], &arr[j]);
    }
  }
  swap(&arr[i + 1], &arr[high]);
  return i + 1;
}

void quicksort(int* arr, int low, int high) {
  if (low < high) {
    int pi = partition(arr, low, high);
    quicksort(arr, low, pi - 1);
    quicksort(arr, pi + 1, high);
  }
}

int main() {
  
  for (int i = 0; i < SIZE; i++) {
    data[i] = (i * 997 + 101) % 1000000; 
  }
    
  quicksort(data, 0, SIZE - 1);    
  int check_result = data[0]; 
  
  printf("Smallest Element (Checksum): %d\n", check_result); 
  return 0;
}
