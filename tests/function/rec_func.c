
int kaijo(int a){
  if(a == 0)
    return 1;
  int b;
  b = a * kaijo(a-1);
  return b;
}

int fib(int a){
  if(a == 0)
    return 0;
  else if(a == 1)
    return 1;
  else return (fib(a-1)+fib(a-2));
}

int main(){
  assert(120, kaijo(5));
  assert(5040, kaijo(7));

  assert(55, fib(10));
  return 0;
}
