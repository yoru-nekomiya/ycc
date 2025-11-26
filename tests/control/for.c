
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

  x = 0; int y = 0;
  for(int j = 0; j < 5; j++){
    x += 1;
    break;
    y += 2;
  }
  assert(1, x, "x, break in for");
  assert(0, y, "y, break in for");

  x = 0;
  for(int j = 0; j < 5; j++){
    for(int k = 0; k < 3; k++){
      x++;
      break;
    }
  }
  assert(5, x, "x, break in nested for");
  
  return 0;
}
