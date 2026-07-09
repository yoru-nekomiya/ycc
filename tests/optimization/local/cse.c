void assert(long expected, long actual, char* desc);

int test(int a, int b){
  return (a+b) * (a+b);
}

int main(){
  assert(9, test(1,2), "test(1,2)");
  return 0;
}
