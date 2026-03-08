
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


int main(){
  assert(24, mul_8l(3), "mul_8l(3)");
  assert(40, mul_8r(5), "mul_8r(5)");

  assert(128, mul_m64l(-2), "mul_m64l(-2)");
  assert(-96, mul_m32r(3), "mul_32r(3)");
}
