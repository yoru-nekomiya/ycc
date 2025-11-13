
int main(){
  assert(1, 1<2, "1<2");
  assert(0, 2<1, "2<1");
  assert(1, 2>1, "2>1");
  assert(0, 1>2, "1>2");

  assert(1, 3<=5, "3<=5");
  assert(1, 3<=3, "3<=3");
  assert(0, 5<=3, "5<=3");
  
  assert(1, 5>=3, "5>=3");
  assert(1, 5>=5, "5>=5");
  assert(0, 5>=8, "5>=8");

  assert(1, 3==3, "3==3");
  assert(0, 3==7, "3==7");

  assert(1, 5!=6, "5!=6");
  assert(0, 9!=9, "9!=9");
  return 0;
}
