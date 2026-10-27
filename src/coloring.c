/**
 * @file coloring.c
 * @brief Implementation of Welsh-Powell, DSATUR, and Exact Backtracking Solvers.
 * @course Data Structures and Algorithms - II (CCSE0301) - Module 2 & 4
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#include "../include/coloring.h"

/* Reset all course timeslot assignments */
static void reset_coloring(Graph* graph) {
    for (int i = 0; i < graph->num_vertices; i++) {
        graph->courses[i].timeslot = -1;
        graph->adj_list[i].saturation_degree = 0;
    }
}

/* Check if assigning color to vertex u violates hard conflict */
static bool is_color_safe(const Graph* graph, int u, int color) {
    AdjListNode* curr = graph->adj_list[u].head;
    while (curr) {
        int v = curr->dest;
        if (graph->courses[v].timeslot == color) {
            return false;
        }
        curr = curr->next;
    }
    return true;
}

/* Compute student spacing gap score */
static double compute_schedule_quality(const Graph* graph) {
    if (!graph || graph->num_edges == 0) return 10.0;

    int total_clashes = 0;
    int weighted_gap_sum = 0;

    for (int u = 0; u < graph->num_vertices; u++) {
        AdjListNode* curr = graph->adj_list[u].head;
        while (curr) {
            int v = curr->dest;
            if (u < v) {
                int gap = abs(graph->courses[u].timeslot - graph->courses[v].timeslot);
                weighted_gap_sum += gap * curr->weight;
                total_clashes += curr->weight;
            }
            curr = curr->next;
        }
    }

    return (total_clashes > 0) ? (double)weighted_gap_sum / total_clashes : 0.0;
}

/* =========================================================================
 * 1. WELSH-POWELL ALGORITHM (Module 2 & 4)
 * ========================================================================= */
ColoringResult solve_welsh_powell(Graph* graph) {
    ColoringResult res = {0};
    strncpy(res.algorithm_name, "Welsh-Powell Heuristic", sizeof(res.algorithm_name) - 1);
    if (!graph || graph->num_vertices == 0) return res;

    reset_coloring(graph);
    clock_t start_time = clock();

    int n = graph->num_vertices;
    int* order = (int*)malloc(n * sizeof(int));
    int* degrees = (int*)malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        order[i] = i;
        degrees[i] = graph->adj_list[i].degree;
    }

    // Step 1: Pre-sort vertices in non-increasing degree order using Heap Sort (Module 1)
    heap_sort_vertices(order, degrees, n);

    // Step 2: Greedy Color Assignment
    int colored_count = 0;
    int current_color = 0;

    while (colored_count < n) {
        for (int i = 0; i < n; i++) {
            int u = order[i];
            if (graph->courses[u].timeslot == -1) {
                if (is_color_safe(graph, u, current_color)) {
                    graph->courses[u].timeslot = current_color;
                    colored_count++;
                }
            }
        }
        current_color++;
    }

    clock_t end_time = clock();
    res.execution_time_ms = (double)(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
    res.chromatic_number = current_color;
    res.total_conflicts_prevented = graph->num_edges;
    res.avg_student_gap_slots = compute_schedule_quality(graph);
    res.is_valid = true;

    // Check validity
    for (int u = 0; u < n; u++) {
        AdjListNode* curr = graph->adj_list[u].head;
        while (curr) {
            if (u < curr->dest && graph->courses[u].timeslot == graph->courses[curr->dest].timeslot) {
                res.is_valid = false;
                break;
            }
            curr = curr->next;
        }
    }

    free(order);
    free(degrees);
    return res;
}

/* =========================================================================
 * 2. DSATUR ALGORITHM (Degree of Saturation - Module 1 & 4)
 * ========================================================================= */
static int compute_vertex_saturation(const Graph* graph, int u, bool* color_seen, int max_possible_colors) {
    memset(color_seen, false, max_possible_colors * sizeof(bool));
    int sat = 0;
    AdjListNode* curr = graph->adj_list[u].head;
    while (curr) {
        int v = curr->dest;
        int c = graph->courses[v].timeslot;
        if (c != -1 && !color_seen[c]) {
            color_seen[c] = true;
            sat++;
        }
        curr = curr->next;
    }
    return sat;
}

ColoringResult solve_dsatur(Graph* graph) {
    ColoringResult res = {0};
    strncpy(res.algorithm_name, "DSATUR (Saturation Degree)", sizeof(res.algorithm_name) - 1);
    if (!graph || graph->num_vertices == 0) return res;

    reset_coloring(graph);
    clock_t start_time = clock();

    int n = graph->num_vertices;
    bool* color_seen = (bool*)calloc(n + 1, sizeof(bool));
    bool* used_colors = (bool*)calloc(n + 1, sizeof(bool));

    // Initialize Max-Heap Priority Queue (DSA Module 1)
    MaxHeap* heap = create_max_heap(n);
    for (int i = 0; i < n; i++) {
        // Initially, saturation degree = 0, tie-breaker = static degree
        insert_max_heap(heap, i, 0, graph->adj_list[i].degree);
    }

    int max_color_used = -1;

    for (int step = 0; step < n; step++) {
        // Extract vertex with maximum saturation degree
        HeapNode top = extract_max(heap);
        int u = top.vertex_id;

        // Find the lowest available color for vertex u
        memset(used_colors, false, (n + 1) * sizeof(bool));
        AdjListNode* curr = graph->adj_list[u].head;
        while (curr) {
            int v = curr->dest;
            if (graph->courses[v].timeslot != -1) {
                used_colors[graph->courses[v].timeslot] = true;
            }
            curr = curr->next;
        }

        int assigned_c = 0;
        while (used_colors[assigned_c]) {
            assigned_c++;
        }
        graph->courses[u].timeslot = assigned_c;
        if (assigned_c > max_color_used) max_color_used = assigned_c;

        // Update saturation degrees of all uncolored neighbors of u
        curr = graph->adj_list[u].head;
        while (curr) {
            int v = curr->dest;
            if (graph->courses[v].timeslot == -1) {
                int new_sat = compute_vertex_saturation(graph, v, color_seen, n + 1);
                graph->adj_list[v].saturation_degree = new_sat;

                // Count uncolored neighbors for tie breaking
                int uncolored_deg = 0;
                AdjListNode* v_curr = graph->adj_list[v].head;
                while (v_curr) {
                    if (graph->courses[v_curr->dest].timeslot == -1) {
                        uncolored_deg++;
                    }
                    v_curr = v_curr->next;
                }

                update_heap_priority(heap, v, new_sat, uncolored_deg);
            }
            curr = curr->next;
        }
    }

    clock_t end_time = clock();
    res.execution_time_ms = (double)(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
    res.chromatic_number = max_color_used + 1;
    res.total_conflicts_prevented = graph->num_edges;
    res.avg_student_gap_slots = compute_schedule_quality(graph);
    res.is_valid = true;

    // Check validity
    for (int u = 0; u < n; u++) {
        AdjListNode* curr = graph->adj_list[u].head;
        while (curr) {
            if (u < curr->dest && graph->courses[u].timeslot == graph->courses[curr->dest].timeslot) {
                res.is_valid = false;
                break;
            }
            curr = curr->next;
        }
    }

    free_max_heap(heap);
    free(color_seen);
    free(used_colors);
    return res;
}

/* =========================================================================
 * 3. EXACT BACKTRACKING SOLVER (Module 4: Backtracking & Branch and Bound)
 * ========================================================================= */
static bool backtracking_dfs(Graph* graph, int v, int max_colors, clock_t deadline, bool* timed_out) {
    if (clock() > deadline) {
        *timed_out = true;
        return false;
    }

    if (v == graph->num_vertices) {
        return true; // All vertices colored successfully with <= max_colors
    }

    for (int c = 0; c < max_colors; c++) {
        if (is_color_safe(graph, v, c)) {
            graph->courses[v].timeslot = c;

            // Recurse to next vertex
            if (backtracking_dfs(graph, v + 1, max_colors, deadline, timed_out)) {
                return true;
            }

            // Pruning: Backtrack
            graph->courses[v].timeslot = -1;
        }
    }

    return false;
}

ColoringResult solve_exact_backtracking(Graph* graph, int time_limit_sec) {
    ColoringResult res = {0};
    strncpy(res.algorithm_name, "Exact DFS Backtracking (Branch & Bound)", sizeof(res.algorithm_name) - 1);
    if (!graph || graph->num_vertices == 0) return res;

    reset_coloring(graph);
    clock_t start_time = clock();
    clock_t deadline = start_time + (clock_t)(time_limit_sec * CLOCKS_PER_SEC);

    int n = graph->num_vertices;
    int lower_bound = compute_max_clique_lower_bound(graph);
    if (lower_bound < 1) lower_bound = 1;

    bool timed_out = false;
    int optimal_k = n;

    // Iterative deepening over chromatic number k starting from lower bound omega(G)
    for (int k = lower_bound; k <= n; k++) {
        reset_coloring(graph);
        if (backtracking_dfs(graph, 0, k, deadline, &timed_out)) {
            optimal_k = k;
            break;
        }
        if (timed_out) {
            printf("[WARNING] Exact Backtracking reached time limit (%d s). Falling back to DSATUR bound.\n", time_limit_sec);
            return solve_dsatur(graph);
        }
    }

    clock_t end_time = clock();
    res.execution_time_ms = (double)(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
    res.chromatic_number = optimal_k;
    res.total_conflicts_prevented = graph->num_edges;
    res.avg_student_gap_slots = compute_schedule_quality(graph);
    res.is_valid = true;

    return res;
}

/* Auxiliary: Greedy Maximum Clique lower bound approximation omega(G) */
int compute_max_clique_lower_bound(const Graph* graph) {
    if (!graph || graph->num_vertices == 0) return 0;
    int n = graph->num_vertices;

    int max_clique = 1;
    for (int u = 0; u < n; u++) {
        int clique_size = 1;
        int clique_nodes[64] = {u};

        AdjListNode* curr = graph->adj_list[u].head;
        while (curr && clique_size < 64) {
            int cand = curr->dest;
            bool connects_to_all = true;
            for (int i = 0; i < clique_size; i++) {
                if (!has_conflict(graph, cand, clique_nodes[i])) {
                    connects_to_all = false;
                    break;
                }
            }
            if (connects_to_all) {
                clique_nodes[clique_size++] = cand;
            }
            curr = curr->next;
        }

        if (clique_size > max_clique) {
            max_clique = clique_size;
        }
    }
    return max_clique;
}
