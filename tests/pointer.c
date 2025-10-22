
int main(){
  int a;
  a = 100;
  int* b;
  b = &a;
  assert(100, *b);

  int** c;
  c = &b;
  assert(100, **c);
  
  return 0;
}
