//#include <stdio.h>
int printf();

// --- Define Configuration ---
#define LIST_SIZE 16384
#define ITERATIONS 50
#define INNER_LOOP_FACTOR 100

// Node structure
struct Node {
  int value;
  struct Node *next;
  // For complexity: A pointer to another, possibly random, node
  struct Node *random_link; 
};

struct Node nodes[LIST_SIZE];

// Global head pointer
struct Node *head = 0/*NULL*/;

// --- Initialization---
void init_list() {
  int i;
  struct Node *rand_targets[LIST_SIZE];
  
  // 1. Create linear list
  for (i = 0; i < LIST_SIZE; i++) {
    // Initial value is non-zero to provide calculation input
    nodes[i].value = i * 123 + 1; 
    nodes[i].next = (i == LIST_SIZE - 1) ? 0/*NULL*/ : &nodes[i + 1];
    nodes[i].random_link = 0/*NULL*/;
    rand_targets[i] = &nodes[i];
  }
  
  // 2. Set random links
  for (i = 0; i < LIST_SIZE; i++) {
    int rand_idx = (i * 123 + nodes[i].value) % LIST_SIZE;
    nodes[i].random_link = rand_targets[rand_idx];
  }
  
  head = &nodes[0];
}

// --- Core Linked Reversal and COMPLEX Update ---
// Reverses the list structure and performs a complex, CPU-heavy update
void complex_reverse_and_update() {
  struct Node *prev = 0/*NULL*/;
  struct Node *current = head;
  struct Node *next_node;
  
  while (current != 0/*NULL*/) {
    // 1. Standard List Reversal (Pointer Chasing)
    next_node = current->next;
    current->next = prev;
    
    // 2. Complex CPU-intensive calculation (New Load)
    if (current->random_link != 0/*NULL*/) {
      long input_val;
      long num_divisors = 0;
      int k;
      
      // Combine values from current and random link to ensure data dependency
      // Use XOR and bit shifts to introduce non-arithmetic integer ops
      input_val = (long)current->value;
      input_val = input_val ^ (long)current->random_link->value;
      input_val = (input_val % 9999) + 10000; // Keep input in a measurable range (10000-19998)
      
      // Inner loop for CPU load: Count divisors of input_val up to INNER_LOOP_FACTOR
      // This is non-vectorizable and highly branchy.
      for (k = 1; k < INNER_LOOP_FACTOR; k++) {
	// The modulus operation here is critical for stressing integer unit
	if (input_val % k == 0) {
	  num_divisors++;
	}
      }
      
      // Update current node value based on the calculation result
      // This result must be stored back to memory, ensuring the dependence is complete.
      current->value = (int)num_divisors + (current->value >> 1);
    }
    
    // 3. Update iteration pointers
    prev = current;
    current = next_node;
  }
  
  head = prev;
}

// ----------------------------------------------------
// Main Function and Checksum
// ----------------------------------------------------

int main() {
  long total_checksum = 0;
  struct Node *current_node;
  
  init_list();
  
  // Benchmark Execution Loop
  for (int it = 0; it < ITERATIONS; it++) {
    complex_reverse_and_update();
  }
  
  // Calculate Checksum (Sum of final values)
  current_node = head;
  while (current_node != 0/*NULL*/) {
    total_checksum += (long)current_node->value;
    current_node = current_node->next;
  }
  
  printf("Benchmark: Complex Linked Reversal (Pointers/Indirection), ");
  printf("List Size: %d, Iterations: %d, ", LIST_SIZE, ITERATIONS);
  printf("Checksum (Final Value Sum): %ld\n", total_checksum);
  
  return 0;
}
