
struct TEST_G {
  char a;
  short b;
  int c;
  long d;
} tg;

struct TEST_G2 {
  char a;
  short b;
  int c;
} tg2 = {1,2,3};

struct TEST_G3 {
  char a;
  short b;
  int c;
} tg3 = {1,2};

struct TEST_G4 {
  char a;
  short b;
  int c;
} tg4 = {1,2,3,4};


struct Tree_G{
  int val;
  struct Tree_G* lest;
  struct Tree_G* right;
};

int main(){
  struct TEST_L {
    int x;
    int y;
  } tl;
  tl.x = 1;
  tl.y = 2;
  assert(1, tl.x, "tl.x");
  assert(2, tl.y, "tl.y");

  struct TEST_L {
    int x;
    int y;
  } *ptl;
  ptl = &tl;
  assert(1, ptl->x, "ptl->x");
  assert(2, ptl->y, "ptl->y");

  tg.a = 10;
  tg.b = 20;
  tg.c = 30;
  tg.d = 40;
  assert(10, tg.a, "tg.a");
  assert(20, tg.b, "tg.b");
  assert(30, tg.c, "tg.c");
  assert(40, tg.d, "tg.d");

  assert(1, tg2.a, "tg2.a");
  assert(2, tg2.b, "tg2.b");
  assert(3, tg2.c, "tg2.c");

  assert(1, tg3.a, "tg3.a");
  assert(2, tg3.b, "tg3.b");
  assert(0, tg3.c, "tg3.c");

  assert(1, tg4.a, "tg4.a");
  assert(2, tg4.b, "tg4.b");
  assert(3, tg4.c, "tg4.c");

  struct Tree_L{
    int val;
    struct Tree_L* lest;
    struct Tree_L* right;
  };

  struct TEST_L2 {
    int x;
    short y;
    char z;
  } tl2 = {1,2,3};
  assert(1, tl2.x, "tl2.x");
  assert(2, tl2.y, "tl2.y");
  assert(3, tl2.z, "tl2.z");

  struct TEST_L3 {
    int x;
    short y;
    char z;
  } tl3 = {1,2};
  assert(1, tl3.x, "tl3.x");
  assert(2, tl3.y, "tl3.y");
  assert(0, tl3.z, "tl3.z");

  struct TEST_L4 {
    int x;
    short y;
    char z;
  } tl4 = {1,2,3,4};
  assert(1, tl4.x, "tl4.x");
  assert(2, tl4.y, "tl4.y");
  assert(3, tl4.z, "tl4.z");
  
  return 0;
}
