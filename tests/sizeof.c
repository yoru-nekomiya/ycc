
int foo(int x, int* y){
  assert(4, sizeof(x));
  assert(8, sizeof(y));
  return 0;
}

int main(){
  int a;
  assert(4, sizeof(a));
  assert(4, sizeof(a+1));

  assert(4, sizeof(1));
  assert(4, sizeof(sizeof(1)));

  int* b; int** c;
  assert(8, sizeof(b));
  assert(8, sizeof(c));

  int arr[10];
  assert(4*10, sizeof(arr));
  assert(4, sizeof(arr[0]));

  int arr2[3][4];
  assert(3*4*4, sizeof(arr2));
  assert(4*4, sizeof(arr2[0]));
  assert(4, sizeof(arr2[0][0]));
  return 0;
  
}
