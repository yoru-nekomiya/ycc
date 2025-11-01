
int main(){
  int a;
  a = 0;
  ++a;
  assert(1, a);
  assert(2, ++a);

  --a;
  assert(1, a);
  assert(0, --a);
  
  return 0;
}
