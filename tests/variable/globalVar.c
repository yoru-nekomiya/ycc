
int x;
int y;
int z = 3;
int w = -42;
int* p = &x; 
int garr[3] = {1,2,3};
int garr2[5] = {4,5,6};
int garr3[2] = {7,8,9};
char garr4[] = "Hello";
int garr5[2][2] = {{1,2},{3,4}};
int garr6[][2] = {{5,6},{7,8}};
int garr7[2][3] = {{1,2}, {3}};
int garr8[2][2][2] = {
    {{1,2}, {3,4}},
    {{5,6}, {7,8}}
  };
int garr9[2][3] = { {1,2,3} };
char garr10[4] = "Hi";
char garr11[] = {'H','e','l','l','o'};
int garr12[2][2] = {1,2,3,4};
int garr13[2][3] = { {1}, 2, 3 };
char* str = "Hello";
int* garr14 = garr + 1;

int main(){
  assert(0, x, "x");
  x = 10;
  assert(10, x, "x");
  assert(10, *p, "int* p = &x");

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

  assert(1, garr7[0][0], "garr7[0][0]");
  assert(2, garr7[0][1], "garr7[0][1]");
  assert(0, garr7[0][2], "garr7[0][2]");
  assert(3, garr7[1][0], "garr7[1][0]");
  assert(0, garr7[1][1], "garr7[1][1]");
  assert(0, garr7[1][2], "garr7[1][2]");

  assert(1, garr8[0][0][0], "garr8[0][0][0]");
  assert(2, garr8[0][0][1], "garr8[0][0][1]");
  assert(3, garr8[0][1][0], "garr8[0][1][0]");
  assert(4, garr8[0][1][1], "garr8[0][1][1]");
  assert(5, garr8[1][0][0], "garr8[1][0][0]");
  assert(6, garr8[1][0][1], "garr8[1][0][1]");
  assert(7, garr8[1][1][0], "garr8[1][1][0]");
  assert(8, garr8[1][1][1], "garr8[1][1][1]");

  assert(1, garr9[0][0], "garr9[0][0]");
  assert(2, garr9[0][1], "garr9[0][1]");
  assert(3, garr9[0][2], "garr9[0][2]");
  assert(0, garr9[1][0], "garr9[1][0]");
  assert(0, garr9[1][1], "garr9[1][1]");
  assert(0, garr9[1][2], "garr9[1][2]");

  assert('H', garr10[0], "garr10[0]");
  assert('i', garr10[1], "garr10[1]");
  assert('\0', garr10[2], "garr10[2]");
  assert(0, garr10[3], "garr10[3]");

  assert('H', garr11[0], "garr11[0]");
  assert('e', garr11[1], "garr11[1]");
  assert('l', garr11[2], "garr11[2]");
  assert('l', garr11[3], "garr11[3]");
  assert('o', garr11[4], "garr11[4]");

  assert(1, garr12[0][0], "garr12[0][0]");
  assert(2, garr12[0][1], "garr12[0][1]");
  assert(3, garr12[1][0], "garr12[1][0]");
  assert(4, garr12[1][1], "garr12[1][1]");

  assert(1, garr13[0][0], "garr13[0][0]");
  assert(0, garr13[0][1], "garr13[0][1]");
  assert(0, garr13[0][2], "garr13[0][2]");
  assert(2, garr13[1][0], "garr13[1][0]");
  assert(3, garr13[1][1], "garr13[1][1]");
  assert(0, garr13[1][2], "garr13[1][2]");

  assert('H', str[0], "str[0]");
  assert('e', str[1], "str[1]");
  assert('l', str[2], "str[2]");
  assert('l', str[3], "str[3]");
  assert('o', str[4], "str[4]");

  assert(2, garr14[0], "garr14[0]");
  
  return 0;
}
