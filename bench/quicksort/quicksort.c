//#include <stdio.h>
//#include <time.h>
//#include <stdlib.h> 

int printf();

//#define SIZE 100000 
//int data[SIZE];
int data[100000];

void swap(int* a, int* b) {
  int t = *a;
  *a = *b;
  *b = t;
}

int partition(int* arr, int low, int high) {
  int pivot = arr[high];
  int i = low - 1;
  int j;
  
  for (j = low; j <= high - 1; j++) {
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
  int S = 100000;
  int i;
  int check_result;
  
  for (i = 0; i < S; i++) {
    data[i] = (i * 997 + 101) % 1000000; 
  }
    
  quicksort(data, 0, S - 1);    
  check_result = data[0]; 
  
  printf("Smallest Element: %d\n", check_result); 
  return 0;
}
