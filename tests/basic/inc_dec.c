
int main(){
  //pre inc/dec
  int a;
  a = 0;
  ++a;
  assert(1, a, "++a;a;");
  assert(2, ++a, "++a");
  assert(4, 1 + ++a, "1 + ++a");
  
  --a;
  assert(2, a, "--a;a;");
  assert(1, --a, "--a");
  assert(-1, --a - 1, "--a - 1");
  
  int arr[3];
  arr[0] = 0; arr[1] = 1; arr[2] = 2;
  int* pa; pa=arr;
  ++pa;
  assert(1, *pa, "++pa;*pa;");
  assert(2, *++pa, "*++pa");
  --pa;
  assert(1, *pa, "--pa;*pa;");
  assert(0, *--pa, "*--pa");

  assert(1, ++*pa, "++*pa");
  assert(0, --*pa, "--*pa");
  
  assert(1, ++arr[0], "++arr[0]");
  assert(0, --arr[0], "--arr[0]");
  
  //post inc/dec
  int b;
  b = 0;
  b++;
  assert(1, b, "b++;b;");
  assert(1, b++, "b++");
  assert(2, b, "b");
  assert(3, 1+b++, "1+b++");
  assert(3, b, "b");
  
  b--;
  assert(2, b, "b--;b;");
  assert(2, b--, "b--");
  assert(1, b, "b");
  assert(0, 1-b--, "1-b--");
  assert(0, b, "b");
  
  int* pb; pb=arr;
  pb++;
  assert(1, *pb, "pb++;*pb");
  assert(1, *pb++, "*pb++");
  assert(2, *pb, "*pb");
  pb--;
  assert(1, *pb, "pb--;*pb");
  assert(1, *pb--, "*pb--");
  assert(0, *pb, "*pb");
  
  assert(0, (*pb)++, "(*pb)++");
  assert(1, *pb, "*pb");
  assert(1, (*pb)--, "(*pb)--");
  assert(0, *pb, "*pb");

  assert(0, arr[0]++, "arr[0]++");
  assert(1, arr[0]--, "arr[0]--");
  assert(0, arr[0], "arr[0]");
  
  return 0;
}
