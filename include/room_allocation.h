/**
 * @file room_allocation.h
 * @brief Dynamic Programming Resource Allocation for Examination Hall Seating.
 * @course Data Structures and Algorithms - II (CCSE0301) - Module 3 (Dynamic Programming)
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#ifndef ROOM_ALLOCATION_H
#define ROOM_ALLOCATION_H

#include "graph.h"
#include <stdbool.h>

#define MAX_ROOM_CODE_LEN 16
#define MAX_BUILDING_LEN 32

/**
 * @brief Structure representing a physical examination hall.
 */
typedef struct Room {
    int id;                             /**< Room identifier */
    char code[MAX_ROOM_CODE_LEN];       /**< Room number / code, e.g. "LH-101" */
    char building[MAX_BUILDING_LEN];    /**< Academic block */
    int capacity;                       /**< Seating capacity */
    int current_occupancy;              /**< Allocated students in current slot */
} Room;

/**
 * @brief Represents a single seating allocation assignment.
 */
typedef struct AllocationEntry {
    int course_id;                      /**< Course identifier */
    int room_id;                        /**< Room identifier */
    int allocated_students;             /**< Number of students assigned to this room */
} AllocationEntry;

/**
 * @brief Schedule allocation result for a specific time slot.
 */
typedef struct SlotRoomSchedule {
    int timeslot;                       /**< Time slot index */
    int num_entries;                    /**< Number of allocation records */
    AllocationEntry* entries;           /**< Array of allocation entries */
    double utilization_pct;             /**< Percentage of capacity utilized */
} SlotRoomSchedule;

/* Function Declarations */
Room* load_rooms_from_csv(const char* rooms_file, int* num_rooms);
void free_rooms(Room* rooms);

/**
 * @brief Allocates examination halls for each time slot using Dynamic Programming.
 * Applies 0/1 Knapsack recurrence: DP[i, w] = max(DP[i-1, w], DP[i-1, w - w_i] + v_i)
 */
SlotRoomSchedule* allocate_rooms_dp(const Graph* graph, int num_timeslots, const Room* rooms, int num_rooms);
void free_slot_schedules(SlotRoomSchedule* schedules, int num_timeslots);
void print_room_allocation_summary(const SlotRoomSchedule* schedules, int num_timeslots, const Room* rooms, int num_rooms, const Graph* graph);

#endif /* ROOM_ALLOCATION_H */
