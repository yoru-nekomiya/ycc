
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

  assert(1, sizeof(char), "sizeof(char)");
  assert(2, sizeof(short), "sizeof(short)");
  assert(4, sizeof(int), "sizeof(int)");
  assert(8, sizeof(long), "sizeof(long)");
  assert(8, sizeof(char*), "sizeof(char*)");
  assert(8, sizeof(short*), "sizeof(short*)");
  assert(8, sizeof(int*), "sizeof(int*)");
  assert(8, sizeof(long*), "sizeof(long*)");
  assert(8, sizeof(int**), "sizeof(int**)");
  assert(16, sizeof(int[4]), "sizeof(int[4])");
  assert(48, sizeof(int[3][4]), "sizeof(int[3][4])");
  assert(8, sizeof(struct {int a; int b;}), "sizeof(struct {int a; int b;})");
  assert(24, sizeof(struct {char a; long b; int c;}), "sizeof(struct {char a; long b; int c;})");

  struct TEST{char a; int b; long c;};
  assert(16, sizeof(struct TEST), "sizeof(struct TEST)");
  return 0;
}
