
int div_8(int x){
  return x / 8;
}

int div_m8(int x){
  return x / -8;
}

int div_2(int x){
  return x / 2;
}

int div_m2(int x){
  return x / -2;
}

int div_1(int x){
  return x / 1;
}

int div_m1(int x){
  return x / -1;
}

int div_5(int x){
  return x / 5;
}

int div_m5(int x){
  return x / -5;
}

int rem_2(int x){
  return x % 2;
}

int rem_m2(int x){
  return x % -2;
}

int rem_1(int x){
  return x % 1;
}

int rem_m1(int x){
  return x % -1;
}

int rem_7(int x){
  return x % 7;
}

int rem_m7(int x){
  return x % -7;
}

int main(){
  assert(15, div_8(120), "div_8(120)");
  assert(-4, div_8(-32), "div_8(-32)");
  assert(-15, div_m8(120), "div_m8(120)");
  assert(8, div_m8(-64), "div_m8(-64)");
  
  assert(-2, div_2(-5), "div_2(-5)");
  assert(2, div_m2(-5), "div_m2(-5)");
  
  assert(3, div_1(3), "div_1(3)");
  assert(-3, div_m1(3), "div_m1(3)");

  assert(2, div_5(10), "div_5(10)");
  assert(-2, div_m5(10), "div_m5(10)");
  assert(-2, div_5(-10), "div_5(-10)");
  assert(2, div_m5(-10), "div_m5(-10)");

  assert(1, rem_2(5), "rem_2(5)");
  assert(-1, rem_2(-5), "rem_2(5)");
  assert(1, rem_m2(5), "rem_2(5)");
  assert(-1, rem_m2(-5), "rem_2(5)");

  assert(0, rem_1(3), "rem_1(3)");
  assert(0, rem_m1(3), "rem_m1(3)");

  assert(3, rem_7(10), "rem_7(10)");
  assert(3, rem_m7(10), "rem_m7(10)");
  assert(-3, rem_7(-10), "rem_7(-10)");
  assert(-3, rem_m7(-10), "rem_m7(-10)");
}
