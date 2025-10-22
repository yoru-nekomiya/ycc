
int main(){
  int a[1+1];
  *a=1;
  *(a+1)=2;
  int* p;
  p=a;
  assert(3, *p + *(p+1));

  p=(a+1);
  assert(3, *p + *(p-1));

  int b[1+2]; *b=1; *(b+1)=2; *(b+2)=3;
  p=b;
  assert(6, *p + *(p+1) + *(p+2));

  int c[2];
  c[0]=2; c[1]=3;
  assert(5, c[0]+c[1]);

  int d[2][2];
  d[0][0] = 1; d[0][1] = 2; d[1][0] = 3; d[1][1] = 4;
  assert(10, d[0][0]+d[0][1]+d[1][0]+d[1][1]);
  return 0;
}
