#define N 1
#define M_a 100
#define  num   2

void assert(long expected, long actual, char* desc);
int printf();

int main(){
  assert(1, N, "#define N 1");
  assert(100, M_a, "#define M_a 100");
  assert(2, num, "#define num 2");
  
  return 0;
}
