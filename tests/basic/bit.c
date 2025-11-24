
int main(){
  // ===== BIT OR =====
  assert(1, 0 | 1, "0|1");
  assert(0, 0 | 0, "0|0");
  assert(5, 1 | 4, "1|4");
  assert(7, 3 | 5, "3|5");
  assert(-1, -1 | 0, "-1|0");
  assert(-1, -1 | 5, "-1|5");

  // ===== BIT AND =====
  assert(0, 0 & 1, "0&1");
  assert(0, 0 & 0, "0&0");
  assert(0, 1 & 4, "1&4");
  assert(1, 3 & 5, "3&5");
  assert(0, -1 & 0, "-1&0");
  assert(5, -1 & 5, "-1&5");

  // ===== BIT XOR =====
  assert(1, 0 ^ 1, "0^1");
  assert(0, 0 ^ 0, "0^0");
  assert(5, 1 ^ 4, "1^4");
  assert(6, 3 ^ 5, "3^5");
  assert(-1, -1 ^ 0, "-1^0"); 
  assert(-6, -1 ^ 5, "-1^5"); 
  
// ===== BIT NOT =====
  assert(-1, ~0, "~0");
  assert(-2, ~1, "~1");
  assert(-6, ~5, "~5");
  assert(~-1, ~-1, "~-1"); // = 0
  
  // ===== Combination =====
  assert(1, ~(~1), "~(~1)");
  assert(0, (3 & 1) ^ 1, "(3&1)^1");
  assert(6, (3 | 1) ^ 5, "(3|1)^5");
  
  return 0;
}
