//#include <stdio.h>
int printf();

// --- Define Configuration ---
#define MAX_NODES 2048
#define DENSITY_FACTOR 8 
#define MAX_EDGES 16384   //(MAX_NODES * DENSITY_FACTOR) 
#define INF 1000000
#define RUNS 10 

// Graph data structures (Adjacency List)
int head_edge[MAX_NODES];
int to[MAX_EDGES];
int weight[MAX_EDGES];
int next_edge[MAX_EDGES];
int dist[MAX_NODES];
long visited_checksum[MAX_NODES]; 
int edge_count = 0;

// Function to add a directed edge to the graph
void add_edge(int u, int v, int w) {
  if (edge_count < MAX_EDGES) {
    to[edge_count] = v;
    weight[edge_count] = w;
    next_edge[edge_count] = head_edge[u];
    head_edge[u] = edge_count++;
  }
}

// Graph initialization: Creates a dense, quasi-random graph
void init_graph() {
  edge_count = 0;
  for (int i = 0; i < MAX_NODES; i++) head_edge[i] = -1;
  
  // Connect edges based on DENSITY_FACTOR
  for (int i = 0; i < MAX_NODES; i++) {
    for (int j = 0; j < DENSITY_FACTOR; j++) {
      // Pseudo-random node selection
      int v = (i * 13 + j * 7 + (i / 10)) % MAX_NODES;
      int w = (i * j % 100) + 1; // Complex weight calculation
      if (v != i) {
	add_edge(i, v, w);
      }
    }
  }
}

// Core Dijkstra's Algorithm with inserted CPU load
void dijkstra(int start) {
  int i;
  int u;
  // Initialize distances and checksums
  for (i = 0; i < MAX_NODES; i++) {
    dist[i] = INF;
    visited_checksum[i] = 0; 
  }
  dist[start] = 0;
  
  int visited[MAX_NODES];
  for (i = 0; i < MAX_NODES; i++) visited[i] = 0;
  
  // Main loop runs MAX_NODES times
  for (i = 0; i < MAX_NODES; i++) {
    int min_dist = INF;
    u = -1;
    int j;
    
    // O(N) Priority Queue linear search (Main CPU load point)
    for (j = 0; j < MAX_NODES; j++) {
      // Heavy, non-vectorizable integer calculation inserted for CPU load
      long temp_calc = (long)j * 1103515245;
      temp_calc = temp_calc ^ (long)visited[j];
      // Write result back to memory, creating data dependence
      visited_checksum[j] += temp_calc; 
      
      if (!visited[j] && dist[j] < min_dist) {
	min_dist = dist[j];
	u = j;
      }
    }
    
    if (u == -1) break;
    visited[u] = 1;
    
    // Edge relaxation (High memory access irregularity)
    int e;
    for (e = head_edge[u]; e != -1; e = next_edge[e]) {
      int v = to[e];
      int w = weight[e];
      if (dist[u] != INF && dist[u] + w < dist[v]) {
	dist[v] = dist[u] + w;
      }
    }
  }
}

int main() {
  int r;
  long total_checksum = 0;
  
  // Execute multiple runs to increase total benchmark time
  for (r = 0; r < RUNS; r++) {
    init_graph();
    // Start from a different node each run
    dijkstra(r % MAX_NODES); 
    
    // Accumulate checksum from distances and internal calculations
    for (int i = 0; i < MAX_NODES; i++) {
      total_checksum += (long)dist[i];
      total_checksum += visited_checksum[i];
    }
  }
    
  printf("Benchmark: Dijkstra Shortest Path (Heavy Load), ");
  printf("Nodes: %d, Runs: %d, Edges: %d, ", MAX_NODES, RUNS, edge_count);
  printf("Checksum (Total Dist + Calc Sum): %ld\n", total_checksum);
  return 0;
}
