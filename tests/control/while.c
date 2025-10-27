
int main(){
  int x;
  x = 0;
  int i;
  i = 0;
  while(i < 5){
    x = x+i;
    i = i+1;
  }
  assert(10, x);
  return 0;
}
