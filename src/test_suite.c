/**
 * @file test_suite.c
 * @brief Automated Test Suite for Verification Matrix TC01 to TC07.
 * @course Data Structures and Algorithms - II (CCSE0301)
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#include "../include/test_suite.h"
#include "../include/graph.h"
#include "../include/coloring.h"
#include "../include/room_allocation.h"
#include "../include/validator.h"

static void print_test_header(const char* id, const char* name) {
    printf("[TEST] %s: %s ... ", id, name);
}

static void print_test_result(bool passed, const char* details) {
    if (passed) {
        printf("PASSED (%s)\n", details);
    } else {
        printf("FAILED! (%s)\n", details);
    }
}

// TC01: Disconnected Graph (|V|=6, |E|=0)
bool test_tc01_disconnected_graph(void) {
    print_test_header("TC01", "Disconnected Graph (Zero Conflicts)");
    Graph* g = create_graph(6);
    for (int i = 0; i < 6; i++) {
        snprintf(g->courses[i].code, sizeof(g->courses[i].code), "C%d", i + 1);
        g->courses[i].enrolled_students = 50;
    }

    ColoringResult cr = solve_dsatur(g);
    bool pass = (cr.is_valid && cr.chromatic_number == 1);

    char msg[64];
    snprintf(msg, sizeof(msg), "Chromatic number = %d, expected 1", cr.chromatic_number);
    print_test_result(pass, msg);

    free_graph(g);
    return pass;
}

// TC02: Direct Conflict Edge (|V|=2, |E|=1)
bool test_tc02_direct_conflict_edge(void) {
    print_test_header("TC02", "Direct Conflict Edge (Shared Students)");
    Graph* g = create_graph(2);
    snprintf(g->courses[0].code, sizeof(g->courses[0].code), "CS101");
    snprintf(g->courses[1].code, sizeof(g->courses[1].code), "CS102");
    add_conflict_edge(g, 0, 1, 15);

    ColoringResult cr = solve_dsatur(g);
    bool pass = (cr.is_valid && cr.chromatic_number == 2 &&
                 g->courses[0].timeslot != g->courses[1].timeslot);

    char msg[64];
    snprintf(msg, sizeof(msg), "Slots: C0=%d, C1=%d", g->courses[0].timeslot, g->courses[1].timeslot);
    print_test_result(pass, msg);

    free_graph(g);
    return pass;
}

// TC03: Triangle Conflict Graph (3-Clique K3)
bool test_tc03_triangle_3_clique(void) {
    print_test_header("TC03", "Triangle Conflict Graph (3-Clique K3)");
    Graph* g = create_graph(3);
    for (int i = 0; i < 3; i++) {
        snprintf(g->courses[i].code, sizeof(g->courses[i].code), "CLIQ_%d", i + 1);
    }
    add_conflict_edge(g, 0, 1, 20);
    add_conflict_edge(g, 1, 2, 25);
    add_conflict_edge(g, 0, 2, 30);

    ColoringResult cr = solve_dsatur(g);
    int lower_bound = compute_max_clique_lower_bound(g);

    bool pass = (cr.is_valid && cr.chromatic_number == 3 && lower_bound == 3);
    char msg[64];
    snprintf(msg, sizeof(msg), "chi(G)=%d, omega(G)=%d", cr.chromatic_number, lower_bound);
    print_test_result(pass, msg);

    free_graph(g);
    return pass;
}

// TC04: Star Graph (Core Hub Course)
bool test_tc04_star_graph_hub(void) {
    print_test_header("TC04", "Star Graph (Hub Course connected to 5 leaves)");
    Graph* g = create_graph(6);
    snprintf(g->courses[0].code, sizeof(g->courses[0].code), "HUB_0");
    for (int i = 1; i < 6; i++) {
        snprintf(g->courses[i].code, sizeof(g->courses[i].code), "LEAF_%d", i);
        add_conflict_edge(g, 0, i, 10);
    }

    ColoringResult cr = solve_dsatur(g);
    bool pass = (cr.is_valid && cr.chromatic_number == 2 && g->courses[0].timeslot == 0);

    char msg[64];
    snprintf(msg, sizeof(msg), "chi(G)=%d, HubSlot=%d", cr.chromatic_number, g->courses[0].timeslot);
    print_test_result(pass, msg);

    free_graph(g);
    return pass;
}

// TC05: Room Capacity Deficit & Multi-Hall Split
bool test_tc05_room_capacity_deficit_split(void) {
    print_test_header("TC05", "Room Capacity Deficit & Multi-Hall Split");
    Graph* g = create_graph(1);
    snprintf(g->courses[0].code, sizeof(g->courses[0].code), "BIG_EXAM");
    g->courses[0].enrolled_students = 180;
    g->courses[0].timeslot = 0;

    Room rooms[2];
    rooms[0].id = 0;
    strncpy(rooms[0].code, "LH-A", sizeof(rooms[0].code));
    rooms[0].capacity = 100;

    rooms[1].id = 1;
    strncpy(rooms[1].code, "LH-B", sizeof(rooms[1].code));
    rooms[1].capacity = 100;

    SlotRoomSchedule* sched = allocate_rooms_dp(g, 1, rooms, 2);
    bool pass = (sched && sched[0].num_entries == 2 &&
                 (sched[0].entries[0].allocated_students + sched[0].entries[1].allocated_students) == 180);

    char msg[64];
    snprintf(msg, sizeof(msg), "Split into %d halls: %d + %d students",
             sched ? sched[0].num_entries : 0,
             sched ? sched[0].entries[0].allocated_students : 0,
             sched ? sched[0].entries[1].allocated_students : 0);
    print_test_result(pass, msg);

    free_slot_schedules(sched, 1);
    free_graph(g);
    return pass;
}

// TC06: Dense Elective Cluster
bool test_tc06_dense_elective_cluster(void) {
    print_test_header("TC06", "Dense Elective Cluster (10 Courses, High Density)");
    Graph* g = create_graph(10);
    for (int i = 0; i < 10; i++) {
        snprintf(g->courses[i].code, sizeof(g->courses[i].code), "ELEC_%d", i);
    }
    // Form dense cluster
    for (int i = 0; i < 10; i++) {
        for (int j = i + 1; j < 10; j++) {
            if ((i + j) % 3 != 0) {
                add_conflict_edge(g, i, j, 5);
            }
        }
    }

    ColoringResult cr_dsatur = solve_dsatur(g);
    ColoringResult cr_wp = solve_welsh_powell(g);

    bool pass = (cr_dsatur.is_valid && cr_dsatur.chromatic_number <= cr_wp.chromatic_number);
    char msg[64];
    snprintf(msg, sizeof(msg), "DSATUR chi=%d <= Welsh-Powell chi=%d", cr_dsatur.chromatic_number, cr_wp.chromatic_number);
    print_test_result(pass, msg);

    free_graph(g);
    return pass;
}

// TC07: Invalid Input Records & Sanitization
bool test_tc07_invalid_input_sanitization(void) {
    print_test_header("TC07", "Input Sanitization & Boundary Protections");
    Graph* g_null = create_graph(0);
    bool pass1 = (g_null == NULL);

    Graph* g = create_graph(3);
    // Attempt invalid edge out of bounds
    add_conflict_edge(g, -1, 5, 10);
    add_conflict_edge(g, 1, 1, 10); // Self loop
    bool pass2 = (g->num_edges == 0);

    bool pass = (pass1 && pass2);
    print_test_result(pass, "Gracefully handled null graph and invalid edge bounds");

    free_graph(g);
    return pass;
}

bool run_all_verification_tests(void) {
    printf("\n+========================================================================+\n");
    printf("|          STARTING AUTOMATED VERIFICATION TEST SUITE (TC01 - TC07)      |\n");
    printf("+========================================================================+\n");

    bool all_passed = true;
    all_passed &= test_tc01_disconnected_graph();
    all_passed &= test_tc02_direct_conflict_edge();
    all_passed &= test_tc03_triangle_3_clique();
    all_passed &= test_tc04_star_graph_hub();
    all_passed &= test_tc05_room_capacity_deficit_split();
    all_passed &= test_tc06_dense_elective_cluster();
    all_passed &= test_tc07_invalid_input_sanitization();

    printf("+------------------------------------------------------------------------+\n");
    if (all_passed) {
        printf("|  ALL 7 ARCHITECTURAL TEST CASES PASSED WITH 100%% VERIFICATION!        |\n");
    } else {
        printf("|  TEST FAILURES DETECTED! PLEASE REVIEW LOGS.                           |\n");
    }
    printf("+========================================================================+\n\n");
    return all_passed;
}
