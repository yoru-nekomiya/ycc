
int main(){
  int a;
  a = 2+3;
  assert(5, a, "a=2+3");

  int b;
  b = a;
  assert(5, b, "b=a");

  int c; int d;
  d = c = a;
  assert(5, c, "d=c=a");
  assert(5, d, "d=c=a");
  
  return 0;
}
