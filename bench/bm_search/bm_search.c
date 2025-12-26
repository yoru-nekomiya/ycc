//#include <stdio.h>
int printf();

#define TEXT_SIZE 131072
#define PATTERN_SIZE 32
#define ALPHABET_SIZE 256
#define SEARCH_RUNS 200

char text[TEXT_SIZE];
char pattern[PATTERN_SIZE];
int bad_char_table[ALPHABET_SIZE];

void init_bm() {
  int i;
  // Create a pattern: "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345"
  for (i = 0; i < PATTERN_SIZE; i++) {
    if (i < 26) pattern[i] = (char)(65 + i); // A-Z
    else pattern[i] = (char)(48 + (i - 26)); // 0-5
  }

  for (i = 0; i < TEXT_SIZE; i++) {
    // Create a pattern that doesn't match too frequently
    text[i] = (char)((i % 43) + 48);
    if (i % 2000 == 0 && i + PATTERN_SIZE < TEXT_SIZE) {
      for (int k = 0; k < PATTERN_SIZE; k++) {
	text[i + k] = pattern[k];
      }
      i += PATTERN_SIZE - 1; // Skip forward
    }
  }
  
  for (i = 0; i < ALPHABET_SIZE; i++) {
    bad_char_table[i] = PATTERN_SIZE;
  }
  
  for (i = 0; i < PATTERN_SIZE - 1; i++) {
    // Cast to int and ensure it's positive before using as index
    int char_idx = (int)pattern[i];
    bad_char_table[char_idx & 255] = PATTERN_SIZE - 1 - i;
  }
}

// Heavy Boyer-Moore search kernel
int search_bm() {
  int count = 0;
  int s = 0; 
  int max_s = TEXT_SIZE - PATTERN_SIZE;
  
  while (s <= max_s) {
    int j = PATTERN_SIZE - 1;
    
    // Match from right to left
    while (j >= 0 && pattern[j] == text[s + j]) {
      j--;
    }
    
    if (j < 0) {
      // Full match found
      count++;
      s += 1; 
    } else {
      // Mismatch: Use the Bad Character Rule
      int mismatch_char = (int)text[s + j];
      int bc_skip = bad_char_table[mismatch_char & 255];
      
      int shift = bc_skip - (PATTERN_SIZE - 1 - j);
      s += (shift > 1) ? shift : 1;
    }
  }
  return count;
}

int main() {
  init_bm();
  int matches = 0;
  for (int i = 0; i < SEARCH_RUNS; i++)
    matches += search_bm();
  printf("Benchmark: Boyer-Moore Search, ");
  printf("Runs: %d, Text Size: %d, ", SEARCH_RUNS, TEXT_SIZE);
  printf("Checksum (Total Match Count): %d\n", matches);
  return 0;
}
