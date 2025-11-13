
int return_10(){
  int a;
  a = 10;
  return a;
}

int add(int x, int y){
  return x + y;
}
/*
int add_6(int a, int b, int c, int d, int e, int f){
  return a+b+c+d+e+f;
}
*/

int g;
void void_test(void){
  g = 1;
  return;
}

void void_test2(){
  g=2;
}

int main(){
  assert(10, return_10(), "return_10()");
  assert(5, add(2,3), "add(2,3)");
  //assert(21, add_6(1,2,3,4,5,6));

  g=0;
  assert(0, g, "before void_test()");
  void_test();
  assert(1, g, "after void_test()");
  void_test2();
  assert(2, g, "before void_test2()");
  
  return 0;
}
