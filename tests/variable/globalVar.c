
int x;
int y;
int z = 3;
int w = -42;
int garr[3] = {1,2,3};
int garr2[5] = {4,5,6};
int garr3[2] = {7,8,9};
char garr4[] = "Hello";
int garr5[2][2] = {{1,2},{3,4}};
int garr6[][2] = {{5,6},{7,8}};

int main(){
  assert(0, x, "x");
  x = 10;
  assert(10, x, "x");

  int x;
  x = 1;
  assert(1, x, "x (local)");

  int a;
  a = 2;
  y = -5;
  assert(-3, a + y, "a+y");

  assert(3, z, "int z = 3");
  assert(-42, w, "int w = -42");
  assert(1, garr[0], "garr[0]");
  assert(2, garr[1], "garr[1]");
  assert(3, garr[2], "garr[2]");

  assert(4, garr2[0], "garr2[0]");
  assert(5, garr2[1], "garr2[1]");
  assert(6, garr2[2], "garr2[2]");
  assert(0, garr2[3], "garr2[3]");
  assert(0, garr2[4], "garr2[4]");

  assert(7, garr3[0], "garr3[0]");
  assert(8, garr3[1], "garr3[1]");

  assert('H', garr4[0], "garr4[0]");
  assert('e', garr4[1], "garr4[1]");
  assert('l', garr4[2], "garr4[2]");
  assert('l', garr4[3], "garr4[3]");
  assert('o', garr4[4], "garr4[4]");
  assert('\0', garr4[5], "garr4[5]");

  assert(1, garr5[0][0], "garr5[0][0]");
  assert(2, garr5[0][1], "garr5[0][1]");
  assert(3, garr5[1][0], "garr5[1][0]");
  assert(4, garr5[1][1], "garr5[1][1]");

  assert(5, garr6[0][0], "garr6[0][0]");
  assert(6, garr6[0][1], "garr6[0][1]");
  assert(7, garr6[1][0], "garr6[1][0]");
  assert(8, garr6[1][1], "garr6[1][1]");
  
  return 0;
}
