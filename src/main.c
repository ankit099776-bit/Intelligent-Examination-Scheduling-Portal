/**
 * @file main.c
 * @brief Main Entry Point & CLI for Intelligent Examination Scheduling System.
 * @course Data Structures and Algorithms - II (CCSE0301)
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../include/graph.h"
#include "../include/heap.h"
#include "../include/coloring.h"
#include "../include/room_allocation.h"
#include "../include/validator.h"
#include "../include/test_suite.h"

static void print_banner(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("         INTELLIGENT EXAMINATION SCHEDULING SYSTEM (GRAPH COLORING & DP)        \n");
    printf("         Course: Data Structures and Algorithms - II (CCSE0301)                 \n");
    printf("         Student: Ankit Pandey | ERP ID: 2501331520029 | NIET Gr. Noida          \n");
    printf("================================================================================\n");
}

static void print_menu(void) {
    printf("\n----------------------------- MAIN CONTROL MENU --------------------------------\n");
    printf(" [1] Run Automated Verification Test Suite (TC01 - TC07)\n");
    printf(" [2] Ingest Academic Dataset & Schedule via DSATUR Heuristic\n");
    printf(" [3] Ingest Academic Dataset & Schedule via Welsh-Powell Heuristic\n");
    printf(" [4] Multi-Algorithm Benchmark (Welsh-Powell vs. DSATUR vs. Backtracking)\n");
    printf(" [5] Export Timetable (CSV, JSON, Audit Log, and HTML Dashboard)\n");
    printf(" [0] Exit System\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("Select an option [0-5]: ");
}

static void run_scheduling_workflow(const char* c_file, const char* e_file, const char* r_file, int algo_choice) {
    printf("\n[STEP 1] Ingesting course catalog and student enrollment registrations...\n");
    Graph* graph = load_dataset_from_csv(c_file, e_file);
    if (!graph) {
        printf("[ERROR] Failed to construct conflict graph from dataset!\n");
        return;
    }

    print_graph_summary(graph);

    printf("\n[STEP 2] Executing Graph Coloring Algorithm...\n");
    ColoringResult cr;
    if (algo_choice == 1) {
        cr = solve_dsatur(graph);
    } else {
        cr = solve_welsh_powell(graph);
    }

    printf("\n+------------------------------------------------------------------------+\n");
    printf("|  ALGORITHM EXECUTION METRICS                                           |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("|  Selected Algorithm      : %-42s  |\n", cr.algorithm_name);
    printf("|  Chromatic Number chi(G) : %-2d Time Slots                           |\n", cr.chromatic_number);
    printf("|  Execution Runtime       : %-6.3f ms                                   |\n", cr.execution_time_ms);
    printf("|  Conflict Clashes Saved  : %-42d  |\n", cr.total_conflicts_prevented);
    printf("|  Mean Student Slot Gap   : %-4.2f Slots (Higher gap = better rest)     |\n", cr.avg_student_gap_slots);
    printf("+------------------------------------------------------------------------+\n");

    int num_rooms = 0;
    Room* rooms = load_rooms_from_csv(r_file, &num_rooms);
    if (!rooms) {
        printf("[WARNING] Could not load rooms file. Skipping room allocation.\n");
        free_graph(graph);
        return;
    }

    printf("\n[STEP 3] Allocating Examination Halls via Dynamic Programming 0/1 Knapsack...\n");
    SlotRoomSchedule* room_sched = allocate_rooms_dp(graph, cr.chromatic_number, rooms, num_rooms);

    print_room_allocation_summary(room_sched, cr.chromatic_number, rooms, num_rooms, graph);

    printf("\n[STEP 4] Running Independent Hard Constraint Validation Gatekeeper...\n");
    ValidationReport rep = validate_complete_schedule(graph, room_sched, cr.chromatic_number, rooms, num_rooms);
    print_validation_report(&rep);

    printf("\n[STEP 5] Exporting Deliverables to Disk...\n");
    export_timetable_csv("timetable_output.csv", graph, room_sched, cr.chromatic_number, rooms, num_rooms);
    printf("  [OK] Saved CSV Timetable -> timetable_output.csv\n");

    export_timetable_json("timetable_output.json", graph, room_sched, cr.chromatic_number, rooms, num_rooms);
    printf("  [OK] Saved JSON Timetable -> timetable_output.json\n");

    export_audit_log("scheduling_audit_log.txt", graph, room_sched, cr.chromatic_number, rooms, num_rooms, &rep);
    printf("  [OK] Saved Explainability Audit Log -> scheduling_audit_log.txt\n");

    export_html_dashboard("web/index.html", graph, room_sched, cr.chromatic_number, rooms, num_rooms, &rep);
    printf("  [OK] Saved Web Dashboard -> web/index.html\n");

    free_slot_schedules(room_sched, cr.chromatic_number);
    free_rooms(rooms);
    free_graph(graph);
}

static void run_benchmark_comparison(const char* c_file, const char* e_file) {
    Graph* g1 = load_dataset_from_csv(c_file, e_file);
    Graph* g2 = load_dataset_from_csv(c_file, e_file);
    Graph* g3 = load_dataset_from_csv(c_file, e_file);
    if (!g1 || !g2 || !g3) {
        printf("[ERROR] Failed to load dataset for benchmark.\n");
        if (g1) free_graph(g1);
        if (g2) free_graph(g2);
        if (g3) free_graph(g3);
        return;
    }

    printf("\n+========================================================================+\n");
    printf("|          EMPIRICAL ALGORITHM BENCHMARKING (WELSH-POWELL vs DSATUR)     |\n");
    printf("+========================================================================+\n");
    int clique_lb = compute_max_clique_lower_bound(g1);
    printf("|  Theoretical Lower Bound (Maximum Clique omega(G)): %-18d |\n", clique_lb);
    printf("+------------------------------------------------------------------------+\n");

    ColoringResult r_wp = solve_welsh_powell(g1);
    ColoringResult r_ds = solve_dsatur(g2);
    ColoringResult r_bt = solve_exact_backtracking(g3, 2); // 2 second safety timeout

    printf("| Metric                     | Welsh-Powell      | DSATUR            | Exact DFS Backtrack |\n");
    printf("+----------------------------+-------------------+-------------------+---------------------+\n");
    printf("| Chromatic Number chi(G)    | %-17d | %-17d | %-19d |\n", r_wp.chromatic_number, r_ds.chromatic_number, r_bt.chromatic_number);
    printf("| Execution Time (ms)        | %-17.3f | %-17.3f | %-19.3f |\n", r_wp.execution_time_ms, r_ds.execution_time_ms, r_bt.execution_time_ms);
    printf("| Invariant Verification     | %-17s | %-17s | %-19s |\n", r_wp.is_valid ? "VALID (No Clash)" : "INVALID", r_ds.is_valid ? "VALID (No Clash)" : "INVALID", r_bt.is_valid ? "VALID (No Clash)" : "INVALID");
    printf("| Student Gap Score          | %-17.2f | %-17.2f | %-19.2f |\n", r_wp.avg_student_gap_slots, r_ds.avg_student_gap_slots, r_bt.avg_student_gap_slots);
    printf("+----------------------------+-------------------+-------------------+---------------------+\n");

    free_graph(g1);
    free_graph(g2);
    free_graph(g3);
}

int main(int argc, char* argv[]) {
    print_banner();

    const char* c_file = "data/sample_courses.csv";
    const char* e_file = "data/sample_enrollments.csv";
    const char* r_file = "data/sample_rooms.csv";

    // If CLI arguments provided, support batch automated mode
    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        bool ok = run_all_verification_tests();
        return ok ? 0 : 1;
    }
    if (argc > 1 && strcmp(argv[1], "--run") == 0) {
        run_scheduling_workflow(c_file, e_file, r_file, 1);
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--benchmark") == 0) {
        run_benchmark_comparison(c_file, e_file);
        return 0;
    }

    int choice = -1;
    while (choice != 0) {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // clear buffer
            choice = -1;
            continue;
        }

        switch (choice) {
            case 1:
                run_all_verification_tests();
                break;
            case 2:
                run_scheduling_workflow(c_file, e_file, r_file, 1); // DSATUR
                break;
            case 3:
                run_scheduling_workflow(c_file, e_file, r_file, 2); // Welsh-Powell
                break;
            case 4:
                run_benchmark_comparison(c_file, e_file);
                break;
            case 5:
                run_scheduling_workflow(c_file, e_file, r_file, 1);
                printf("\n[SUCCESS] All files exported successfully!\n");
                break;
            case 0:
                printf("\nExiting Intelligent Examination Scheduling System. Academic Project CCSE0301.\n\n");
                break;
            default:
                printf("\n[ERROR] Invalid option. Please select 0 to 5.\n");
                break;
        }
    }

    return 0;
}
