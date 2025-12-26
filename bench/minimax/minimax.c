//#include <stdio.h>
int printf();

#define MAX_DEPTH 25
#define NODE_BRANCH 3
#define EVAL_SEED 1103515245 // A large prime number used for pseudo-randomness

long nodes_visited = 0;

// Pseudo-evaluation function to simulate game state
int evaluate(int depth, int node_idx) {
  return (node_idx % EVAL_SEED + depth) % 100;
}

int minimax(int depth, int node_idx, int is_max, int alpha, int beta) {
  nodes_visited++;
  if (depth == MAX_DEPTH) return evaluate(depth, node_idx);
  
  if (is_max) {
    int best = -10000;
    for (int i = 0; i < NODE_BRANCH; i++) {
      int val = minimax(depth + 1, node_idx * NODE_BRANCH + i, 0, alpha, beta);
      if (val > best) best = val;
      if (val > alpha) alpha = val;
      if (beta <= alpha) break; // Alpha-beta pruning
    }
    return best;
  } else {
    int best = 10000;
    for (int i = 0; i < NODE_BRANCH; i++) {
      int val = minimax(depth + 1, node_idx * NODE_BRANCH + i, 1, alpha, beta);
      if (val < best) best = val;
      if (val < beta) beta = val;
      if (beta <= alpha) break; // Alpha-beta pruning
    }
    return best;
  }
}

int main() {
  int result = minimax(0, 0, 1, -10000, 10000);
  printf("Benchmark: Minimax Alpha-Beta, ");
  printf("Checksum (Result + Nodes): %ld\n", (long)result + nodes_visited);
  return 0;
}
