/**
 * @file validator.h
 * @brief Independent Hard Constraint Gatekeeper and Schedule Explainability Engine.
 * @course Data Structures and Algorithms - II (CCSE0301)
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "graph.h"
#include "room_allocation.h"
#include <stdbool.h>

/**
 * @brief Verification report detailing hard constraint adherence.
 */
typedef struct ValidationReport {
    int total_courses;                  /**< Total courses verified */
    int total_edges_checked;            /**< Total conflict pairs evaluated */
    int conflict_violations;            /**< Violations where slot(u) == slot(v) */
    int capacity_violations;            /**< Violations where allocated > room_cap */
    int unassigned_courses;             /**< Courses missing timeslots */
    double overall_room_utilization;    /**< Mean room seating efficiency % */
    bool all_hard_constraints_satisfied;/**< Overall feasibility flag */
} ValidationReport;

/* Function Declarations */
ValidationReport validate_complete_schedule(const Graph* graph, const SlotRoomSchedule* room_sched, int num_slots, const Room* rooms, int num_rooms);
void print_validation_report(const ValidationReport* report);
void export_audit_log(const char* filepath, const Graph* graph, const SlotRoomSchedule* room_sched, int num_slots, const Room* rooms, int num_rooms, const ValidationReport* report);
void export_timetable_csv(const char* filepath, const Graph* graph, const SlotRoomSchedule* room_sched, int num_slots, const Room* rooms, int num_rooms);
void export_timetable_json(const char* filepath, const Graph* graph, const SlotRoomSchedule* room_sched, int num_slots, const Room* rooms, int num_rooms);
void export_html_dashboard(const char* filepath, const Graph* graph, const SlotRoomSchedule* room_sched, int num_slots, const Room* rooms, int num_rooms, const ValidationReport* report);

#endif /* VALIDATOR_H */
