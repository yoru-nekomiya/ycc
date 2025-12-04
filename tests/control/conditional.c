int main(){
   assert(1, 1 ? 1 : 2, "1 ? 1 : 2");
   assert(2, 0 ? 1 : 2, "0 ? 1 : 2");

   assert(10, (3+2) ? 10 : 20, "(3+2) ? 10 : 20");
   assert(20, (3-3) ? 10 : 20, "(3-3) ? 10 : 20");

   assert(1, 1 ? (0 ? 9 : 1) : 2, "ternary nested 1");
  assert(2, 0 ? 1 : (1 ? 2 : 3), "ternary nested 2");

  int a = 0;
  int b = 0;
  int x = (1 ? (a = 5) : (b = 10));
  assert(5, a, "ternary short-circuit true a=5");
  assert(0, b, "ternary short-circuit true b unchanged");
  assert(5, x, "ternary short-circuit true result");

  int c = 0;
  int d = 0;
  int y = (0 ? (c = 5) : (d = 10));
  assert(0, c, "ternary short-circuit false c unchanged");
  assert(10, d, "ternary short-circuit false d=10");
  assert(10, y, "ternary short-circuit false result");

  int p = 0;
  int q = 0;
  int r = ((p = 1) ? (q = 2) : 3);
  assert(1, p, "ternary side-effect in condition p=1");
  assert(2, q, "ternary side-effect in true branch q=2");
  assert(2, r, "ternary result with side-effect");

  return 0;
}
