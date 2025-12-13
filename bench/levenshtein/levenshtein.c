//#include <stdio.h>
//#include <time.h>
//#include <stdlib.h>

int printf();

// Constant definitions
/*
int STR_LEN = 256;
int ITERATIONS = 1500;
*/
#define STR_LEN 256
#define ITERATIONS 1500

// This is the size of the DP table needed for strings up to 256 characters long.
int dp[257][257]; 
char s1[257];
char s2[257];

// Simple min function
int min(int a, int b, int c) {
  int m = a;
  if (b < m) m = b;
  if (c < m) m = c;
  return m;
}

// Function to initialize strings with a pattern to ensure non-zero distance.
// s1: "abcde..."
// s2: "abcde..." with slight variations
void init_strings(int len) {
  for (int i = 0; i < len; i++) {
    // Repeated sequence of 'a' through 'z'
    s1[i] = (char)('a' + (i % 26));
    
    // s2 is generated to be similar but different
    if (i % 7 == 0) {
      // Change case every 7 characters to ensure a non-zero edit distance
      s2[i] = (char)('A' + (i % 26)); 
    } else {
      s2[i] = s1[i];
    }
  }
  s1[len] = 0; // Null-terminate
  s2[len] = 0;
}

// Calculates the Levenshtein distance using Dynamic Programming.
int levenshtein_distance(int len1, int len2) {
  int i; int j;
  
  // Initialization (first row/column)
  for (i = 0; i <= len1; i++) {
    dp[i][0] = i;
  }
  for (j = 0; j <= len2; j++) {
    dp[0][j] = j;
  }
  
  // Fill the DP table. This is the main computational loop.
  // Tests data dependency and conditional branching performance.
  for (i = 1; i <= len1; i++) {
    for (j = 1; j <= len2; j++) {
      // Calculate cost (0 if match, 1 if mismatch)
      int cost;
      if (s1[i - 1] == s2[j - 1]) {
	cost = 0;
      } else {
	cost = 1;
      }
      
      // Choose the minimum of: Deletion, Insertion, or Substitution/Match
      dp[i][j] = min(dp[i - 1][j] + 1, 
		     dp[i][j - 1] + 1, 
		     dp[i - 1][j - 1] + cost);
    }
  }
  
  return dp[len1][len2];
}

int main() {  
  int i;
  // Use long long for checksum accumulation to prevent overflow
  long total_distance = 0;
  
  // Data generation
  init_strings(STR_LEN);  
  
  // Benchmark execution
  for (i = 0; i < ITERATIONS; i++) {
    // Accumulate the distance to prevent dead-code elimination by aggressive optimizers
    total_distance += (long)levenshtein_distance(STR_LEN, STR_LEN);
  }    
  
  printf("Levenshtein Distance (Len=%d, Iter=%d), ", STR_LEN, ITERATIONS);
  printf("Total Distance Checksum: %ld\n", total_distance); 
  
  return 0;
}
