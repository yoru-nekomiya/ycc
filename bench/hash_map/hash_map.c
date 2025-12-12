//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>

int printf();
void* malloc(long);
void free(void*);

// Hash Map Configuration
int HASH_TABLE_SIZE = 1024; 
int OPERATIONS = 500000;

// Hash Node Structure (Linked List element)
struct Node {
  int key;
  int value;
  struct Node* next;
};

// Hash Table (Array of Pointers to Nodes)
struct Node* table[/*HASH_TABLE_SIZE*/1024];

long current_rand = 42;

int simple_rand() {
  // LCG constants (a = 1103515245, c = 12345, m = 2^31)
  // Calculate the next state.
  current_rand = current_rand * 1103515245 + 12345;
  // Mask the result to the range of a positive 31-bit signed integer (2147483647)
  return current_rand & 2147483647;
}

// Simple Hash Function (Modulo operator with table size)
int hash(int key) {
  // A simple modulo hash, assuming HASH_TABLE_SIZE is a power of two
  return key & (HASH_TABLE_SIZE - 1);
}

// Initialize the hash table
void init_hash_table() {
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    table[i] = /*NULL*/0;
  }
}

// Insert a key-value pair
void insert(int key, int value) {
  int index = hash(key);
  struct Node* current = table[index];
  
  // Check for existing key (Update if found)
  while (current != /*NULL*/0) {
    if (current->key == key) {
      current->value = value;
      return;
    }
    current = current->next;
  }
  
  // Key not found, create new node and insert at the head of the linked list
  // This tests malloc overhead and pointer manipulation
  struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
  if (new_node == /*NULL*/0) return; 
  
  new_node->key = key;
  new_node->value = value;
  new_node->next = table[index];
  table[index] = new_node;
}

// Search for a key (returns value or -1 if not found)
int search(int key) {
  int index = hash(key);
  struct Node* current = table[index];
  
  // Traverse the linked list (tests pointer dereferencing and cache misses)
  while (current != /*NULL*/0) {
    if (current->key == key) {
      return current->value;
    }
    current = current->next;
  }
  return -1; // Not found
}

// Delete a key-value pair
void delete_key(int key) {
  int index = hash(key);
  struct Node* current = table[index];
  struct Node* prev = /*NULL*/0;
  
  while (current != /*NULL*/0) {
    if (current->key == key) {
      // Unlink the node
      if (prev == /*NULL*/0) {
	// Deleting the head node
	table[index] = current->next;
      } else {
	// Deleting a non-head node
	prev->next = current->next;
      }
      // Free the memory 
      free(current);
      return;
    }
    prev = current;
    current = current->next;
  }
}

// Calculate the checksum (Sum of all keys + values)
long calculate_checksum() {
  long sum = 0;
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    struct Node* current = table[i];
    while (current != /*NULL*/0) {
      sum += (long)current->key + (long)current->value;
      current = current->next;
    }
  }
  return sum;
}

int main() {  
  init_hash_table();
  
  // --- Phase 1: Initial Insertion (to build up data structure) ---
  for (int i = 0; i < HASH_TABLE_SIZE * 2; i++) {
    insert(i * 10, i * 2);
  }
  
  // --- Phase 2: Mixed Operations Benchmark ---
  for (int i = 0; i < OPERATIONS; i++) {
    // Generate a pseudo-random key within a large range
    int key = simple_rand(); 
    
    // Operation mix (e.g., 50% Search, 40% Insert, 10% Delete)
    if (key % 10 < 5) { // 50% Search
      search(key);
    } else if (key % 10 < 9) { // 40% Insert/Update
      insert(key, i);
    } else { // 10% Delete
      delete_key(key);
    }
  }
  
  long checksum = calculate_checksum();
  
  // Memory cleanup omitted for brevity
  
  printf("Hash Map Operations (Chaining), ");
  printf("Checksum (Keys + Values Sum): %ld\n", checksum); 
  
  return 0;
}
