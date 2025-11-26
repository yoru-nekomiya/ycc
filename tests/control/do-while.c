
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

  i = 0; int j = 0;
  do {
    i++;
    break;
    j++;
  } while (i < 5);
  assert(1, i, "i, break in do-while loop");
  assert(0, j, "j, break in do-while loop");

  i = 0; j = 0;
  do {
    do {
      i++;
      break;      
    } while(1);
    j++;
  } while (i < 5);
  assert(5, i, "i, break in do-while loop 2");
  assert(5, j, "j, break in do-while loop 2");
  
  return 0;
}
