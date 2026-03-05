
int return_42(){
  return 42;
}

int branch(){
  if(1){
    return 1;
  }
  return 0;
}

int main(){
  int a = 20;
  int b = 22;
  assert(42, a+b, "20+22");
  assert(42, 20+22, "20+22");
  assert(42, 45-3, "45-3");
  assert(56, 7*8, "7*8");
  assert(5, 100/20, "100/20");
  assert(1, 100%3, "100%3");

  assert(1, 2==2, "2==2");
  assert(0, 2==3, "2==3");
  assert(0, 2!=2, "2!=2");
  assert(1, 2!=3, "2!=3");
  
  assert(0, 2<1, "2<1");
  assert(1, 2<3, "2<3");
  assert(0, 2<=1, "2<=1");
  assert(1, 2<=2, "2<=2");
  assert(1, 2<=3, "2<=3");

  assert(8, 1<<3, "1<<3");
  assert(1, 8>>3, "8>>3");

  assert(7, 3 | 5, "3|5");
  assert(1, 3 & 5, "3&5");
  assert(6, 3 ^ 5, "3^5");
  
  return 0;
}
