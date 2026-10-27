/**
 * @file coloring.h
 * @brief Graph Coloring Algorithms: Welsh-Powell, DSATUR, and Exact Backtracking.
 * @course Data Structures and Algorithms - II (CCSE0301) - Module 2 & 4
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#ifndef COLORING_H
#define COLORING_H

#include "graph.h"
#include "heap.h"
#include <time.h>

/**
 * @brief Result summary of graph coloring execution.
 */
typedef struct ColoringResult {
    char algorithm_name[32];        /**< Name of algorithm used */
    int chromatic_number;           /**< Number of distinct time slots used \chi(G) */
    double execution_time_ms;       /**< Time taken in milliseconds */
    int total_conflicts_prevented;  /**< Number of potential clashes eliminated */
    double avg_student_gap_slots;   /**< Quality metric: average slot gap between exams for students */
    bool is_valid;                  /**< Whether hard constraint (slot[u] != slot[v]) holds */
} ColoringResult;

/* Graph Coloring Algorithms */
ColoringResult solve_welsh_powell(Graph* graph);
ColoringResult solve_dsatur(Graph* graph);
ColoringResult solve_exact_backtracking(Graph* graph, int time_limit_sec);

/* Auxiliary lower bound computation */
int compute_max_clique_lower_bound(const Graph* graph);

#endif /* COLORING_H */
