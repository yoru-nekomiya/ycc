
int main(){
  int x;
  x = 0;
  int i;
  i = 0;
  
  while(i < 5){
    x = x+i;
    i = i+1;
  }
  assert(10, x, "while test");

  x = 0; i = 0;
  while(1){
    x++;
    break;
    i++;
  }
  assert(1, x, "x, break in while");
  assert(0, i, "i, break in while");
  
  x = 0;
  while(x < 3){
    while(1){
      x++;
      break;
    }
  }
  assert(3, x, "x, break in nested while");
  
  x = 0; i = 0;
  while(x < 3){
    i++;
    while(1){
      x++;
      break;
    }
    i++;
  }
  assert(3, x, "x, break in nested while 2");
  assert(6, i, "i, break in nested while 2");

  x = 0; i = 0;
  while(i < 5){
    i++;
    continue;
    x++;
  }
  assert(0, x, "x, continue in while");
  
  x = 0; i = 0;
  while(i < 5){
    i++;
    if(i == 3) continue;
    x++;
  }
  assert(4, x, "x, continue in while 2");
  
  return 0;
}
