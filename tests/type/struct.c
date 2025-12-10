
struct TEST_G {
  char a;
  short b;
  int c;
  long d;
} tg;

int main(){
  struct TEST_L {
    int x;
    int y;
  } tl;
  tl.x = 1;
  tl.y = 2;
  assert(1, tl.x, "tl.x");
  assert(2, tl.y, "tl.y");

  tg.a = 10;
  tg.b = 20;
  tg.c = 30;
  tg.d = 40;
  assert(10, tg.a, "tg.a");
  assert(20, tg.b, "tg.b");
  assert(30, tg.c, "tg.c");
  assert(40, tg.d, "tg.d");
  
  
  return 0;
}
