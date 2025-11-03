
int main(){
  //pre inc/dec
  int a;
  a = 0;
  ++a;
  assert(1, a);
  assert(2, ++a);
  
  --a;
  assert(1, a);
  assert(0, --a);
  
  int arr[3];
  arr[0] = 0; arr[1] = 1; arr[2] = 2;
  int* pa; pa=arr;
  ++pa;
  assert(1, *pa);
  assert(2, *++pa);
  --pa;
  assert(1, *pa);
  assert(0, *--pa);

  assert(1, ++*pa);
  assert(0, --*pa);
  
  assert(1, ++arr[0]);
  assert(0, --arr[0]);
  
  //post inc/dec
  int b;
  b = 0;
  b++;
  assert(1, b);
  assert(1, b++);
  assert(2, b);

  b--;
  assert(1, b);
  assert(1, b--);
  assert(0, b);
  
  int* pb; pb=arr;
  pb++;
  assert(1, *pb);
  assert(1, *pb++);
  assert(2, *pb);
  pb--;
  assert(1, *pb);
  assert(1, *pb--);
  assert(0, *pb);
  
  assert(0, (*pb)++);
  assert(1, *pb);
  assert(1, (*pb)--);
  assert(0, *pb);

  assert(0, arr[0]++);
  assert(1, arr[0]--);
  assert(0, arr[0]);
  
  return 0;
}
