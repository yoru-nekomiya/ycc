
int func(){
  return 1 || 0;
}

int main(){
  
  assert(0, 0, "0");
  assert(42, 42, "42");
  assert(-42, -42, "-42");
  
  assert(2, 1+1, "1+1");
  assert(9, 10-1, "10-1");
  assert(4, +5+(-1), "+5+(-1)");
  assert(15, 3*5, "3*5");
  assert(5, 10/2, "10/2");
  assert(3, (1+2) * (8/4) / (3-1), "(1+2) * (8/4) / (3-1)");

  assert(1, 1||2, "1||2");
  assert(1, 1||0, "1||0");
  assert(0, 0||0, "0||0");
  assert(1, -3||0, "-3||0");
  int x = 0;
  1 || (x = 1);
  assert(0, x, "x (LOGOR short-circuit test)");
  assert(1, 1 || (0 && (x = 1)), "deep short-circuit OR");

  
  assert(1, 1&&2, "1&&2");
  assert(0, 1&&0, "1&&0");
  assert(0, 0&&0, "0&&0");
  assert(1, -3&&5, "-3&&5");
  0 && (x = 1);
  assert(0, x, "x (LOGAND short-circuit test)");

  assert(1, (1||0)&&(0||1), "(1||0)&&(0||1)");
  assert(0, (1&&0)||(0&&1), "(1&&0)||(0&&1)");
  assert(1, 1 || 0 && 0, "1 || 0 && 0");
  assert(0, (1 || 0) && 0, "(1 || 0) && 0");

  assert(1, func(), "return with ||");

  assert(4, 1<<2, "1<<2");
  assert(1, 4>>2, "4>>2");
  assert(5, 5<<0, "5<<0");
  assert(5, 5>>0, "5>>0");
  assert(-2, -8>>2, "-8>>2");
  assert(-1, -1>>1, "-1>>1");

  assert(8, 1 << 2 << 1, "1<<2<<1");
  assert(1, 16 >> 2 >> 2, "16>>2>>2");
  assert(4, (8 >> 2) << 1, "(8>>2)<<1");
  assert(-4, (-16 >> 2), "-16>>2");

  int k = 3;
  assert(8, 1 << k, "1<<k");
  assert(2, 8 >> (k=2), "8>>(k=2)");
  
  return 0;
}
