
char g;

char add(char a, char b){
  return a+b;
}

char mul(char a){
  return a*2;
}

int main(){
  
  char a;
  char b;
  a = 127;
  b = 1;
  assert(126, a-b, "a-b");

  a = 10;
  b = -128;
  assert(-118, a+b, "a+b");
  
  char c[3];
  c[0] = 5;
  c[1] = 8;
  c[2] = 2;
  assert(15, c[0]+c[1]+c[2], "c[0]+c[1]+c[2]");

  char* p; char* q;
  p = c; q = &c[2];
  assert(13, *p + *(p+1), "*p + *(p+1)");
  assert(2, q - p);
  assert(-2, p - q);
  
  g = 1+2;
  assert(g, 3, "gobal char variable test");

  assert(-118, add(a,b), "add(a,b)");
  assert(20, mul(a), "mul(a)");
  return 0;
}
