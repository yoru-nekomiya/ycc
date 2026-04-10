
int mul_8l(int x){
  return 8 * x;
}

int mul_8r(int x){
  return x * 8;
}

int mul_m64l(int x){
  return -64 * x;
}

int mul_m32r(int x){
  return x * -32;
}

int mul_1l(int x){
  return 1 * x;
}

int mul_m1l(int x){
  return -1 * x;
}

int mul_0l(int x){
  return 0 * x;
}

int mul_1r(int x){
  return x * 1;
}

int mul_m1r(int x){
  return x * -1;
}

int mul_0r(int x){
  return x * 0;
}

int mul_3l(int x){
  return 3 * x; //converted to lea
}

int mul_5l(int x){
  return 5 * x; //converted to lea
}

int mul_6l(int x){
  return 6 * x; //converted to lea+shl
}

int mul_7l(int x){
  return 7 * x; //converted to shl+sub
}

int mul_9l(int x){
  return 9 * x; //converted to lea
}

int mul_10l(int x){
  return 10 * x; //converted to lea+shl
}

int mul_11l(int x){
  return 11 * x; 
}

int mul_12l(int x){
  return 12 * x; //converted to lea+shl
}

int mul_33l(int x){
  return 33 * x; //converted to shl+add
}

int mul_34l(int x){
  return 34 * x; //converted to shl+add+shl
}

int mul_m34l(int x){
  return -34 * x; //converted to imul
}

//---
int mul_3r(int x){
  return x * 3; //converted to lea
}

int mul_5r(int x){
  return x * 5; //converted to lea
}

int mul_6r(int x){
  return x * 6; //converted to lea+shl
}

int mul_7r(int x){
  return x * 7; //converted to shl+sub
}

int mul_9r(int x){
  return x * 9; //converted to lea
}

int mul_10r(int x){
  return x * 10; //converted to lea+shl
}

int mul_11r(int x){
  return x * 11; 
}

int mul_12r(int x){
  return x * 12; //converted to lea+shl
}

int mul_33r(int x){
  return x * 33; //converted to shl+add
}

int mul_34r(int x){
  return x * 34; //converted to shl+add+shl
}

int mul_m34r(int x){
  return x * -34; //converted to imul
}

int main(){
  assert(24, mul_8l(3), "mul_8l(3)");
  assert(40, mul_8r(5), "mul_8r(5)");

  assert(128, mul_m64l(-2), "mul_m64l(-2)");
  assert(-96, mul_m32r(3), "mul_32r(3)");

  assert(7, mul_1l(7), "mul_1l(7)");
  assert(-7, mul_m1l(7), "mul_m1l(7)");
  assert(0, mul_0l(7), "mul_0l(7)");

  assert(9, mul_1r(9), "mul_1r(9)");
  assert(-9, mul_m1r(9), "mul_m1r(9)");
  assert(0, mul_0r(9), "mul_0r(9)");

  assert(15, mul_3l(5), "mul_3l(5)");
  assert(-65, mul_5l(-13), "mul_5l(-13)");
  assert(-18, mul_6l(-3), "mul_6l(-3)");
  assert(56, mul_7l(8), "mul_7l(8)");
  assert(-153, mul_9l(-17), "mul_9l(-17)");
  assert(70, mul_10l(7), "mul_10l(7)");
  assert(22, mul_11l(2), "mul_11l(2)");
  assert(60, mul_12l(5), "mul_12l(5)");
  assert(132, mul_33l(4), "mul_33l(4)");
  assert(238, mul_34l(7), "mul_34l(7)");
  assert(-238, mul_m34l(7), "mul_m34l(7)");

  assert(15, mul_3r(5), "mul_3r(5)");
  assert(-65, mul_5r(-13), "mul_5r(-13)");
  assert(-18, mul_6r(-3), "mul_6r(-3)");
  assert(56, mul_7r(8), "mul_7r(8)");
  assert(-153, mul_9r(-17), "mul_9r(-17)");
  assert(70, mul_10r(7), "mul_10r(7)");
  assert(22, mul_11r(2), "mul_11r(2)");
  assert(60, mul_12r(5), "mul_12r(5)");
  assert(132, mul_33r(4), "mul_33r(4)");
  assert(238, mul_34r(7), "mul_34r(7)");
  assert(-238, mul_m34r(7), "mul_m34r(7)");
}
