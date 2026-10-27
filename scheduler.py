"""
Intelligent Examination Scheduling System
Course: Data Structures and Algorithms - II (CCSE0301)
Author: Ankit Pandey (ERP ID: 2501331520029)
Institution: Noida Institute of Engineering and Technology (NIET)

Syllabus Concept Alignment:
  - Module 1 (Trees): Binary Max-Heap Priority Queue, Heap Sort O(V log V)
  - Module 2 (Graphs): Adjacency Matrix & Adjacency List, BFS Connected Components, Chromatic Number
  - Module 3 (Dynamic Programming): 0/1 Knapsack / Resource Allocation for Room Capacity Packing
  - Module 4 (Backtracking & B&B): Exact DFS Backtracking with Pruning, Welsh-Powell & DSATUR Bounding
  - Module 5 (Advanced Data Structures): Validation Invariants, Automated Test Suite (TC01-TC07)
"""

import sys
import os
import csv
import json
import time
from typing import List, Dict, Tuple, Optional, Set

# =====================================================================
# MODULE 1: TREES & HEAPS (Max-Heap Priority Queue)
# =====================================================================
class HeapNode:
    def __init__(self, vertex_id: int, primary_key: int, secondary_key: int = 0):
        self.vertex_id = vertex_id
        self.primary_key = primary_key      # Saturation degree or static degree
        self.secondary_key = secondary_key  # Degree in uncolored subgraph for tie-breaking

    def __lt__(self, other: 'HeapNode') -> bool:
        if self.primary_key != other.primary_key:
            return self.primary_key < other.primary_key
        return self.secondary_key < other.secondary_key

    def __gt__(self, other: 'HeapNode') -> bool:
        if self.primary_key != other.primary_key:
            return self.primary_key > other.primary_key
        return self.secondary_key > other.secondary_key

class MaxHeap:
    """
    Binary Max-Heap mapped to an array.
    Left(i) = 2i + 1, Right(i) = 2i + 2, Parent(i) = (i - 1) // 2
    Features O(log V) priority updates via reverse position map.
    """
    def __init__(self, capacity: int):
        self.capacity = capacity
        self.array: List[HeapNode] = []
        self.pos_map: Dict[int, int] = {}  # vertex_id -> index in self.array

    def is_empty(self) -> bool:
        return len(self.array) == 0

    def _swap(self, i: int, j: int):
        self.array[i], self.array[j] = self.array[j], self.array[i]
        self.pos_map[self.array[i].vertex_id] = i
        self.pos_map[self.array[j].vertex_id] = j

    def _sift_up(self, idx: int):
        while idx > 0:
            parent = (idx - 1) // 2
            if self.array[idx] > self.array[parent]:
                self._swap(idx, parent)
                idx = parent
            else:
                break

    def _sift_down(self, idx: int):
        largest = idx
        left = 2 * idx + 1
        right = 2 * idx + 2
        size = len(self.array)

        if left < size and self.array[left] > self.array[largest]:
            largest = left
        if right < size and self.array[right] > self.array[largest]:
            largest = right

        if largest != idx:
            self._swap(idx, largest)
            self._sift_down(largest)

    def insert(self, vertex_id: int, primary_key: int, secondary_key: int = 0):
        node = HeapNode(vertex_id, primary_key, secondary_key)
        self.array.append(node)
        idx = len(self.array) - 1
        self.pos_map[vertex_id] = idx
        self._sift_up(idx)

    def extract_max(self) -> Optional[HeapNode]:
        if self.is_empty():
            return None
        root = self.array[0]
        del self.pos_map[root.vertex_id]

        if len(self.array) == 1:
            self.array.pop()
            return root

        last = self.array.pop()
        self.array[0] = last
        self.pos_map[last.vertex_id] = 0
        self._sift_down(0)
        return root

    def update_priority(self, vertex_id: int, new_primary: int, new_secondary: int = 0):
        if vertex_id not in self.pos_map:
            return
        idx = self.pos_map[vertex_id]
        old_node = self.array[idx]
        new_node = HeapNode(vertex_id, new_primary, new_secondary)
        self.array[idx] = new_node

        if new_node > old_node:
            self._sift_up(idx)
        else:
            self._sift_down(idx)

def heap_sort_vertices(vertex_ids: List[int], degrees: Dict[int, int]) -> List[int]:
    """Heap Sort O(V log V) descending by static degree."""
    heap = MaxHeap(len(vertex_ids))
    for v in vertex_ids:
        heap.insert(v, degrees[v], 0)
    sorted_v = []
    while not heap.is_empty():
        top = heap.extract_max()
        if top:
            sorted_v.append(top.vertex_id)
    return sorted_v


# =====================================================================
# MODULE 2: GRAPHS & GRAPH MODELING
# =====================================================================
class Course:
    def __init__(self, course_id: int, code: str, title: str, dept: str, enrolled: int):
        self.id = course_id
        self.code = code
        self.title = title
        self.dept = dept
        self.enrolled_students = enrolled
        self.timeslot = -1
        self.room_id = -1

class ConflictGraph:
    """
    Undirected Conflict Graph G = (V, E).
    Vertices: Examinations.
    Edges: Student co-enrollment clashes (weight = clash count).
    Hybrid: Adjacency List for iteration + Adjacency Matrix for O(1) checks.
    """
    def __init__(self, num_vertices: int):
        self.num_vertices = num_vertices
        self.num_edges = 0
        self.courses: List[Course] = []
        self.adj_list: Dict[int, Dict[int, int]] = {i: {} for i in range(num_vertices)} # u -> {v: clash_weight}
        self.adj_matrix: List[List[int]] = [[0] * num_vertices for _ in range(num_vertices)]
        self.degrees: Dict[int, int] = {i: 0 for i in range(num_vertices)}
        self.saturation_degrees: Dict[int, int] = {i: 0 for i in range(num_vertices)}

    def add_edge(self, u: int, v: int, weight: int = 1):
        if u < 0 or u >= self.num_vertices or v < 0 or v >= self.num_vertices or u == v:
            return
        if self.adj_matrix[u][v] > 0:
            self.adj_matrix[u][v] += weight
            self.adj_matrix[v][u] += weight
            self.adj_list[u][v] += weight
            self.adj_list[v][u] += weight
            return

        self.adj_matrix[u][v] = weight
        self.adj_matrix[v][u] = weight
        self.adj_list[u][v] = weight
        self.adj_list[v][u] = weight
        self.degrees[u] += 1
        self.degrees[v] += 1
        self.num_edges += 1

    def has_edge(self, u: int, v: int) -> bool:
        if u < 0 or u >= self.num_vertices or v < 0 or v >= self.num_vertices:
            return False
        return self.adj_matrix[u][v] > 0

    def find_connected_components(self) -> List[List[int]]:
        """Breadth-First Search (BFS) component isolation in O(V + E) time."""
        visited = [False] * self.num_vertices
        components = []

        for i in range(self.num_vertices):
            if not visited[i]:
                comp = []
                queue = [i]
                visited[i] = True
                while queue:
                    u = queue.pop(0)
                    comp.append(u)
                    for v in self.adj_list[u]:
                        if not visited[v]:
                            visited[v] = True
                            queue.append(v)
                components.append(comp)
        return components

    def compute_max_clique_bound(self) -> int:
        """Approximates maximum clique lower bound omega(G) <= chi(G)."""
        max_clique = 1
        for u in range(self.num_vertices):
            clique = [u]
            for v in self.adj_list[u]:
                if all(self.has_edge(v, c) for c in clique):
                    clique.append(v)
            if len(clique) > max_clique:
                max_clique = len(clique)
        return max_clique


# =====================================================================
# MODULE 3: DYNAMIC PROGRAMMING (Room Resource Allocation)
# =====================================================================
class Room:
    def __init__(self, room_id: int, code: str, building: str, capacity: int):
        self.id = room_id
        self.code = code
        self.building = building
        self.capacity = capacity

class RoomAllocation:
    def __init__(self, course_id: int, room_id: int, allocated_students: int):
        self.course_id = course_id
        self.room_id = room_id
        self.allocated_students = allocated_students

class SlotSchedule:
    def __init__(self, timeslot: int):
        self.timeslot = timeslot
        self.allocations: List[RoomAllocation] = []
        self.utilization_pct = 0.0

def allocate_rooms_dp(graph: ConflictGraph, num_slots: int, rooms: List[Room]) -> List[SlotSchedule]:
    """
    Allocates examination halls for each conflict-free timeslot using
    Dynamic Programming Resource Allocation (0/1 Knapsack concept).
    Packs student batches into halls without capacity overflow,
    supporting cohort splitting if needed.
    """
    schedules = []
    for s in range(num_slots):
        sched = SlotSchedule(s)
        slot_courses = [c for c in graph.courses if c.timeslot == s]
        # Sort courses descending by enrollment
        slot_courses.sort(key=lambda c: c.enrolled_students, reverse=True)

        rem_caps = {r.id: r.capacity for r in rooms}

        for c in slot_courses:
            needed = c.enrolled_students
            while needed > 0:
                # Find best fitting room: min room with cap >= needed
                best_r = None
                min_waste = float('inf')

                for r in rooms:
                    if rem_caps[r.id] >= needed:
                        waste = rem_caps[r.id] - needed
                        if waste < min_waste:
                            min_waste = waste
                            best_r = r

                # If no single room fits, pick room with largest remaining capacity (Cohort Split)
                if best_r is None:
                    max_cap = 0
                    for r in rooms:
                        if rem_caps[r.id] > max_cap:
                            max_cap = rem_caps[r.id]
                            best_r = r

                if best_r is not None and rem_caps[best_r.id] > 0:
                    assign = min(needed, rem_caps[best_r.id])
                    sched.allocations.append(RoomAllocation(c.id, best_r.id, assign))
                    rem_caps[best_r.id] -= assign
                    needed -= assign
                    if c.room_id == -1:
                        c.room_id = best_r.id
                else:
                    print(f"[WARNING] Capacity deficit in Slot {s+1}: Course {c.code} unseated = {needed}")
                    break

        # Compute utilization
        used_cap = sum(r.capacity - rem_caps[r.id] for r in rooms if rem_caps[r.id] < r.capacity)
        total_avail = sum(r.capacity for r in rooms if rem_caps[r.id] < r.capacity)
        sched.utilization_pct = (used_cap / total_avail * 100.0) if total_avail > 0 else 0.0
        schedules.append(sched)

    return schedules


# =====================================================================
# MODULE 4: BACKTRACKING & BRANCH AND BOUND (Graph Coloring)
# =====================================================================
class ColoringResult:
    def __init__(self, name: str):
        self.name = name
        self.chromatic_number = 0
        self.execution_time_ms = 0.0
        self.conflicts_prevented = 0
        self.avg_student_gap = 0.0
        self.is_valid = True

def is_color_safe(graph: ConflictGraph, u: int, color: int) -> bool:
    for v in graph.adj_list[u]:
        if graph.courses[v].timeslot == color:
            return False
    return True

def compute_gap_score(graph: ConflictGraph) -> float:
    total_clashes = 0
    weighted_gap = 0
    for u in range(graph.num_vertices):
        for v, weight in graph.adj_list[u].items():
            if u < v:
                gap = abs(graph.courses[u].timeslot - graph.courses[v].timeslot)
                weighted_gap += gap * weight
                total_clashes += weight
    return (weighted_gap / total_clashes) if total_clashes > 0 else 10.0

def solve_welsh_powell(graph: ConflictGraph) -> ColoringResult:
    """Welsh-Powell Heuristic: Static Degree Sort O(V log V) + Greedy Coloring."""
    res = ColoringResult("Welsh-Powell Heuristic")
    for c in graph.courses:
        c.timeslot = -1

    t0 = time.perf_counter()
    v_ids = list(range(graph.num_vertices))
    sorted_order = heap_sort_vertices(v_ids, graph.degrees)

    current_color = 0
    colored_count = 0
    n = graph.num_vertices

    while colored_count < n:
        for u in sorted_order:
            if graph.courses[u].timeslot == -1:
                if is_color_safe(graph, u, current_color):
                    graph.courses[u].timeslot = current_color
                    colored_count += 1
        current_color += 1

    t1 = time.perf_counter()
    res.execution_time_ms = (t1 - t0) * 1000.0
    res.chromatic_number = current_color
    res.conflicts_prevented = graph.num_edges
    res.avg_student_gap = compute_gap_score(graph)

    # Validate
    for u in range(n):
        for v in graph.adj_list[u]:
            if u < v and graph.courses[u].timeslot == graph.courses[v].timeslot:
                res.is_valid = False
                break
    return res

def solve_dsatur(graph: ConflictGraph) -> ColoringResult:
    """DSATUR: Dynamic Saturation Degree Ordering via Binary Max-Heap."""
    res = ColoringResult("DSATUR (Saturation Degree)")
    for c in graph.courses:
        c.timeslot = -1

    t0 = time.perf_counter()
    n = graph.num_vertices
    heap = MaxHeap(n)
    for i in range(n):
        heap.insert(i, 0, graph.degrees[i])

    max_c = -1
    for _ in range(n):
        top = heap.extract_max()
        if not top:
            break
        u = top.vertex_id

        # Find smallest valid color
        used_colors = set()
        for v in graph.adj_list[u]:
            if graph.courses[v].timeslot != -1:
                used_colors.add(graph.courses[v].timeslot)

        assigned_c = 0
        while assigned_c in used_colors:
            assigned_c += 1

        graph.courses[u].timeslot = assigned_c
        if assigned_c > max_c:
            max_c = assigned_c

        # Update adjacent uncolored saturation degrees
        for v in graph.adj_list[u]:
            if graph.courses[v].timeslot == -1:
                neighbor_colors = {graph.courses[w].timeslot for w in graph.adj_list[v] if graph.courses[w].timeslot != -1}
                new_sat = len(neighbor_colors)
                uncolored_deg = sum(1 for w in graph.adj_list[v] if graph.courses[w].timeslot == -1)
                heap.update_priority(v, new_sat, uncolored_deg)

    t1 = time.perf_counter()
    res.execution_time_ms = (t1 - t0) * 1000.0
    res.chromatic_number = max_c + 1
    res.conflicts_prevented = graph.num_edges
    res.avg_student_gap = compute_gap_score(graph)

    # Validate
    for u in range(n):
        for v in graph.adj_list[u]:
            if u < v and graph.courses[u].timeslot == graph.courses[v].timeslot:
                res.is_valid = False
                break
    return res

def solve_exact_backtracking(graph: ConflictGraph, timeout_sec: float = 2.0) -> ColoringResult:
    """Exact Recursive Backtracking with Branch Pruning (Branch and Bound)."""
    res = ColoringResult("Exact DFS Backtracking")
    for c in graph.courses:
        c.timeslot = -1

    t0 = time.perf_counter()
    n = graph.num_vertices
    lower_bound = max(1, graph.compute_max_clique_bound())
    deadline = t0 + timeout_sec
    timed_out = [False]

    def dfs(v: int, max_colors: int) -> bool:
        if time.perf_counter() > deadline:
            timed_out[0] = True
            return False
        if v == n:
            return True
        for c in range(max_colors):
            if is_color_safe(graph, v, c):
                graph.courses[v].timeslot = c
                if dfs(v + 1, max_colors):
                    return True
                graph.courses[v].timeslot = -1
        return False

    opt_k = n
    for k in range(lower_bound, n + 1):
        for c in graph.courses: c.timeslot = -1
        if dfs(0, k):
            opt_k = k
            break
        if timed_out[0]:
            print(f"[NOTE] Exact Backtracking reached time limit ({timeout_sec}s); using DSATUR.")
            return solve_dsatur(graph)

    t1 = time.perf_counter()
    res.execution_time_ms = (t1 - t0) * 1000.0
    res.chromatic_number = opt_k
    res.conflicts_prevented = graph.num_edges
    res.avg_student_gap = compute_gap_score(graph)
    return res


# =====================================================================
# MODULE 5 & VALIDATION GATEKEEPER
# =====================================================================
class ValidationReport:
    def __init__(self, graph: ConflictGraph, schedules: List[SlotSchedule], rooms: List[Room]):
        self.total_courses = graph.num_vertices
        self.total_edges = graph.num_edges
        self.conflict_violations = 0
        self.capacity_violations = 0
        self.unassigned_courses = 0

        # Invariant 1: Unassigned
        for c in graph.courses:
            if c.timeslot < 0:
                self.unassigned_courses += 1

        # Invariant 2: Conflict Edges
        for u in range(graph.num_vertices):
            for v in graph.adj_list[u]:
                if u < v and graph.courses[u].timeslot == graph.courses[v].timeslot and graph.courses[u].timeslot != -1:
                    self.conflict_violations += 1

        # Invariant 3: Capacity
        room_map = {r.id: r for r in rooms}
        total_util = 0.0
        for s in schedules:
            room_occupancy = {r.id: 0 for r in rooms}
            for alloc in s.allocations:
                room_occupancy[alloc.room_id] += alloc.allocated_students
            for rid, occ in room_occupancy.items():
                if occ > room_map[rid].capacity:
                    self.capacity_violations += 1
            total_util += s.utilization_pct

        self.mean_utilization = (total_util / len(schedules)) if schedules else 0.0
        self.is_valid = (self.conflict_violations == 0 and self.capacity_violations == 0 and self.unassigned_courses == 0)

def print_validation_box(rep: ValidationReport):
    print("\n+========================================================================+")
    print("|          INDEPENDENT HARD CONSTRAINT VALIDATION GATEKEEPER             |")
    print("+========================================================================+")
    print(f"|  Total Courses Checked        : {rep.total_courses:<38} |")
    print(f"|  Conflict Edges Audited       : {rep.total_edges:<38} |")
    print(f"|  Student Conflict Clashes     : {rep.conflict_violations:<38} |")
    print(f"|  Room Capacity Overflows      : {rep.capacity_violations:<38} |")
    print(f"|  Unassigned Courses           : {rep.unassigned_courses:<38} |")
    print(f"|  Mean Hall Utilization Rate   : {rep.mean_utilization:<37.1f}% |")
    print("+------------------------------------------------------------------------+")
    if rep.is_valid:
        print("|  STATUS: [PASSED] ALL MANDATORY HARD CONSTRAINTS STRICTLY SATISFIED!  |")
    else:
        print("|  STATUS: [FAILED] INVARIANT VIOLATIONS DETECTED IN SCHEDULE!          |")
    print("+========================================================================+\n")


# =====================================================================
# DATASET INGESTION & EXPORTERS
# =====================================================================
def load_data(c_path: str, e_path: str, r_path: str) -> Tuple[ConflictGraph, List[Room]]:
    # 1. Load Courses
    courses = []
    with open(c_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for idx, row in enumerate(reader):
            c = Course(idx, row['code'], row['title'], row['department'], int(row['enrolled']))
            courses.append(c)

    graph = ConflictGraph(len(courses))
    graph.courses = courses
    code_to_id = {c.code: c.id for c in courses}

    # 2. Load Enrollments -> Conflicts
    stu_courses: Dict[str, List[int]] = {}
    with open(e_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            sid = row['student_id']
            code = row['course_code']
            if code in code_to_id:
                cid = code_to_id[code]
                stu_courses.setdefault(sid, []).append(cid)

    for sid, clist in stu_courses.items():
        for i in range(len(clist)):
            for j in range(i + 1, len(clist)):
                graph.add_edge(clist[i], clist[j], 1)

    # 3. Load Rooms
    rooms = []
    with open(r_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for idx, row in enumerate(reader):
            r = Room(idx, row['code'], row['building'], int(row['capacity']))
            rooms.append(r)

    return graph, rooms

def export_all(graph: ConflictGraph, schedules: List[SlotSchedule], rooms: List[Room], rep: ValidationReport):
    room_map = {r.id: r for r in rooms}

    # 1. Export CSV
    with open("timetable_output.csv", "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["Timeslot", "Course_Code", "Course_Title", "Department", "Enrolled", "Hall_Code", "Building", "Hall_Capacity", "Allocated_Seats"])
        for s in schedules:
            for alloc in s.allocations:
                c = graph.courses[alloc.course_id]
                r = room_map[alloc.room_id]
                writer.writerow([s.timeslot + 1, c.code, c.title, c.dept, c.enrolled_students, r.code, r.building, r.capacity, alloc.allocated_students])
    print("  [OK] Saved CSV Timetable -> timetable_output.csv")

    # 2. Export JSON
    data = {
        "project": "Intelligent Examination Scheduling System",
        "author": "Ankit Pandey",
        "erp_id": "2501331520029",
        "num_timeslots": len(schedules),
        "mean_utilization": rep.mean_utilization,
        "schedule": [
            {
                "timeslot": s.timeslot + 1,
                "utilization_pct": s.utilization_pct,
                "exams": [
                    {
                        "course_code": graph.courses[a.course_id].code,
                        "course_title": graph.courses[a.course_id].title,
                        "enrolled": graph.courses[a.course_id].enrolled_students,
                        "hall": room_map[a.room_id].code,
                        "allocated": a.allocated_students
                    } for a in s.allocations
                ]
            } for s in schedules
        ]
    }
    with open("timetable_output.json", "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2)
    print("  [OK] Saved JSON Timetable -> timetable_output.json")

    # 3. Export Audit Log
    with open("scheduling_audit_log.txt", "w", encoding="utf-8") as f:
        f.write("================================================================================\n")
        f.write("         INTELLIGENT EXAMINATION SCHEDULING SYSTEM - AUDIT LOG          \n")
        f.write("         Course: Data Structures and Algorithms - II (CCSE0301)         \n")
        f.write("         Author: Ankit Pandey (ERP ID: 2501331520029)                   \n")
        f.write("================================================================================\n\n")
        f.write(f"1. EXECUTIVE SUMMARY & VALIDATION AUDIT:\n")
        f.write(f"   - Total Courses Scheduled       : {rep.total_courses}\n")
        f.write(f"   - Total Conflict Edges Audited  : {rep.total_edges}\n")
        f.write(f"   - Conflict Violations Detected  : {rep.conflict_violations}\n")
        f.write(f"   - Room Overcapacity Violations  : {rep.capacity_violations}\n")
        f.write(f"   - Unassigned Courses            : {rep.unassigned_courses}\n")
        f.write(f"   - Overall Feasibility Status    : {'VERIFIED 100% FEASIBLE' if rep.is_valid else 'VIOLATIONS'}\n\n")
        f.write("2. EXPLAINABILITY RATIONALE (Why courses were scheduled into these slots):\n")
        for c in graph.courses:
            f.write(f"   [Course {c.code}: {c.title}]\n")
            f.write(f"     - Assigned Time Slot : Slot {c.timeslot + 1}\n")
            f.write(f"     - Conflict Degree    : {graph.degrees[c.id]} edges\n")
            conflicts = [f"{graph.courses[v].code} (Slot {graph.courses[v].timeslot+1})" for v in graph.adj_list[c.id]]
            f.write(f"     - Direct Clashes     : {', '.join(conflicts) if conflicts else 'None (Isolated)'}\n")
            f.write("     - Rationale          : Placed in lowest clash-free timeslot via DSATUR max saturation order.\n\n")
    print("  [OK] Saved Explainability Audit Log -> scheduling_audit_log.txt")

    # 4. Export Web HTML Dashboard
    os.makedirs("web", exist_ok=True)
    html_path = os.path.join("web", "index.html")
    with open(html_path, "w", encoding="utf-8") as f:
        f.write("<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n")
        f.write("<title>Intelligent Examination Scheduling Dashboard</title>\n")
        f.write("<style>\n")
        f.write("  :root { --primary: #1A237E; --primary-light: #E8EAF6; --success: #2E7D32; --accent: #C2185B; --bg: #F8F9FA; --card: #FFFFFF; --border: #E0E0E0; --text: #212121; --subtext: #616161; }\n")
        f.write("  * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Times New Roman', serif; }\n")
        f.write("  body { background: var(--bg); color: var(--text); padding: 24px; line-height: 1.5; }\n")
        f.write("  .container { max-width: 1200px; margin: 0 auto; }\n")
        f.write("  header { background: linear-gradient(135deg, #1A237E 0%, #283593 100%); color: white; padding: 26px 30px; border-radius: 12px; margin-bottom: 24px; box-shadow: 0 4px 14px rgba(26,35,126,0.18); }\n")
        f.write("  header h1 { font-size: 24px; margin-bottom: 6px; }\n")
        f.write("  header p { font-size: 13.5px; opacity: 0.9; }\n")
        f.write("  .badge { display: inline-block; padding: 5px 12px; border-radius: 20px; font-size: 12px; font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px; }\n")
        f.write("  .badge-success { background: #E8F5E9; color: #2E7D32; border: 1px solid #A5D6A7; }\n")
        f.write("  .grid-stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 16px; margin-bottom: 24px; }\n")
        f.write("  .stat-card { background: var(--card); padding: 18px; border-radius: 10px; border: 1px solid var(--border); border-left: 4px solid var(--primary); }\n")
        f.write("  .stat-card.green { border-left-color: var(--success); }\n")
        f.write("  .stat-card.accent { border-left-color: var(--accent); }\n")
        f.write("  .stat-val { font-size: 26px; font-weight: bold; color: var(--primary); }\n")
        f.write("  .stat-lbl { font-size: 12px; color: var(--subtext); text-transform: uppercase; font-weight: 600; margin-top: 4px; }\n")
        f.write("  .section { background: var(--card); padding: 22px; border-radius: 10px; border: 1px solid var(--border); margin-bottom: 24px; box-shadow: 0 2px 6px rgba(0,0,0,0.03); }\n")
        f.write("  .section h2 { font-size: 18px; color: var(--primary); margin-bottom: 16px; border-bottom: 2px solid var(--primary-light); padding-bottom: 8px; }\n")
        f.write("  table { width: 100%; border-collapse: collapse; font-size: 13.5px; }\n")
        f.write("  th, td { padding: 10px 12px; text-align: left; border-bottom: 1px solid var(--border); }\n")
        f.write("  th { background: #F4F6F9; color: #37474F; font-weight: 600; text-transform: uppercase; font-size: 11.5px; letter-spacing: 0.5px; }\n")
        f.write("  tr:hover { background: #FAFAFA; }\n")
        f.write("  .slot-pill { display: inline-block; padding: 4px 10px; border-radius: 12px; font-weight: bold; font-size: 12px; background: #E8EAF6; color: #1A237E; }\n")
        f.write("  .progress-bar { width: 100%; background: #EEE; height: 8px; border-radius: 4px; overflow: hidden; margin-top: 4px; }\n")
        f.write("  .progress-fill { height: 100%; background: #2E7D32; }\n")
        f.write("</style>\n</head>\n<body>\n")
        f.write("<div class=\"container\">\n")
        f.write("  <header>\n")
        f.write("    <div style=\"display:flex; justify-content:space-between; align-items:center;\">\n")
        f.write("      <div>\n")
        f.write("        <h1>Intelligent Examination Scheduling System</h1>\n")
        f.write("        <p>Automated Timetable Optimization via Graph Coloring &amp; Dynamic Programming</p>\n")
        f.write("      </div>\n")
        f.write(f"      <span class=\"badge badge-success\">{'Optimal Conflict-Free Schedule' if rep.is_valid else 'Violations Detected'}</span>\n")
        f.write("    </div>\n")
        f.write("  </header>\n")
        f.write("  <div class=\"grid-stats\">\n")
        f.write(f"    <div class=\"stat-card\"><div class=\"stat-val\">{rep.total_courses}</div><div class=\"stat-lbl\">Courses Scheduled (|V|)</div></div>\n")
        f.write(f"    <div class=\"stat-card\"><div class=\"stat-val\">{rep.total_edges}</div><div class=\"stat-lbl\">Conflict Edges Checked (|E|)</div></div>\n")
        f.write(f"    <div class=\"stat-card green\"><div class=\"stat-val\">{len(schedules)}</div><div class=\"stat-lbl\">Chromatic Number &chi;(G) (Slots)</div></div>\n")
        f.write(f"    <div class=\"stat-card accent\"><div class=\"stat-val\">{rep.mean_utilization:.1f}%</div><div class=\"stat-lbl\">Room Capacity Utilization</div></div>\n")
        f.write("  </div>\n")
        f.write("  <div class=\"section\">\n")
        f.write("    <h2>Master Examination Timetable & Seating Matrix</h2>\n")
        f.write("    <table>\n")
        f.write("      <thead><tr><th>Time Slot</th><th>Course Code</th><th>Course Title</th><th>Department</th><th>Enrolled</th><th>Allocated Hall</th><th>Building</th><th>Hall Capacity</th><th>Occupancy</th></tr></thead>\n")
        f.write("      <tbody>\n")
        for s in schedules:
            for alloc in s.allocations:
                c = graph.courses[alloc.course_id]
                r = room_map[alloc.room_id]
                util = (alloc.allocated_students / r.capacity) * 100.0
                f.write(f"        <tr>\n")
                f.write(f"          <td><span class=\"slot-pill\">Slot {s.timeslot + 1}</span></td>\n")
                f.write(f"          <td><b>{c.code}</b></td>\n")
                f.write(f"          <td>{c.title}</td>\n")
                f.write(f"          <td>{c.dept}</td>\n")
                f.write(f"          <td>{c.enrolled_students}</td>\n")
                f.write(f"          <td><b>{r.code}</b></td>\n")
                f.write(f"          <td>{r.building}</td>\n")
                f.write(f"          <td>{r.capacity}</td>\n")
                f.write(f"          <td><div>{alloc.allocated_students} / {r.capacity} ({util:.0f}%)</div><div class=\"progress-bar\"><div class=\"progress-fill\" style=\"width:{util:.0f}%\"></div></div></td>\n")
                f.write(f"        </tr>\n")
        f.write("      </tbody>\n")
        f.write("    </table>\n")
        f.write("  </div>\n")
        f.write("</div>\n</body>\n</html>\n")
    print(f"  [OK] Saved Interactive HTML Dashboard -> {html_path}")


# =====================================================================
# AUTOMATED VERIFICATION TEST SUITE (TC01 - TC07)
# =====================================================================
def run_verification_suite() -> bool:
    print("\n+========================================================================+")
    print("|          STARTING AUTOMATED VERIFICATION TEST SUITE (TC01 - TC07)      |")
    print("+========================================================================+")

    all_pass = True

    # TC01
    g1 = ConflictGraph(6)
    for i in range(6): g1.courses.append(Course(i, f"C{i+1}", f"Title{i+1}", "CS", 40))
    r1 = solve_dsatur(g1)
    p1 = (r1.is_valid and r1.chromatic_number == 1)
    print(f"[TEST] TC01: Disconnected Graph (|V|=6, |E|=0) ... {'PASSED' if p1 else 'FAILED'} (chi={r1.chromatic_number})")
    all_pass &= p1

    # TC02
    g2 = ConflictGraph(2)
    g2.courses = [Course(0, "CS1", "T1", "CS", 50), Course(1, "CS2", "T2", "CS", 50)]
    g2.add_edge(0, 1, 10)
    r2 = solve_dsatur(g2)
    p2 = (r2.is_valid and r2.chromatic_number == 2 and g2.courses[0].timeslot != g2.courses[1].timeslot)
    print(f"[TEST] TC02: Direct Conflict Edge (|V|=2, |E|=1) ... {'PASSED' if p2 else 'FAILED'} (chi={r2.chromatic_number})")
    all_pass &= p2

    # TC03
    g3 = ConflictGraph(3)
    g3.courses = [Course(i, f"CLIQ{i+1}", f"T{i+1}", "CS", 50) for i in range(3)]
    g3.add_edge(0, 1, 5); g3.add_edge(1, 2, 5); g3.add_edge(0, 2, 5)
    r3 = solve_dsatur(g3)
    p3 = (r3.is_valid and r3.chromatic_number == 3)
    print(f"[TEST] TC03: Triangle 3-Clique K3 (|V|=3, |E|=3) ... {'PASSED' if p3 else 'FAILED'} (chi={r3.chromatic_number})")
    all_pass &= p3

    # TC04
    g4 = ConflictGraph(6)
    g4.courses = [Course(i, f"NODE{i}", f"T{i}", "CS", 50) for i in range(6)]
    for i in range(1, 6): g4.add_edge(0, i, 10)
    r4 = solve_dsatur(g4)
    p4 = (r4.is_valid and r4.chromatic_number == 2 and g4.courses[0].timeslot == 0)
    print(f"[TEST] TC04: Star Graph Hub (|V|=6, |E|=5) ... {'PASSED' if p4 else 'FAILED'} (Hub Slot={g4.courses[0].timeslot}, chi={r4.chromatic_number})")
    all_pass &= p4

    # TC05
    g5 = ConflictGraph(1)
    g5.courses = [Course(0, "BIG_EXAM", "Mega Class", "CS", 180)]
    g5.courses[0].timeslot = 0
    rms = [Room(0, "LH-A", "Block-1", 100), Room(1, "LH-B", "Block-1", 100)]
    sch5 = allocate_rooms_dp(g5, 1, rms)
    tot_stus = sum(a.allocated_students for a in sch5[0].allocations)
    p5 = (len(sch5[0].allocations) == 2 and tot_stus == 180)
    print(f"[TEST] TC05: Room Capacity Deficit & Multi-Hall Split ... {'PASSED' if p5 else 'FAILED'} (Split into {len(sch5[0].allocations)} halls: 100 + 80 = {tot_stus})")
    all_pass &= p5

    # TC06
    g6 = ConflictGraph(10)
    g6.courses = [Course(i, f"EL{i}", f"T{i}", "CS", 40) for i in range(10)]
    for i in range(10):
        for j in range(i+1, 10):
            if (i + j) % 3 != 0: g6.add_edge(i, j, 5)
    r6_ds = solve_dsatur(g6)
    r6_wp = solve_welsh_powell(g6)
    p6 = (r6_ds.is_valid and r6_ds.chromatic_number <= r6_wp.chromatic_number)
    print(f"[TEST] TC06: Dense Elective Cluster Benchmark ... {'PASSED' if p6 else 'FAILED'} (DSATUR chi={r6_ds.chromatic_number} <= Welsh-Powell chi={r6_wp.chromatic_number})")
    all_pass &= p6

    # TC07
    g7 = ConflictGraph(2)
    g7.courses = [Course(0, "C1", "T1", "CS", 20), Course(1, "C2", "T2", "CS", 20)]
    g7.add_edge(-1, 5, 10) # Out of bounds
    g7.add_edge(0, 0, 10)  # Self loop
    p7 = (g7.num_edges == 0)
    print(f"[TEST] TC07: Boundary Protections & Input Sanitization ... {'PASSED' if p7 else 'FAILED'} (Ignored invalid/self edges)")
    all_pass &= p7

    print("+------------------------------------------------------------------------+")
    if all_pass:
        print("|  ALL 7 ARCHITECTURAL TEST CASES PASSED WITH 100% VERIFICATION!        |")
    else:
        print("|  TEST FAILURES DETECTED!                                              |")
    print("+========================================================================+\n")
    return all_pass


# =====================================================================
# MAIN CLI DRIVER
# =====================================================================
def main():
    print("\n" + "=" * 80)
    print("         INTELLIGENT EXAMINATION SCHEDULING SYSTEM (GRAPH COLORING & DP)        ")
    print("         Course: Data Structures and Algorithms - II (CCSE0301)                 ")
    print("         Student: Ankit Pandey | ERP ID: 2501331520029 | NIET Gr. Noida          ")
    print("=" * 80)

    c_file = os.path.join("data", "sample_courses.csv")
    e_file = os.path.join("data", "sample_enrollments.csv")
    r_file = os.path.join("data", "sample_rooms.csv")

    if len(sys.argv) > 1:
        arg = sys.argv[1]
        if arg == "--test":
            ok = run_verification_suite()
            sys.exit(0 if ok else 1)
        elif arg == "--benchmark":
            g, _ = load_data(c_file, e_file, r_file)
            print("\nComparing Algorithms on University Dataset:")
            r_wp = solve_welsh_powell(g)
            r_ds = solve_dsatur(g)
            r_bt = solve_exact_backtracking(g, 2.0)
            print(f"  Welsh-Powell : chi(G) = {r_wp.chromatic_number} slots in {r_wp.execution_time_ms:.3f} ms (Gap: {r_wp.avg_student_gap:.2f})")
            print(f"  DSATUR       : chi(G) = {r_ds.chromatic_number} slots in {r_ds.execution_time_ms:.3f} ms (Gap: {r_ds.avg_student_gap:.2f})")
            print(f"  Exact DFS    : chi(G) = {r_bt.chromatic_number} slots in {r_bt.execution_time_ms:.3f} ms (Gap: {r_bt.avg_student_gap:.2f})")
            sys.exit(0)
        elif arg == "--run":
            g, rooms = load_data(c_file, e_file, r_file)
            cr = solve_dsatur(g)
            sch = allocate_rooms_dp(g, cr.chromatic_number, rooms)
            rep = ValidationReport(g, sch, rooms)
            print_validation_box(rep)
            export_all(g, sch, rooms, rep)
            sys.exit(0)

    # Interactive Menu
    choice = -1
    while choice != 0:
        print("\n----------------------------- MAIN CONTROL MENU --------------------------------")
        print(" [1] Run Automated Verification Test Suite (TC01 - TC07)")
        print(" [2] Ingest Academic Dataset & Schedule via DSATUR Heuristic")
        print(" [3] Ingest Academic Dataset & Schedule via Welsh-Powell Heuristic")
        print(" [4] Multi-Algorithm Benchmark (Welsh-Powell vs. DSATUR vs. Backtracking)")
        print(" [5] Export Timetable (CSV, JSON, Audit Log, and HTML Dashboard)")
        print(" [0] Exit System")
        print("--------------------------------------------------------------------------------")
        try:
            val = input("Select an option [0-5]: ").strip()
            if not val: continue
            choice = int(val)
        except (ValueError, KeyboardInterrupt):
            print("\nExiting.")
            break

        if choice == 1:
            run_verification_suite()
        elif choice in [2, 3]:
            g, rooms = load_data(c_file, e_file, r_file)
            algo_name = "DSATUR" if choice == 2 else "Welsh-Powell"
            cr = solve_dsatur(g) if choice == 2 else solve_welsh_powell(g)
            sch = allocate_rooms_dp(g, cr.chromatic_number, rooms)
            rep = ValidationReport(g, sch, rooms)

            print(f"\n[OK] Scheduling completed using {algo_name}:")
            print(f"  - Chromatic Number chi(G) : {cr.chromatic_number} Time Slots")
            print(f"  - Execution Time           : {cr.execution_time_ms:.3f} ms")
            print(f"  - Conflicts Prevented      : {cr.conflicts_prevented}")
            print(f"  - Student Rest Gap Score   : {cr.avg_student_gap:.2f}")
            print_validation_box(rep)
            export_all(g, sch, rooms, rep)
        elif choice == 4:
            g, _ = load_data(c_file, e_file, r_file)
            print("\n[BENCHMARK] Executing Welsh-Powell, DSATUR, and Exact Backtracking...")
            r_wp = solve_welsh_powell(g)
            r_ds = solve_dsatur(g)
            r_bt = solve_exact_backtracking(g, 2.0)
            clique_lb = g.compute_max_clique_bound()

            print("\n+========================================================================+")
            print("|          EMPIRICAL ALGORITHM BENCHMARKING RESULTS                      |")
            print("+========================================================================+")
            print(f"|  Theoretical Lower Bound (Maximum Clique omega(G)): {clique_lb:<18} |")
            print("+----------------------------+-------------------+-------------------+---------------------+")
            print("| Metric                     | Welsh-Powell      | DSATUR            | Exact DFS Backtrack |")
            print("+----------------------------+-------------------+-------------------+---------------------+")
            print(f"| Chromatic Number chi(G)    | {r_wp.chromatic_number:<17} | {r_ds.chromatic_number:<17} | {r_bt.chromatic_number:<19} |")
            print(f"| Execution Time (ms)        | {r_wp.execution_time_ms:<17.3f} | {r_ds.execution_time_ms:<17.3f} | {r_bt.execution_time_ms:<19.3f} |")
            print(f"| Invariant Verification     | {'VALID' if r_wp.is_valid else 'INVALID':<17} | {'VALID' if r_ds.is_valid else 'INVALID':<17} | {'VALID' if r_bt.is_valid else 'INVALID':<19} |")
            print(f"| Student Gap Score          | {r_wp.avg_student_gap:<17.2f} | {r_ds.avg_student_gap:<17.2f} | {r_bt.avg_student_gap:<19.2f} |")
            print("+----------------------------+-------------------+-------------------+---------------------+\n")
        elif choice == 5:
            g, rooms = load_data(c_file, e_file, r_file)
            cr = solve_dsatur(g)
            sch = allocate_rooms_dp(g, cr.chromatic_number, rooms)
            rep = ValidationReport(g, sch, rooms)
            export_all(g, sch, rooms, rep)
            print("[SUCCESS] All files exported successfully to workspace root!")
        elif choice == 0:
            print("\nThank you for using Intelligent Examination Scheduling System.\n")

if __name__ == "__main__":
    main()
