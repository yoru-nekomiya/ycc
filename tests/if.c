
int main(){
  int a; int b;
  a = 0; b = 0;
  if(b < 5)
    a = 1;
  else
    a = 2;
  assert(1, a);

  a = 0; b = 0;
  if(b < -1)
    a = 1;
  else
    a = 2;
  assert(2, a);

  a = 0; b = 0;
  if(b < 5){
    a = 1;
    b = 1;
  } else {
    a = 2;
    b = 2;
  }
  assert(1, a);
  assert(1, b);

  a = 0; b = 0;
  if(b < -1){
    a = 1;
    b = 1;
  } else {
    a = 2;
    b = 2;
  }
  assert(2, a);
  assert(2, b);
  return 0;
}
