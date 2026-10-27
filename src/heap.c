/**
 * @file heap.c
 * @brief Implementation of Binary Max-Heap Priority Queue.
 * @course Data Structures and Algorithms - II (CCSE0301) - Module 1 (Trees)
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#include "../include/heap.h"

MaxHeap* create_max_heap(int capacity) {
    if (capacity <= 0) return NULL;

    MaxHeap* heap = (MaxHeap*)malloc(sizeof(MaxHeap));
    if (!heap) return NULL;

    heap->capacity = capacity;
    heap->size = 0;
    heap->array = (HeapNode*)malloc(capacity * sizeof(HeapNode));
    heap->pos_map = (int*)malloc(capacity * sizeof(int));

    for (int i = 0; i < capacity; i++) {
        heap->pos_map[i] = -1;
    }

    return heap;
}

void free_max_heap(MaxHeap* heap) {
    if (!heap) return;
    if (heap->array) free(heap->array);
    if (heap->pos_map) free(heap->pos_map);
    free(heap);
}

bool is_heap_empty(const MaxHeap* heap) {
    return (!heap || heap->size == 0);
}

void swap_heap_nodes(MaxHeap* heap, int i, int j) {
    HeapNode temp = heap->array[i];
    heap->array[i] = heap->array[j];
    heap->array[j] = temp;

    // Update reverse position mapping
    heap->pos_map[heap->array[i].vertex_id] = i;
    heap->pos_map[heap->array[j].vertex_id] = j;
}

/**
 * @brief Returns true if node A has higher priority than node B.
 * Primary key: Saturation degree (or static degree).
 * Secondary key: Tie-breaker on degree.
 */
static inline bool is_higher_priority(const HeapNode* a, const HeapNode* b) {
    if (a->primary_key > b->primary_key) return true;
    if (a->primary_key == b->primary_key && a->secondary_key > b->secondary_key) return true;
    return false;
}

void max_heapify(MaxHeap* heap, int idx) {
    int largest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size && is_higher_priority(&heap->array[left], &heap->array[largest])) {
        largest = left;
    }

    if (right < heap->size && is_higher_priority(&heap->array[right], &heap->array[largest])) {
        largest = right;
    }

    if (largest != idx) {
        swap_heap_nodes(heap, idx, largest);
        max_heapify(heap, largest);
    }
}

void insert_max_heap(MaxHeap* heap, int vertex_id, int primary_key, int secondary_key) {
    if (!heap || heap->size >= heap->capacity) return;

    int i = heap->size++;
    heap->array[i].vertex_id = vertex_id;
    heap->array[i].primary_key = primary_key;
    heap->array[i].secondary_key = secondary_key;
    heap->pos_map[vertex_id] = i;

    // Bubble up
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (is_higher_priority(&heap->array[i], &heap->array[parent])) {
            swap_heap_nodes(heap, i, parent);
            i = parent;
        } else {
            break;
        }
    }
}

HeapNode extract_max(MaxHeap* heap) {
    HeapNode empty_node = {-1, -1, -1};
    if (is_heap_empty(heap)) return empty_node;

    HeapNode root = heap->array[0];
    heap->pos_map[root.vertex_id] = -1;

    heap->array[0] = heap->array[heap->size - 1];
    heap->pos_map[heap->array[0].vertex_id] = 0;
    heap->size--;

    if (heap->size > 0) {
        max_heapify(heap, 0);
    }

    return root;
}

void update_heap_priority(MaxHeap* heap, int vertex_id, int new_primary_key, int new_secondary_key) {
    if (!heap || vertex_id < 0 || vertex_id >= heap->capacity) return;

    int idx = heap->pos_map[vertex_id];
    if (idx < 0 || idx >= heap->size) return; // Node already extracted

    bool increased = false;
    HeapNode new_val = {vertex_id, new_primary_key, new_secondary_key};
    if (is_higher_priority(&new_val, &heap->array[idx])) {
        increased = true;
    }

    heap->array[idx].primary_key = new_primary_key;
    heap->array[idx].secondary_key = new_secondary_key;

    if (increased) {
        // Sift up
        while (idx > 0) {
            int parent = (idx - 1) / 2;
            if (is_higher_priority(&heap->array[idx], &heap->array[parent])) {
                swap_heap_nodes(heap, idx, parent);
                idx = parent;
            } else {
                break;
            }
        }
    } else {
        // Sift down
        max_heapify(heap, idx);
    }
}

/**
 * @brief Heap Sort implementation for ordering vertices descending by degree.
 * Used by the Welsh-Powell scheduling heuristic.
 * Time Complexity: O(n log n).
 */
void heap_sort_vertices(int* vertex_ids, int* degrees, int n) {
    if (!vertex_ids || !degrees || n <= 1) return;

    // Use a min-heap to sort descending, or extract-max from max-heap
    MaxHeap* heap = create_max_heap(n);
    for (int i = 0; i < n; i++) {
        insert_max_heap(heap, vertex_ids[i], degrees[vertex_ids[i]], 0);
    }

    for (int i = 0; i < n; i++) {
        HeapNode top = extract_max(heap);
        vertex_ids[i] = top.vertex_id;
    }

    free_max_heap(heap);
}
