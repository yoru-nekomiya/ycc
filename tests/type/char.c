
char g;

int main(){
  
  char a;
  char b;
  a = 127;
  b = 1;
  assert(126, a-b);

  a = 10;
  b = -128;
  assert(-118, a+b);
  
  char c[3];
  c[0] = 5;
  c[1] = 8;
  c[2] = 2;
  assert(15, c[0]+c[1]+c[2]);

  char* p;
  p = c;
  assert(13, *p + *(p+1));

  g = 1+2;
  assert(g, 3);

  return 0;
}
