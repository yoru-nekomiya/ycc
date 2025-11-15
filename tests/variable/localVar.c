
int main(){
  int a;
  a = 2+3;
  assert(5, a, "a=2+3");

  int b;
  b = a;
  assert(5, b, "b=a");

  int c; int d;
  d = c = a;
  assert(5, c, "d=c=a");
  assert(5, d, "d=c=a");
  
  int x = 42;
  int y = -42;
  int z = x + y;
  assert(42, x, "int x = 42");
  assert(-42, y, "int y = -42");
  assert(0, z, "int z = x + y");
  
  int arr0[3] = {1,2,3};
  assert(1, arr0[0], "arr0[0]");
  assert(2, arr0[1], "arr0[1]");
  assert(3, arr0[2], "arr0[2]");
  
  int arr1[5] = {4,5,6};
  assert(4, arr1[0], "arr1[0]");
  assert(5, arr1[1], "arr1[1]");
  assert(6, arr1[2], "arr1[2]");
  assert(0, arr1[3], "arr1[3]");
  assert(0, arr1[4], "arr1[4]");
  
  int arr2[2] = {7,8,9};
  assert(7, arr2[0], "arr2[0]");
  assert(8, arr2[1], "arr2[1]");
			 
  char arr3[] = "Hello";
  assert('H', arr3[0], "arr3[0]");
  assert('e', arr3[1], "arr3[1]");
  assert('l', arr3[2], "arr3[2]");
  assert('l', arr3[3], "arr3[3]");
  assert('o', arr3[4], "arr3[4]");
  assert('\0', arr3[5], "arr3[5]");
  
  int arr4[2][2] = {{1,2},{3,4}};
  assert(1, arr4[0][0], "arr4[0][0]");
  assert(2, arr4[0][1], "arr4[0][1]");
  assert(3, arr4[1][0], "arr4[1][0]");
  assert(4, arr4[1][1], "arr4[1][1]");

  int arr5[][2] = {{5,6},{7,8}};
  assert(5, arr5[0][0], "arr5[0][0]");
  assert(6, arr5[0][1], "arr5[0][1]");
  assert(7, arr5[1][0], "arr5[1][0]");
  assert(8, arr5[1][1], "arr5[1][1]");

  int arr6[2][3] = {{1,2}, {3}};
  assert(1, arr6[0][0], "arr6[0][0]");
  assert(2, arr6[0][1], "arr6[0][1]");
  assert(0, arr6[0][2], "arr6[0][2]");
  assert(3, arr6[1][0], "arr6[1][0]");
  assert(0, arr6[1][1], "arr6[1][1]");
  assert(0, arr6[1][2], "arr6[1][2]");
  
  int arr7[2][2][2] = {
    {{1,2}, {3,4}},
    {{5,6}, {7,8}}
  };
  assert(1, arr7[0][0][0], "arr7[0][0][0]");
  assert(2, arr7[0][0][1], "arr7[0][0][1]");
  assert(3, arr7[0][1][0], "arr7[0][1][0]");
  assert(4, arr7[0][1][1], "arr7[0][1][1]");
  assert(5, arr7[1][0][0], "arr7[1][0][0]");
  assert(6, arr7[1][0][1], "arr7[1][0][1]");
  assert(7, arr7[1][1][0], "arr7[1][1][0]");
  assert(8, arr7[1][1][1], "arr7[1][1][1]");
  
  int arr8[2][3] = { {1,2,3} };
  assert(1, arr8[0][0], "arr8[0][0]");
  assert(2, arr8[0][1], "arr8[0][1]");
  assert(3, arr8[0][2], "arr8[0][2]");
  assert(0, arr8[1][0], "arr8[1][0]");
  assert(0, arr8[1][1], "arr8[1][1]");
  assert(0, arr8[1][2], "arr8[1][2]");
  
  char arr9[4] = "Hi";
  assert('H', arr9[0], "arr9[0]");
  assert('i', arr9[1], "arr9[1]");
  assert('\0', arr9[2], "arr9[2]");
  assert(0, arr9[3], "arr9[3]");

  char arr10[] = {'H','e','l','l','o'};
  assert('H', arr10[0], "arr10[0]");
  assert('e', arr10[1], "arr10[1]");
  assert('l', arr10[2], "arr10[2]");
  assert('l', arr10[3], "arr10[3]");
  assert('o', arr10[4], "arr10[4]");

  int i = 1;
  int arr11[2] = {i+1, i+2};
  assert(2, arr11[0], "arr11[0]");
  assert(3, arr11[1], "arr11[1]");

  int arr12[2][2] = {1,2,3,4};
  assert(1, arr12[0][0], "arr12[0][0]");
  assert(2, arr12[0][1], "arr12[0][1]");
  assert(3, arr12[1][0], "arr12[1][0]");
  assert(4, arr12[1][1], "arr12[1][1]");

  int arr13[2][3] = { {1}, 2, 3 };
  assert(1, arr13[0][0], "arr13[0][0]");
  assert(0, arr13[0][1], "arr13[0][1]");
  assert(0, arr13[0][2], "arr13[0][2]");
  assert(2, arr13[1][0], "arr13[1][0]");
  assert(3, arr13[1][1], "arr13[1][1]");
  assert(0, arr13[1][2], "arr13[1][2]");
  
  return 0;
}
