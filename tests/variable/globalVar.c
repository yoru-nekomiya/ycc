
int x;
int y;

int main(){
  x = 10;
  assert(10, x);

  int x;
  x = 1;
  assert(1, x);

  int a;
  a = 2;
  y = -5;
  assert(-3, a + y);
  
  return 0;
}
