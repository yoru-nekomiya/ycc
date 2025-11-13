
int main(){
  int a; int b;
  a = 0; b = 0;
  if(b < 5)
    a = 1;
  else
    a = 2;
  assert(1, a, "if-then test");

  a = 0; b = 0;
  if(b < -1)
    a = 1;
  else
    a = 2;
  assert(2, a, "if-else test");

  a = 0; b = 0;
  if(b < 5){
    a = 1;
    b = 1;
  } else {
    a = 2;
    b = 2;
  }
  assert(1, a, "if-then (compound stmt) test");
  assert(1, b, "if-then (compound stmt) test");

  a = 0; b = 0;
  if(b < -1){
    a = 1;
    b = 1;
  } else {
    a = 2;
    b = 2;
  }
  assert(2, a, "if-else (compound stmt) test");
  assert(2, b, "if-else (compound stmt) test");
  return 0;
}
