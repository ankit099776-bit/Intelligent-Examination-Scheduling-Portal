/**
 * @file test_suite.h
 * @brief Automated verification suite covering architectural test cases TC01 to TC07.
 * @course Data Structures and Algorithms - II (CCSE0301)
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <stdbool.h>

/**
 * @brief Runs all 7 test cases from the official PBL verification matrix.
 * @return true if all test cases pass with zero constraint violations.
 */
bool run_all_verification_tests(void);

/* Individual Test Case Functions */
bool test_tc01_disconnected_graph(void);
bool test_tc02_direct_conflict_edge(void);
bool test_tc03_triangle_3_clique(void);
bool test_tc04_star_graph_hub(void);
bool test_tc05_room_capacity_deficit_split(void);
bool test_tc06_dense_elective_cluster(void);
bool test_tc07_invalid_input_sanitization(void);

#endif /* TEST_SUITE_H */
