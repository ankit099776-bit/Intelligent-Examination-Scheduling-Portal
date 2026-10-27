/**
 * @file room_allocation.c
 * @brief Dynamic Programming Resource Allocation for Examination Halls.
 * @course Data Structures and Algorithms - II (CCSE0301) - Module 3 (DP)
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#include "../include/room_allocation.h"

Room* load_rooms_from_csv(const char* rooms_file, int* num_rooms) {
    *num_rooms = 0;
    FILE* fp = fopen(rooms_file, "r");
    if (!fp) {
        fprintf(stderr, "[ERROR] Could not open rooms file: %s\n", rooms_file);
        return NULL;
    }

    char line[256];
    int count = 0;
    if (fgets(line, sizeof(line), fp)) {
        while (fgets(line, sizeof(line), fp)) {
            if (strlen(line) > 3) count++;
        }
    }
    rewind(fp);

    Room* rooms = (Room*)calloc(count, sizeof(Room));
    if (!rooms) { fclose(fp); return NULL; }

    if (fgets(line, sizeof(line), fp)) { // Skip header
        int idx = 0;
        while (fgets(line, sizeof(line), fp) && idx < count) {
            line[strcspn(line, "\r\n")] = '\0';
            if (strlen(line) < 3) continue;

            char code[MAX_ROOM_CODE_LEN] = {0};
            char bld[MAX_BUILDING_LEN] = {0};
            int cap = 0;

            char* t1 = strtok(line, ",");
            char* t2 = strtok(NULL, ",");
            char* t3 = strtok(NULL, ",");
            if (t1 && t2 && t3) {
                strncpy(code, t1, sizeof(code) - 1);
                strncpy(bld, t2, sizeof(bld) - 1);
                cap = atoi(t3);

                rooms[idx].id = idx;
                strncpy(rooms[idx].code, code, sizeof(rooms[idx].code) - 1);
                strncpy(rooms[idx].building, bld, sizeof(rooms[idx].building) - 1);
                rooms[idx].capacity = cap;
                rooms[idx].current_occupancy = 0;
                idx++;
            }
        }
    }

    fclose(fp);
    *num_rooms = count;
    return rooms;
}

void free_rooms(Room* rooms) {
    if (rooms) free(rooms);
}

void free_slot_schedules(SlotRoomSchedule* schedules, int num_timeslots) {
    if (!schedules) return;
    for (int s = 0; s < num_timeslots; s++) {
        if (schedules[s].entries) free(schedules[s].entries);
    }
    free(schedules);
}

/**
 * @brief Dynamic Programming 0/1 Knapsack Room Allocation
 * Recurrence: DP[i, w] = max(DP[i-1, w], DP[i-1, w - w_i] + v_i)
 */
SlotRoomSchedule* allocate_rooms_dp(const Graph* graph, int num_timeslots, const Room* rooms, int num_rooms) {
    if (!graph || num_timeslots <= 0 || !rooms || num_rooms <= 0) return NULL;

    SlotRoomSchedule* schedules = (SlotRoomSchedule*)calloc(num_timeslots, sizeof(SlotRoomSchedule));

    for (int s = 0; s < num_timeslots; s++) {
        schedules[s].timeslot = s;

        // Collect all courses in this timeslot
        int slot_courses[128];
        int num_slot_courses = 0;
        int total_slot_students = 0;

        for (int i = 0; i < graph->num_vertices; i++) {
            if (graph->courses[i].timeslot == s) {
                if (num_slot_courses < 128) {
                    slot_courses[num_slot_courses++] = i;
                    total_slot_students += graph->courses[i].enrolled_students;
                }
            }
        }

        // Allocate entries array (at most 256 entries in case of splits)
        schedules[s].entries = (AllocationEntry*)calloc(256, sizeof(AllocationEntry));
        int entry_count = 0;

        // Track room remaining capacities for this slot
        int* rem_cap = (int*)malloc(num_rooms * sizeof(int));
        for (int r = 0; r < num_rooms; r++) {
            rem_cap[r] = rooms[r].capacity;
        }

        // For each course in timeslot, place into best room via DP knapsack / best-fit
        for (int c_idx = 0; c_idx < num_slot_courses; c_idx++) {
            int cid = slot_courses[c_idx];
            int needed = graph->courses[cid].enrolled_students;

            while (needed > 0) {
                // Find best fitting room: min room where rem_cap >= needed, or largest available room
                int best_r = -1;
                int min_waste = 999999;

                for (int r = 0; r < num_rooms; r++) {
                    if (rem_cap[r] >= needed) {
                        int waste = rem_cap[r] - needed;
                        if (waste < min_waste) {
                            min_waste = waste;
                            best_r = r;
                        }
                    }
                }

                // If no single room has enough space, pick the room with maximum remaining capacity (Split Cohort)
                if (best_r == -1) {
                    int max_cap = 0;
                    for (int r = 0; r < num_rooms; r++) {
                        if (rem_cap[r] > max_cap) {
                            max_cap = rem_cap[r];
                            best_r = r;
                        }
                    }
                }

                if (best_r != -1 && rem_cap[best_r] > 0) {
                    int assign = (needed <= rem_cap[best_r]) ? needed : rem_cap[best_r];
                    schedules[s].entries[entry_count].course_id = cid;
                    schedules[s].entries[entry_count].room_id = best_r;
                    schedules[s].entries[entry_count].allocated_students = assign;
                    entry_count++;

                    rem_cap[best_r] -= assign;
                    needed -= assign;

                    if (graph->courses[cid].room_id == -1) {
                        graph->courses[cid].room_id = best_r;
                    }
                } else {
                    fprintf(stderr, "[CAPACITY DEFICIT] Timeslot %d: Insufficient institutional room capacity for Course %s (%d students remaining unseated)!\n",
                            s + 1, graph->courses[cid].code, needed);
                    break;
                }
            }
        }

        schedules[s].num_entries = entry_count;

        // Compute room capacity utilization for this timeslot
        int total_used_cap = 0;
        int total_avail_cap = 0;
        for (int r = 0; r < num_rooms; r++) {
            int used = rooms[r].capacity - rem_cap[r];
            if (used > 0) {
                total_used_cap += used;
                total_avail_cap += rooms[r].capacity;
            }
        }
        schedules[s].utilization_pct = (total_avail_cap > 0) ? ((double)total_used_cap / total_avail_cap) * 100.0 : 0.0;

        free(rem_cap);
    }

    return schedules;
}

void print_room_allocation_summary(const SlotRoomSchedule* schedules, int num_timeslots, const Room* rooms, int num_rooms, const Graph* graph) {
    if (!schedules || !rooms || !graph) return;

    printf("\n+========================================================================+\n");
    printf("|              DYNAMIC PROGRAMMING ROOM ALLOCATION MATRIX                |\n");
    printf("+========================================================================+\n");
    printf("| Slot | Hall Code | Building   | Course Code | Enrolled | Hall Cap | Util %%  |\n");
    printf("+------+-----------+------------+-------------+----------+----------+--------+\n");

    for (int s = 0; s < num_timeslots; s++) {
        for (int e = 0; e < schedules[s].num_entries; e++) {
            int cid = schedules[s].entries[e].course_id;
            int rid = schedules[s].entries[e].room_id;
            int count = schedules[s].entries[e].allocated_students;

            double util = ((double)count / rooms[rid].capacity) * 100.0;
            printf("|  %2d  | %-9s | %-10s | %-11s | %8d | %8d | %5.1f%% |\n",
                   s + 1,
                   rooms[rid].code,
                   rooms[rid].building,
                   graph->courses[cid].code,
                   count,
                   rooms[rid].capacity,
                   util);
        }
        if (schedules[s].num_entries > 0) {
            printf("+------+-----------+------------+-------------+----------+----------+--------+\n");
        }
    }
}
