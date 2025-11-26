
int main(){
  int x = 0;
  switch (2) {
    case 1: x = 10; break;
    case 2: x = 20; break;
    case 3: x = 30; break;
  }
  assert(20, x, "switch basic match");

  x = 0;
  switch (5) {
  case 1: x = 1; break;
  case 2: x = 2; break;
  default: x = 99;
  }
  assert(99, x, "switch default");

  x = 0;
  switch (5) {
  default: x = 99; break;
  case 1: x = 1; break;
  case 2: x = 2; break;
  }
  assert(99, x, "switch default in middle");

  x = 0;
  switch (1) {
  case 1: x += 1; 
  case 2: x += 2; break;
  case 3: x += 3;
  }
  assert(3, x, "switch break stops fall-through");

  x = 1;
  switch (x) {
  case 1:  
  case 2: x = 20; break;
  case 3: x = 30; break;
  }
  assert(20, x, "switch break stops fall-through 2");

  x = 0;
  switch (1) {
  case 1:
    switch (2) {
    case 2: x = 22; break;
    }
    break;
  default:
    x = 99;
  }
  assert(22, x, "nested switch");

  x = 0;
  for (int i = 0; i < 5; i++) {
    switch (i) {
    case 2:
      continue; 
    }
    x++;
  }
  assert(4, x, "switch inside for with continue");
 
  return 0;
}
