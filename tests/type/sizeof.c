
int foo(int x, int* y){
  assert(4, sizeof(x), "sizeof(x) in function");
  assert(8, sizeof(y), "sizeof(y) in function");
  return 0;
}

int main(){
  int a;
  assert(4, sizeof(a), "sizeof(a)");
  assert(4, sizeof(a+1), "sizeof(a+1)");

  assert(4, sizeof(1), "sizeof(1)");
  assert(4, sizeof(sizeof(1)), "sizeof(sizeof(1))");

  int* b; int** c;
  assert(8, sizeof(b), "sizeof(b)");
  assert(8, sizeof(c), "sizeof(c)");

  int arr[10];
  assert(4*10, sizeof(arr), "sizeof(arr)");
  assert(4, sizeof(arr[0]), "sizeof(arr[0])");

  int arr2[3][4];
  assert(3*4*4, sizeof(arr2), "sizeof(arr2)");
  assert(4*4, sizeof(arr2[0]), "sizeof(arr2[0])");
  assert(4, sizeof(arr2[0][0]), "sizeof(arr2[0][0])");

  char d;
  short e;
  long f;
  assert(1, sizeof(d), "sizeof(d)");
  assert(2, sizeof(e), "sizeof(e)");
  assert(8, sizeof(f), "sizeof(f)");
  return 0;
}
