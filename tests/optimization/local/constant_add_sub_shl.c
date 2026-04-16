
int add_0l(int x){
  return 0 + x;
}

int add_0r(int x){
  return x + 0;
}

int sub_0r(int x){
  return x - 0;
}

int shl_1r(int x){
  return x << 1;
}

int main(){
  assert(5, add_0l(5), "add_0l(5)");
  assert(5, add_0r(5), "add_0r(5)");

  assert(3, sub_0r(3), "sub_0r(3)");

  assert(14, shl_1r(7), "shl_1r(7)");

   return 0;
}
