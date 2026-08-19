#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_VERTICES 26
#define INF INT_MAX

// Structure for Min-Heap Node
typedef struct {
    int vertex;
    int dist;
} HeapNode;

// Structure for Min-Heap
typedef struct {
    HeapNode array[MAX_VERTICES];
    int pos[MAX_VERTICES];
    int size;
} MinHeap;

// --- HEAP DATA STRUCTURE FUNCTIONS ---

MinHeap* createMinHeap() {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->size = 0;
    return heap;
}

void swapHeapNodes(HeapNode* a, HeapNode* b, MinHeap* heap) {
    heap->pos[a->vertex] = (int)(b - heap->array);
    heap->pos[b->vertex] = (int)(a - heap->array);
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void minHeapify(MinHeap* heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size && heap->array[left].dist < heap->array[smallest].dist)
        smallest = left;

    if (right < heap->size && heap->array[right].dist < heap->array[smallest].dist)
        smallest = right;

    if (smallest != idx) {
        swapHeapNodes(&heap->array[smallest], &heap->array[idx], heap);
        minHeapify(heap, smallest);
    }
}

int isEmpty(MinHeap* heap) {
    return heap->size == 0;
}

HeapNode extractMin(MinHeap* heap) {
    if (isEmpty(heap)) {
        HeapNode nullNode = {-1, INF};
        return nullNode;
    }

    HeapNode root = heap->array[0];
    HeapNode lastNode = heap->array[heap->size - 1];
    heap->array[0] = lastNode;

    heap->pos[root.vertex] = -1;
    heap->pos[lastNode.vertex] = 0;

    heap->size--;
    minHeapify(heap, 0);

    return root;
}

void decreaseKey(MinHeap* heap, int v, int dist) {
    int i = heap->pos[v];
    heap->array[i].dist = dist;

    while (i && heap->array[i].dist < heap->array[(i - 1) / 2].dist) {
        heap->pos[heap->array[i].vertex] = (i - 1) / 2;
        heap->pos[heap->array[(i - 1) / 2].vertex] = i;
        swapHeapNodes(&heap->array[i], &heap->array[(i - 1) / 2], heap);
        i = (i - 1) / 2;
    }
}

int isInMinHeap(MinHeap* heap, int v) {
    return (heap->pos[v] < heap->size && heap->pos[v] != -1);
}

// --- PATH AND ROUTING HELPER FUNCTIONS ---

void printPath(int parent[], int j) {
    if (parent[j] == -1) {
        printf("%c", j + 'A');
        return;
    }
    printPath(parent, parent[j]);
    printf(" -> %c", j + 'A');
}

int getNextHop(int parent[], int target, int source) {
    if (target == source) return source;
    if (parent[target] == source) return target;

    int curr = target;
    while (parent[curr] != -1 && parent[curr] != source) {
        curr = parent[curr];
    }
    return (parent[curr] == source) ? curr : -1;
}

int countOutgoingEdges(int adj[MAX_VERTICES][MAX_VERTICES], int V, int u) {
    int count = 0;
    int v;
    for ( v = 0; v < V; v++) {
        if (adj[u][v] != INF) count++;
    }
    return count;
}

// --- LOGGING AND OUTPUT FORMATTING FUNCTIONS ---

void printLogHeader(int src) {
    printf("\n=======================================================================\n");
    printf("               STEP-BY-STEP DISTANCE LOG TABLE (SOURCE: %c)\n", src + 'A');
    printf("=======================================================================\n");
    printf("%-7s | %-16s | %-20s | %-22s\n", "Step", "Extracted Node", "Path Addition", "New Shortest Distance");
    printf("-----------------------------------------------------------------------\n");
}

void printUnreachableLog(int *stepCount, int processed[], int V) {
    int v;
    for (v = 0; v < V; v++) {
        if (!processed[v]) {
            printf("%-7d | %-16c | %-20s | %-22s\n", (*stepCount)++, v + 'A', "-", "Unreachable / INF");
            processed[v] = 1;
        }
    }
}

void processNodeNeighbors(int adj[MAX_VERTICES][MAX_VERTICES], int V, int u,
                          int dist[], int parent[], MinHeap* heap, int stepCount) {
    int v;
    for (v = 0; v < V; v++) {
        if (adj[u][v] != INF) {
            char pathAdd[50];
            char distStr[50];

            snprintf(pathAdd, sizeof(pathAdd), "%d + %d = %d", dist[u], adj[u][v], dist[u] + adj[u][v]);

            if (dist[u] + adj[u][v] < dist[v]) {
                if (dist[v] == INF) {
                    snprintf(distStr, sizeof(distStr), "INF -> %d", dist[u] + adj[u][v]);
                } else {
                    snprintf(distStr, sizeof(distStr), "%d -> %d", dist[v], dist[u] + adj[u][v]);
                }
                dist[v] = dist[u] + adj[u][v];
                parent[v] = u;

                if (isInMinHeap(heap, v)) {
                    decreaseKey(heap, v, dist[v]);
                }
            } else {
                snprintf(distStr, sizeof(distStr), "%d >= %d (Ignored)", dist[u] + adj[u][v], dist[v]);
            }

            printf("%-7d | %-16c | %-20s | %-22s\n", stepCount, u + 'A', pathAdd, distStr);
        }
    }
}

void printRoutingTable(int src, int V, int dist[], int parent[]) {
    printf("\n---------------------------------------------------------------------------------------------------\n");
    printf("%-9s | %-13s | %-20s | %-14s | %-20s\n", "Source", "Destination", "Next Node to Visit", "Final Distance", "Full Path");
    printf("---------------------------------------------------------------------------------------------------\n");

    int dest;
    for (dest = 0; dest < V; dest++) {
        printf("%-9c | %-13c | ", src + 'A', dest + 'A');

        if (dest == src) {
            printf("%-20s | Dist: %-9d | %c\n", "Self (0)", 0, src + 'A');
        } else if (dist[dest] == INF) {
            printf("%-20s | Dist: %-9s | %s\n", "None", "INF", "No Path");
        } else {
            int nextHop = getNextHop(parent, dest, src);
            char nextHopStr[30];
            snprintf(nextHopStr, sizeof(nextHopStr), "%c (Next Hop)", nextHop + 'A');
            printf("%-20s | Dist: %-9d | ", nextHopStr, dist[dest]);
            printPath(parent, dest);
            printf("\n");
        }
    }
    printf("---------------------------------------------------------------------------------------------------\n\n");
}

// --- ALGORITHM EXECUTION FUNCTIONS ---

void initializeDijkstra(MinHeap* heap, int V, int src, int dist[], int parent[], int processed[]) {
    int i;
    for (i = 0; i < V; i++) {
        dist[i] = INF;
        parent[i] = -1;
        processed[i] = 0;
        heap->array[i].vertex = i;
        heap->array[i].dist = INF;
        heap->pos[i] = i;
    }
    dist[src] = 0;
    heap->array[src].dist = 0;
    heap->size = V;
    decreaseKey(heap, src, 0);
}

void runDijkstra(int adj[MAX_VERTICES][MAX_VERTICES], int V, int src) {
    int dist[MAX_VERTICES];
    int parent[MAX_VERTICES];
    int processed[MAX_VERTICES];

    MinHeap* heap = createMinHeap();
    initializeDijkstra(heap, V, src, dist, parent, processed);

    printLogHeader(src);

    int stepCount = 1;
    int count;
    for ( count = 0; count < V; count++) {
        HeapNode minNode = extractMin(heap);
        int u = minNode.vertex;

        if (u == -1 || minNode.dist == INF) {
            printUnreachableLog(&stepCount, processed, V);
            break;
        }

        processed[u] = 1;
        int outgoingCount = countOutgoingEdges(adj, V, u);

        if (outgoingCount == 0) {
            printf("%-7d | %-16c | %-20s | %-22s\n", stepCount++, u + 'A', "-", "No Outgoing Edges");
        } else {
            processNodeNeighbors(adj, V, u, dist, parent, heap, stepCount);
            stepCount++;
        }
    }

    free(heap);
    printRoutingTable(src, V, dist, parent);
}

// --- INPUT MANAGEMENT FUNCTIONS ---

void initializeGraph(int adj[MAX_VERTICES][MAX_VERTICES], int V) {
    int i,j;
    for ( i = 0; i < V; i++) {
        for ( j = 0; j < V; j++) {
            adj[i][j] = INF;
        }
    }
}

void readSingleEdge(int adj[MAX_VERTICES][MAX_VERTICES], int V, int edgeNum) {
    char uChar, vChar;
    int w;

    while (1) {
        if (scanf(" %c %c %d", &uChar, &vChar, &w) != 3) {
            printf("Invalid input format. Try again.\n");
            while (getchar() != '\n');
            continue;
        }

        int u = uChar - 'A';
        int v = vChar - 'A';

        if (u < 0 || u >= V || v < 0 || v >= V) {
            printf("Invalid vertices! Re-enter edge %d: ", edgeNum);
            continue;
        }

        if (w < 0) {
            printf("Negative weights not allowed! Re-enter edge %d: ", edgeNum);
            continue;
        }

        adj[u][v] = w;
        break;
    }
}

void readGraphInputs(int adj[MAX_VERTICES][MAX_VERTICES], int *V, int *E) {
    printf("Enter number of vertices: ");
    if (scanf("%d", V) != 1 || *V <= 0 || *V > MAX_VERTICES) {
        printf("Invalid vertex count.\n");
        exit(1);
    }

    initializeGraph(adj, *V);

    printf("Enter number of edges: ");
    if (scanf("%d", E) != 1 || *E < 0) {
        printf("Invalid edge count.\n");
        exit(1);
    }

    printf("Enter edges (Source Destination Weight, e.g., A B 5):\n");
    int i;
    for ( i = 0; i < *E; i++) {
        readSingleEdge(adj, *V, i + 1);
    }
}

void runAllRoutingTables(int adj[MAX_VERTICES][MAX_VERTICES], int V) {
    int i;
    for ( i = 0; i < V; i++) {
        runDijkstra(adj, V, i);
    }
}

// --- MAIN FUNCTION ---

int main() {
    int V, E;
    int adj[MAX_VERTICES][MAX_VERTICES];

    readGraphInputs(adj, &V, &E);
    runAllRoutingTables(adj, V);

    return 0;
}
