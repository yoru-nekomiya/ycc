
int func(){
  return 1 || 0;
}

int main(){
  
  assert(0, 0, "0");
  assert(42, 42, "42");
  assert(-42, -42, "-42");
  
  assert(2, 1+1, "1+1");
  assert(9, 10-1, "10-1");
  assert(4, +5+(-1), "+5+(-1)");
  assert(15, 3*5, "3*5");
  assert(5, 10/2, "10/2");
  assert(3, (1+2) * (8/4) / (3-1), "(1+2) * (8/4) / (3-1)");

  assert(1, 1||2, "1||2");
  assert(1, 1||0, "1||0");
  assert(0, 0||0, "0||0");
  assert(1, -3||0, "-3||0");
  int x = 0;
  1 || (x = 1);
  assert(0, x, "x (LOGOR short-circuit test)");
  assert(1, 1 || (0 && (x = 1)), "deep short-circuit OR");

  
  assert(1, 1&&2, "1&&2");
  assert(0, 1&&0, "1&&0");
  assert(0, 0&&0, "0&&0");
  assert(1, -3&&5, "-3&&5");
  0 && (x = 1);
  assert(0, x, "x (LOGAND short-circuit test)");

  assert(1, (1||0)&&(0||1), "(1||0)&&(0||1)");
  assert(0, (1&&0)||(0&&1), "(1&&0)||(0&&1)");
  assert(1, 1 || 0 && 0, "1 || 0 && 0");
  assert(0, (1 || 0) && 0, "(1 || 0) && 0");

  assert(1, func(), "return with ||");

  assert(4, 1<<2, "1<<2");
  assert(1, 4>>2, "4>>2");
  assert(5, 5<<0, "5<<0");
  assert(5, 5>>0, "5>>0");
  assert(-2, -8>>2, "-8>>2");
  assert(-1, -1>>1, "-1>>1");

  assert(8, 1 << 2 << 1, "1<<2<<1");
  assert(1, 16 >> 2 >> 2, "16>>2>>2");
  assert(4, (8 >> 2) << 1, "(8>>2)<<1");
  assert(-4, (-16 >> 2), "-16>>2");

  int k = 3;
  assert(8, 1 << k, "1<<k");
  assert(2, 8 >> (k=2), "8>>(k=2)");

  int a = 3; a += 5; assert(8, a, "a+=5");
  int b = 10; b -= 4; assert(6, b, "b-=4");
  int c = 6; c *= 7; assert(42, c, "c*=7");
  int d = 20; d /= 5; assert(4, d, "d/=5");

  int e = -3; e += 10; assert(7, e, "e+=10");
  int f = 5; f -= 12; assert(-7, f, "f-=12");
  int g = -4; g *= 3; assert(-12, g, "g*=3");
  int h = -20; h /= 4; assert(-5, h, "h/=4");

  b = (a += 4);
  assert(12, a, "a+=4");
  assert(12, b, "b=(a+=4)");

  a += 3 * 5;
  assert(27, a, "a+=3*5");

  int arr[3] = {1,2,3}; int* p = arr; int i = 0;
  p += 1; assert(2, *p, "p+=1");
  p -= 1; assert(1, *p, "p-=1");
  *(p+=1) += 10; assert(12, arr[1], "*(p+=1)+=10");
  arr[1] = 2;
  
  arr[i++] += 3;
  assert(1, i, "i, post-increment once");
  assert(4, arr[0], "arr[i++]+=3");
  i = 0; arr[++i] += 3;
  assert(1, i, "i, pre-increment once");
  assert(5, arr[1], "arr[++i]+=3");
  arr[1] += 5; assert(10, arr[1], "arr[1]+=5");
  arr[2] *= -2; assert(-6, arr[2], "arr[2]*=-2");

  int j = 10; int* pj = &j;
  *pj += 3; assert(13, j, "*pj+=3");
  *pj -= 10; assert(3, j, "*pj-=10");
  
  return 0;
}
