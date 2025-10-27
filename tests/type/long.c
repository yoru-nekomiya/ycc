
long g;

long add(long a, long b){
  return a+b;
}

long mul(long a){
  return a * 2;
}

int main(){
  
  long a;
  long b;
  a = 9223372036854775807;
  b = 1;
  assert(9223372036854775806, a-b);

  a = 1;
  b = -9223372036854775808;
  assert(-9223372036854775807, a+b);
  
  long c[3];
  c[0] = 5;
  c[1] = 8;
  c[2] = 2;
  assert(15, c[0]+c[1]+c[2]);

  long* p;
  p = c;
  assert(13, *p + *(p+1));

  g = 1;
  assert(g, 1);

  assert(-9223372036854775807, add(a,b));
  assert(-2, mul(-1));
  
  return 0;
}
