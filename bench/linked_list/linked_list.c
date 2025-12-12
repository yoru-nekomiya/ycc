//#include <stdio.h>
//#include <time.h>
//#include <stdlib.h>
int printf();
void* malloc(long);

struct Node {
  int data;
  struct Node* next;
};

int LIST_SIZE = /*50000*/5000;
int INSERT_COUNT = /*100000*/50000;

long current_rand = 12345;
int simple_rand() {
  // LCG constants (a = 1103515245, c = 12345, m = 2^31)
  // Calculate the next state.
  current_rand = current_rand * 1103515245 + 12345;
  // Mask the result to the range of a positive 31-bit signed integer (2147483647)
  return current_rand & 2147483647;
}

// Inserts a new node at a pseudo-random position in the list.
void insert_random(struct Node* head, int value) {
  struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
  if (new_node == /*NULL*/0) return; 
  
  new_node->data = value;
  new_node->next = /*NULL*/0;

  // Determine a random target position (simple random access)
  int target_pos = simple_rand() % LIST_SIZE; 
  struct Node* current = head;

  // Traverse the list (causes pointer dereferences and cache misses)
  for (int i = 0; i < target_pos && current->next != /*NULL*/0; i++) {
    current = current->next;
  }

  // Perform insertion
  new_node->next = current->next;
  current->next = new_node;
}

// Calculates the sum of all elements in the list for a checksum.
long calculate_sum(struct Node* head) {
  long sum = 0;
  struct Node* current = head;
  while (current != /*NULL*/0) {
    sum += current->data;
    current = current->next;
  }
  return sum;
}

int main() {
  // Initialization of pseudo-random seed
  int rand_seed = 12345;

  // List construction (initial nodes)
  struct Node* head = (struct Node*)malloc(sizeof(struct Node));
  if (head == /*NULL*/0) return 1;
  head->data = 1;
  head->next = /*NULL*/0;
  
  struct Node* current = head;
  for (int i = 2; i <= LIST_SIZE; i++) {
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    if (new_node == /*NULL*/0) return 1; 
    new_node->data = i;
    new_node->next = /*NULL*/0;
    current->next = new_node;
    current = new_node;
  }
    
  for (int i = 0; i < INSERT_COUNT; i++) {
    //srand(rand_seed + i); // Use a new seed for simple variation
    insert_random(head, i + LIST_SIZE + 1);
  }
  
  long checksum = calculate_sum(head);
  
  // Memory deallocation is omitted for brevity
  
  printf("Linked List Ops, ");
  printf("Checksum (Total Sum): %ld\n", checksum);
  
  return 0;
}
