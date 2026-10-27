/**
 * @file graph.h
 * @brief Graph data structures and algorithms for Intelligent Examination Scheduling System.
 * @course Data Structures and Algorithms - II (CCSE0301)
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CODE_LEN 16
#define MAX_NAME_LEN 64
#define MAX_DEPT_LEN 32

/**
 * @brief Structure representing a course/exam subject (Vertex in Conflict Graph).
 */
typedef struct Course {
    int id;                         /**< 0-indexed vertex identifier */
    char code[MAX_CODE_LEN];        /**< Course code, e.g., "CS301" */
    char title[MAX_NAME_LEN];       /**< Course title, e.g., "Data Structures - II" */
    char department[MAX_DEPT_LEN];  /**< Department, e.g., "CSE-AI" */
    int enrolled_students;          /**< Number of registered students */
    int timeslot;                   /**< Assigned timeslot (-1 if unassigned) */
    int room_id;                    /**< Primary allocated room ID (-1 if unassigned) */
} Course;

/**
 * @brief Node in adjacency linked list.
 */
typedef struct AdjListNode {
    int dest;                       /**< Destination vertex index */
    int weight;                     /**< Number of shared student clashes */
    struct AdjListNode* next;       /**< Pointer to next neighbor */
} AdjListNode;

/**
 * @brief Head of adjacency list for a single vertex.
 */
typedef struct AdjList {
    AdjListNode* head;              /**< Linked list of incident edges */
    int degree;                     /**< Total degree deg(v) */
    int saturation_degree;          /**< DSATUR dynamic saturation degree */
} AdjList;

/**
 * @brief Complete Graph data structure supporting both Matrix and List.
 * Demonstrates Module 2 (Graphs & Graph Algorithms).
 */
typedef struct Graph {
    int num_vertices;               /**< Number of examination courses |V| */
    int num_edges;                  /**< Number of conflict edges |E| */
    Course* courses;                /**< Array of Course metadata [|V|] */
    AdjList* adj_list;              /**< Adjacency List array [|V|] */
    int** adj_matrix;               /**< Adjacency Matrix [|V|][|V|] for O(1) checks */
} Graph;

/* Function Declarations */
Graph* create_graph(int num_vertices);
void free_graph(Graph* graph);
void add_conflict_edge(Graph* graph, int src, int dest, int clash_count);
bool has_conflict(const Graph* graph, int u, int v);
int get_clash_count(const Graph* graph, int u, int v);
int find_connected_components(const Graph* graph, int* component_map);
void print_graph_summary(const Graph* graph);

/* CSV Ingestion Functions */
Graph* load_dataset_from_csv(const char* courses_file, const char* enrollments_file);

#endif /* GRAPH_H */
