
int inc(int* a){
  *a = *a + 1;
  return 0;
}

int main(){
  int a;
  a = 100;
  int* b;
  b = &a;
  assert(100, *b);

  int** c;
  c = &b;
  assert(100, **c);

  inc(&a);
  assert(101, a);
  inc(b);
  assert(102, *b);
  
  return 0;
}
