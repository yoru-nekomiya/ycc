
int main(){
  char* s; s = "Hello";
  assert('H', s[0]);
  assert('e', s[1]);
  assert('l', s[2]);
  assert('l', s[3]);
  assert('o', s[4]);
  assert('\0', s[5]);

  s = "Hello,\tWorld!\n";
  printf("%s", s);
  
  s = "Hello,\'\"YCC\"\' World!\n";
  printf("%s", s);
  return 0;
}
