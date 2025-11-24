
int main(){
  int i = 0;
  do {
    i++;
  } while (0);
  assert(1, i, "do-while basic 1");

  i = 0;
  do {
    i++;
  } while (i < 5);
  assert(5, i, "do-while loop");
  
  return 0;
}
