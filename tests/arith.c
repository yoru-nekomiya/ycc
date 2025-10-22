
int main(){
  assert(0, 0);
  assert(42, 42);
  assert(-42, -42);
  
  assert(2, 1+1);
  assert(9, 10-1);
  assert(4, +5+(-1));
  assert(15, 3*5);
  assert(5, 10/2);
  assert(3, (1+2) * (8/4) / (3-1));
  
  return 0;
}
