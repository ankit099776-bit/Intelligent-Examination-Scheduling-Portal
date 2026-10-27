/**
 * @file heap.h
 * @brief Binary Max-Heap Priority Queue for Intelligent Examination Scheduling System.
 * @course Data Structures and Algorithms - II (CCSE0301) - Module 1 (Trees)
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#ifndef HEAP_H
#define HEAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief Element stored inside the Max-Heap.
 */
typedef struct HeapNode {
    int vertex_id;                  /**< Course / vertex identifier */
    int primary_key;                /**< Primary priority: Saturation Degree or Vertex Degree */
    int secondary_key;              /**< Tie-breaking priority: Degree in uncolored subgraph */
} HeapNode;

/**
 * @brief Binary Max-Heap Priority Queue structure with reverse lookup mapping.
 * Implements Complete Binary Tree mapped to contiguous 0-indexed array.
 * Formulas:
 *   LeftChild(i)  = 2*i + 1
 *   RightChild(i) = 2*i + 2
 *   Parent(i)     = (i - 1) / 2
 */
typedef struct MaxHeap {
    HeapNode* array;                /**< Dynamic array of heap nodes */
    int* pos_map;                   /**< pos_map[v] gives index of vertex v in heap (for O(log V) updates) */
    int capacity;                   /**< Total allocated capacity */
    int size;                       /**< Current number of elements */
} MaxHeap;

/* Function Declarations */
MaxHeap* create_max_heap(int capacity);
void free_max_heap(MaxHeap* heap);
bool is_heap_empty(const MaxHeap* heap);
void swap_heap_nodes(MaxHeap* heap, int i, int j);
void max_heapify(MaxHeap* heap, int idx);
void insert_max_heap(MaxHeap* heap, int vertex_id, int primary_key, int secondary_key);
HeapNode extract_max(MaxHeap* heap);
void update_heap_priority(MaxHeap* heap, int vertex_id, int new_primary_key, int new_secondary_key);

/* Heap Sort Implementation for Welsh-Powell static degree pre-sorting */
void heap_sort_vertices(int* vertex_ids, int* degrees, int n);

#endif /* HEAP_H */
