
int main(){
  char* s; s = "Hello";
  assert('H', s[0], "\'H\'");
  assert('e', s[1], "\'e\'");
  assert('l', s[2], "\'l\'");
  assert('l', s[3], "\'l\'");
  assert('o', s[4], "\'o\'");
  assert('\0', s[5], "\'\\0\'");

  s = "Hello,\tWorld!\n";
  printf("%s", s);
  
  s = "Hello,\'\"YCC\"\' World!\n";
  printf("%s", s);
  return 0;
}
