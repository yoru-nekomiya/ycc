
short g;

short add(short a, short b){
  return a+b;
}

short mul(short a){
  return a * 2;
}

int main(){
  
  short a;
  short b;
  a = 32767;
  b = 1;
  assert(32766, a-b, "a-b");

  a = 1;
  b = -32768;
  assert(-32767, a+b, "a+b");
  
  short c[3];
  c[0] = 5;
  c[1] = 8;
  c[2] = 2;
  assert(15, c[0]+c[1]+c[2], "c[0]+c[1]+c[2]");

  short* p; short* q;
  p = c; q = &c[2];
  assert(13, *p + *(p+1), "*p + *(p+1)");
  assert(2, q-p);
  assert(-2, p-q);
  
  g = 1;
  assert(g, 1, "short global variable test");

  assert(-32767, add(a,b), "add(a,b)");
  assert(-2, mul(-1), "mul(-1)");

  return 0;
}
