
int y = 10;

int main(){
  int x = 0;
  {
    int x = 1;
    assert(1, x, "x, inner scope shadowing");
  }
  assert(0, x, "x, outer scope looking");
  
  {
    assert(0, x, "x, inner scope looking");
  }

  {
    int y = 20;
    {
      int y = 30;
      assert(30, y, "y, inner-most scope shadowing");
    }
    assert(20, y, "y, inner scope shadowing");
  }
  assert(10, y, "y, outer scope looking");
  /*
  {
    //redeclaration of variable
    int x = 1;
    int x = 2;
  }
  */
  for(int x = 10; ; x--){
    assert(10, x, "x, for-loop shadowing");
    break;
  }
  assert(0, x, "x, after for-loop shadowing");
  
  return 0;
}
