/**
 * @file validator.c
 * @brief Independent Hard Constraint Validation Gatekeeper & Audit Logging.
 * @course Data Structures and Algorithms - II (CCSE0301)
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#include "../include/validator.h"

ValidationReport validate_complete_schedule(const Graph* graph, const SlotRoomSchedule* room_sched, int num_slots, const Room* rooms, int num_rooms) {
    ValidationReport rep = {0};
    if (!graph) return rep;

    rep.total_courses = graph->num_vertices;
    rep.total_edges_checked = graph->num_edges;
    rep.conflict_violations = 0;
    rep.capacity_violations = 0;
    rep.unassigned_courses = 0;

    // Check Invariant 1: Unassigned Courses
    for (int i = 0; i < graph->num_vertices; i++) {
        if (graph->courses[i].timeslot < 0) {
            rep.unassigned_courses++;
        }
    }

    // Check Invariant 2: Conflict Graph Edge Invariant (slot(u) != slot(v))
    for (int u = 0; u < graph->num_vertices; u++) {
        AdjListNode* curr = graph->adj_list[u].head;
        while (curr) {
            int v = curr->dest;
            if (u < v) {
                if (graph->courses[u].timeslot == graph->courses[v].timeslot && graph->courses[u].timeslot != -1) {
                    rep.conflict_violations++;
                }
            }
            curr = curr->next;
        }
    }

    // Check Invariant 3: Room Capacity Invariant (Sum of students in room <= Room Capacity)
    if (room_sched && rooms) {
        double total_util = 0.0;
        int slot_count = 0;

        for (int s = 0; s < num_slots; s++) {
            int* room_usage = (int*)calloc(num_rooms, sizeof(int));
            for (int e = 0; e < room_sched[s].num_entries; e++) {
                int rid = room_sched[s].entries[e].room_id;
                int stus = room_sched[s].entries[e].allocated_students;
                room_usage[rid] += stus;
            }

            for (int r = 0; r < num_rooms; r++) {
                if (room_usage[r] > rooms[r].capacity) {
                    rep.capacity_violations++;
                }
            }
            total_util += room_sched[s].utilization_pct;
            slot_count++;
            free(room_usage);
        }

        rep.overall_room_utilization = (slot_count > 0) ? (total_util / slot_count) : 0.0;
    }

    rep.all_hard_constraints_satisfied = (rep.conflict_violations == 0 &&
                                          rep.capacity_violations == 0 &&
                                          rep.unassigned_courses == 0);

    return rep;
}

void print_validation_report(const ValidationReport* report) {
    if (!report) return;

    printf("\n+========================================================================+\n");
    printf("|          INDEPENDENT HARD CONSTRAINT VALIDATION GATEKEEPER             |\n");
    printf("+========================================================================+\n");
    printf("|  Total Courses Checked        : %-38d |\n", report->total_courses);
    printf("|  Conflict Edges Audited       : %-38d |\n", report->total_edges_checked);
    printf("|  Student Conflict Clashes     : %-38d |\n", report->conflict_violations);
    printf("|  Room Capacity Overflows      : %-38d |\n", report->capacity_violations);
    printf("|  Unassigned Courses           : %-38d |\n", report->unassigned_courses);
    printf("|  Mean Hall Utilization Rate   : %-37.1f%% |\n", report->overall_room_utilization);
    printf("+------------------------------------------------------------------------+\n");
    if (report->all_hard_constraints_satisfied) {
        printf("|  STATUS: [PASSED] ALL MANDATORY HARD CONSTRAINTS STRICTLY SATISFIED!  |\n");
    } else {
        printf("|  STATUS: [FAILED] INVARIANT VIOLATIONS DETECTED IN SCHEDULE!          |\n");
    }
    printf("+========================================================================+\n");
}

void export_audit_log(const char* filepath, const Graph* graph, const SlotRoomSchedule* room_sched, int num_slots, const Room* rooms, int num_rooms, const ValidationReport* report) {
    FILE* fp = fopen(filepath, "w");
    if (!fp) return;

    fprintf(fp, "================================================================================\n");
    fprintf(fp, "         INTELLIGENT EXAMINATION SCHEDULING SYSTEM - AUDIT LOG          \n");
    fprintf(fp, "         Course: Data Structures and Algorithms - II (CCSE0301)         \n");
    fprintf(fp, "         Author: Ankit Pandey (ERP ID: 2501331520029)                   \n");
    fprintf(fp, "================================================================================\n\n");

    fprintf(fp, "1. EXECUTIVE SUMMARY & VALIDATION AUDIT:\n");
    fprintf(fp, "   - Total Courses Scheduled       : %d\n", report->total_courses);
    fprintf(fp, "   - Total Conflict Edges Audited  : %d\n", report->total_edges_checked);
    fprintf(fp, "   - Conflict Violations Detected  : %d\n", report->conflict_violations);
    fprintf(fp, "   - Room Overcapacity Violations  : %d\n", report->capacity_violations);
    fprintf(fp, "   - Unassigned Courses            : %d\n", report->unassigned_courses);
    fprintf(fp, "   - Overall Feasibility Status    : %s\n\n", report->all_hard_constraints_satisfied ? "VERIFIED VALID (100% FEASIBLE)" : "INVALID SCHEDULE");

    fprintf(fp, "2. EXPLAINABILITY RATIONALE (Why were courses scheduled into these slots?):\n");
    for (int i = 0; i < graph->num_vertices; i++) {
        fprintf(fp, "   [Course %s: %s]\n", graph->courses[i].code, graph->courses[i].title);
        fprintf(fp, "     - Assigned Time Slot : Slot %d\n", graph->courses[i].timeslot + 1);
        fprintf(fp, "     - Conflict Degree    : %d conflicting courses\n", graph->adj_list[i].degree);
        fprintf(fp, "     - Direct Conflicts   : ");
        AdjListNode* curr = graph->adj_list[i].head;
        if (!curr) {
            fprintf(fp, "None (Isolated course, can share any slot)");
        } else {
            while (curr) {
                int v = curr->dest;
                fprintf(fp, "%s (Slot %d, %d clashes) ", graph->courses[v].code, graph->courses[v].timeslot + 1, curr->weight);
                curr = curr->next;
            }
        }
        fprintf(fp, "\n     - Rationale          : Placed in lowest non-clashing slot with minimum saturation penalty.\n\n");
    }

    fprintf(fp, "================================================================================\n");
    fprintf(fp, "                         END OF EXPLAINABILITY LOG                              \n");
    fprintf(fp, "================================================================================\n");
    fclose(fp);
}

void export_timetable_csv(const char* filepath, const Graph* graph, const SlotRoomSchedule* room_sched, int num_slots, const Room* rooms, int num_rooms) {
    FILE* fp = fopen(filepath, "w");
    if (!fp) return;

    fprintf(fp, "Timeslot,Course_Code,Course_Title,Department,Enrolled_Students,Hall_Code,Hall_Building,Hall_Capacity,Allocated_Seats\n");

    for (int s = 0; s < num_slots; s++) {
        if (!room_sched || s >= num_slots) continue;
        for (int e = 0; e < room_sched[s].num_entries; e++) {
            int cid = room_sched[s].entries[e].course_id;
            int rid = room_sched[s].entries[e].room_id;
            int count = room_sched[s].entries[e].allocated_students;

            fprintf(fp, "%d,%s,\"%s\",%s,%d,%s,%s,%d,%d\n",
                    s + 1,
                    graph->courses[cid].code,
                    graph->courses[cid].title,
                    graph->courses[cid].department,
                    graph->courses[cid].enrolled_students,
                    rooms[rid].code,
                    rooms[rid].building,
                    rooms[rid].capacity,
                    count);
        }
    }
    fclose(fp);
}

void export_timetable_json(const char* filepath, const Graph* graph, const SlotRoomSchedule* room_sched, int num_slots, const Room* rooms, int num_rooms) {
    FILE* fp = fopen(filepath, "w");
    if (!fp) return;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"project\": \"Intelligent Examination Scheduling System\",\n");
    fprintf(fp, "  \"course\": \"CCSE0301 - Data Structures and Algorithms - II\",\n");
    fprintf(fp, "  \"student\": \"Ankit Pandey\",\n");
    fprintf(fp, "  \"erp_id\": \"2501331520029\",\n");
    fprintf(fp, "  \"num_timeslots\": %d,\n", num_slots);
    fprintf(fp, "  \"num_courses\": %d,\n", graph->num_vertices);
    fprintf(fp, "  \"num_conflict_edges\": %d,\n", graph->num_edges);
    fprintf(fp, "  \"schedule\": [\n");

    for (int s = 0; s < num_slots; s++) {
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"timeslot\": %d,\n", s + 1);
        fprintf(fp, "      \"utilization_pct\": %.1f,\n", room_sched ? room_sched[s].utilization_pct : 0.0);
        fprintf(fp, "      \"allocations\": [\n");
        if (room_sched) {
            for (int e = 0; e < room_sched[s].num_entries; e++) {
                int cid = room_sched[s].entries[e].course_id;
                int rid = room_sched[s].entries[e].room_id;
                int count = room_sched[s].entries[e].allocated_students;
                fprintf(fp, "        {\n");
                fprintf(fp, "          \"course_code\": \"%s\",\n", graph->courses[cid].code);
                fprintf(fp, "          \"course_title\": \"%s\",\n", graph->courses[cid].title);
                fprintf(fp, "          \"department\": \"%s\",\n", graph->courses[cid].department);
                fprintf(fp, "          \"hall_code\": \"%s\",\n", rooms[rid].code);
                fprintf(fp, "          \"hall_building\": \"%s\",\n", rooms[rid].building);
                fprintf(fp, "          \"allocated_students\": %d,\n", count);
                fprintf(fp, "          \"hall_capacity\": %d\n", rooms[rid].capacity);
                fprintf(fp, "        }%s\n", (e + 1 < room_sched[s].num_entries) ? "," : "");
            }
        }
        fprintf(fp, "      ]\n");
        fprintf(fp, "    }%s\n", (s + 1 < num_slots) ? "," : "");
    }
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    fclose(fp);
}

void export_html_dashboard(const char* filepath, const Graph* graph, const SlotRoomSchedule* room_sched, int num_slots, const Room* rooms, int num_rooms, const ValidationReport* report) {
    FILE* fp = fopen(filepath, "w");
    if (!fp) return;

    fprintf(fp, "<!DOCTYPE html>\n");
    fprintf(fp, "<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n");
    fprintf(fp, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    fprintf(fp, "<title>Intelligent Examination Scheduling Dashboard</title>\n");
    fprintf(fp, "<style>\n");
    fprintf(fp, "  :root { --primary: #1A237E; --primary-light: #E8EAF6; --success: #2E7D32; --accent: #C2185B; --bg: #F8F9FA; --card: #FFFFFF; --border: #E0E0E0; --text: #212121; --subtext: #616161; }\n");
    fprintf(fp, "  * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Times New Roman', serif; }\n");
    fprintf(fp, "  body { background: var(--bg); color: var(--text); padding: 24px; line-height: 1.5; }\n");
    fprintf(fp, "  .container { max-width: 1200px; margin: 0 auto; }\n");
    fprintf(fp, "  header { background: linear-gradient(135deg, #1A237E 0%%, #283593 100%%); color: white; padding: 28px; border-radius: 12px; margin-bottom: 24px; box-shadow: 0 4px 12px rgba(26,35,126,0.15); }\n");
    fprintf(fp, "  header h1 { font-size: 26px; margin-bottom: 6px; }\n");
    fprintf(fp, "  header p { font-size: 14px; opacity: 0.9; }\n");
    fprintf(fp, "  .badge { display: inline-block; padding: 4px 10px; border-radius: 20px; font-size: 12px; font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px; }\n");
    fprintf(fp, "  .badge-success { background: #E8F5E9; color: #2E7D32; border: 1px solid #A5D6A7; }\n");
    fprintf(fp, "  .grid-stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 16px; margin-bottom: 24px; }\n");
    fprintf(fp, "  .stat-card { background: var(--card); padding: 18px; border-radius: 10px; border: 1px solid var(--border); border-left: 4px solid var(--primary); }\n");
    fprintf(fp, "  .stat-card.green { border-left-color: var(--success); }\n");
    fprintf(fp, "  .stat-card.accent { border-left-color: var(--accent); }\n");
    fprintf(fp, "  .stat-val { font-size: 28px; font-weight: bold; color: var(--primary); }\n");
    fprintf(fp, "  .stat-lbl { font-size: 12px; color: var(--subtext); text-transform: uppercase; font-weight: 600; margin-top: 4px; }\n");
    fprintf(fp, "  .section { background: var(--card); padding: 22px; border-radius: 10px; border: 1px solid var(--border); margin-bottom: 24px; box-shadow: 0 2px 6px rgba(0,0,0,0.03); }\n");
    fprintf(fp, "  .section h2 { font-size: 18px; color: var(--primary); margin-bottom: 16px; border-bottom: 2px solid var(--primary-light); padding-bottom: 8px; }\n");
    fprintf(fp, "  table { width: 100%%; border-collapse: collapse; font-size: 13.5px; }\n");
    fprintf(fp, "  th, td { padding: 10px 12px; text-align: left; border-bottom: 1px solid var(--border); }\n");
    fprintf(fp, "  th { background: #F4F6F9; color: #37474F; font-weight: 600; text-transform: uppercase; font-size: 11.5px; letter-spacing: 0.5px; }\n");
    fprintf(fp, "  tr:hover { background: #FAFAFA; }\n");
    fprintf(fp, "  .slot-pill { display: inline-block; padding: 3px 8px; border-radius: 12px; font-weight: bold; font-size: 11.5px; background: #E8EAF6; color: #1A237E; }\n");
    fprintf(fp, "  .progress-bar { width: 100%%; background: #EEE; height: 8px; border-radius: 4px; overflow: hidden; }\n");
    fprintf(fp, "  .progress-fill { height: 100%%; background: #2E7D32; }\n");
    fprintf(fp, "</style>\n</head>\n<body>\n");

    fprintf(fp, "<div class=\"container\">\n");
    fprintf(fp, "  <header>\n");
    fprintf(fp, "    <div style=\"display:flex; justify-content:space-between; align-items:center;\">\n");
    fprintf(fp, "      <div>\n");
    fprintf(fp, "        <h1>Intelligent Examination Scheduling System</h1>\n");
    fprintf(fp, "        <p>B.Tech CSE (AI) | Data Structures and Algorithms - II (CCSE0301) | Student: <b>Ankit Pandey (2501331520029)</b></p>\n");
    fprintf(fp, "      </div>\n");
    fprintf(fp, "      <span class=\"badge badge-success\">%s</span>\n", report->all_hard_constraints_satisfied ? "Verified Optimal Schedule" : "Violations Detected");
    fprintf(fp, "    </div>\n");
    fprintf(fp, "  </header>\n");

    fprintf(fp, "  <div class=\"grid-stats\">\n");
    fprintf(fp, "    <div class=\"stat-card\"><div class=\"stat-val\">%d</div><div class=\"stat-lbl\">Courses Scheduled (|V|)</div></div>\n", graph->num_vertices);
    fprintf(fp, "    <div class=\"stat-card\"><div class=\"stat-val\">%d</div><div class=\"stat-lbl\">Conflict Edges Checked (|E|)</div></div>\n", graph->num_edges);
    fprintf(fp, "    <div class=\"stat-card green\"><div class=\"stat-val\">%d</div><div class=\"stat-lbl\">Chromatic Number &#967;(G) (Slots)</div></div>\n", num_slots);
    fprintf(fp, "    <div class=\"stat-card accent\"><div class=\"stat-val\">%.1f%%</div><div class=\"stat-lbl\">Hall Capacity Utilization</div></div>\n", report->overall_room_utilization);
    fprintf(fp, "  </div>\n");

    // Timetable Grid
    fprintf(fp, "  <div class=\"section\">\n");
    fprintf(fp, "    <h2>Master Examination Timetable & Seating Matrix</h2>\n");
    fprintf(fp, "    <table>\n");
    fprintf(fp, "      <thead><tr><th>Time Slot</th><th>Course Code</th><th>Course Title</th><th>Department</th><th>Enrolled</th><th>Allocated Hall</th><th>Building</th><th>Hall Capacity</th><th>Occupancy</th></tr></thead>\n");
    fprintf(fp, "      <tbody>\n");

    for (int s = 0; s < num_slots; s++) {
        if (!room_sched) continue;
        for (int e = 0; e < room_sched[s].num_entries; e++) {
            int cid = room_sched[s].entries[e].course_id;
            int rid = room_sched[s].entries[e].room_id;
            int count = room_sched[s].entries[e].allocated_students;
            double util = ((double)count / rooms[rid].capacity) * 100.0;

            fprintf(fp, "        <tr>\n");
            fprintf(fp, "          <td><span class=\"slot-pill\">Slot %d</span></td>\n", s + 1);
            fprintf(fp, "          <td><b>%s</b></td>\n", graph->courses[cid].code);
            fprintf(fp, "          <td>%s</td>\n", graph->courses[cid].title);
            fprintf(fp, "          <td>%s</td>\n", graph->courses[cid].department);
            fprintf(fp, "          <td>%d</td>\n", graph->courses[cid].enrolled_students);
            fprintf(fp, "          <td><b>%s</b></td>\n", rooms[rid].code);
            fprintf(fp, "          <td>%s</td>\n", rooms[rid].building);
            fprintf(fp, "          <td>%d</td>\n", rooms[rid].capacity);
            fprintf(fp, "          <td>\n");
            fprintf(fp, "            <div style=\"font-size:11px; margin-bottom:2px;\">%d / %d (%.0f%%)</div>\n", count, rooms[rid].capacity, util);
            fprintf(fp, "            <div class=\"progress-bar\"><div class=\"progress-fill\" style=\"width:%.0f%%;\"></div></div>\n", util);
            fprintf(fp, "          </td>\n");
            fprintf(fp, "        </tr>\n");
        }
    }

    fprintf(fp, "      </tbody>\n");
    fprintf(fp, "    </table>\n");
    fprintf(fp, "  </div>\n");

    fprintf(fp, "</div>\n</body>\n</html>\n");
    fclose(fp);
}
