
int main(){
  int x;
  int i;
  x = 0;
  for(i = 0; i < 10; i = i+1){
    x = x+1;
  }
  assert(10, x, "for test");

  x = 0;
  for(int j = 0; j < 5; j++){
    x += 1;
  }
  assert(5, x, "init decl in for test");
  
  return 0;
}
