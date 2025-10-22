
int main(){
  int x;
  int i;
  x = 0;
  for(i = 0; i < 10; i = i+1){
    x = x+1;
  }
  assert(10, x);
  return 0;
}
