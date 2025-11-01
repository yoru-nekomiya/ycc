
int main(){
  int a;
  a = 2+3;
  assert(5, a);

  int b;
  b = a;
  assert(5, b);

  int c; int d;
  d = c = a;
  assert(5, c);
  assert(5, d);
  
  return 0;
}
