
int main(){
  int a;
  assert(4, sizeof(a));
  assert(4, sizeof(a+1));

  assert(4, sizeof(1));
  assert(4, sizeof(sizeof(1)));

  int* b; int** c;
  assert(8, sizeof(b));
  assert(8, sizeof(c));
  
  return 0;
  
}
