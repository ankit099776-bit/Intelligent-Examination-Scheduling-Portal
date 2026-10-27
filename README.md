# Intelligent Examination Scheduling Portal

[![Course](https://img.shields.io/badge/Course-DSA--II%20(CCSE0301)-blue.svg)](https://github.com/ankit099776-bit/Intelligent-Examination-Scheduling-Portal)
[![Student](https://img.shields.io/badge/Author-Ankit%20Pandey%20(2501331520029)-orange.svg)](https://github.com/ankit099776-bit)
[![SDG](https://img.shields.io/badge/SDG-4%20Quality%20Education-brightgreen.svg)](https://sdgs.un.org/goals/goal4)
[![Language](https://img.shields.io/badge/Language-ANSI%20C%20%2F%20Python%203.12-blueviolet.svg)](https://github.com/ankit099776-bit/Intelligent-Examination-Scheduling-Portal)
[![Tests](https://img.shields.io/badge/Verification%20Suite-7%2F7%20Passed%20(100%25)-success.svg)](https://github.com/ankit099776-bit/Intelligent-Examination-Scheduling-Portal)

> **Academic Milestone Project for Data Structures and Algorithms – II (`CCSE0301`)**  
> **Student:** Ankit Pandey (ERP ID: `2501331520029`)  
> **Programme:** B.Tech Computer Science and Engineering (Artificial Intelligence)  
> **Department:** Department of Computer Science & Engineering  
> **Institution:** Noida Institute of Engineering and Technology (NIET), Greater Noida  
> **Faculty Mentor:** Mr Sachin Singh  

---

## 1. Project Overview

The **Intelligent Examination Scheduling Portal** is an algorithmic timetable optimization and automated classroom seating system developed for university examination cells. It solves the NP-hard academic examination timetabling and room seating problem using principles from **Data Structures and Algorithms – II**.

Traditional university examination scheduling is manual, error-prone, and causes student exam clashes when dealing with modern elective courses and cross-departmental curricula. This system models the entire university exam schedule as an undirected **Conflict Graph** $G = (V, E)$, where:
- **Vertices ($V$):** Represent individual examination courses/papers.
- **Edges ($E$):** Connect pairs of courses that share registered students, representing hard conflict constraints ($\text{edge}(u, v) \implies \text{slot}(u) \neq \text{slot}(v)$).
- **Colors (Time Slots):** Represent conflict-free examination **days/sessions**.

### Key System Capabilities
1. **Zero Conflict 6-Day Examination Calendar:** Exactly 6 days (Day 1 to Day 6: Monday to Saturday, 10:00 AM – 01:00 PM), proven optimal by calculating the Maximum Clique Lower Bound ($\chi(G) = \omega(G) = 6$).
2. **Department-Specific Separate Timetables:** Dedicated filter views for individual departments (`CSE`, `CSE-AI`, `CSE-AIML`, `CSE-DS`, `IT`, `ECE`), showing their specific conflict-free exam dates, timings, subjects, assigned halls, and built-in revision gap days.
3. **Classroom Co-Seating & Anti-Cheating Bench Pairing:** When searching any department (e.g., typing **`AI`**), the system displays which exam hall students are seated in, **which other department's class they are sitting with**, their respective non-conflicting subjects, and the alternating bench arrangement pattern (Odd columns: Dept 1, Even columns: Dept 2).
4. **Instant Downloadable Formats:**
   - **Download Seating Plan (CSV / Excel):** Generates structured `.csv` records.
   - **Print / Save PDF Notice Chart:** Formatted official NIET Central Examination Cell notice board matrix with signature lines for Invigilator and Center Superintendent.
5. **Full Red Academic Portal & Authentication:** Built with NIET official crimson branding, ERP login authentication, and fast 1-click evaluation demo access.

---

## 2. DSA Module Architecture & Syllabus Alignment

The system directly implements concepts from all five units of the **DSA-II (`CCSE0301`)** curriculum:

```
+---------------------------------------------------------------------------------------------------+
|                        INTELLIGENT EXAMINATION SCHEDULING PORTAL                                  |
+---------------------------------------------------------------------------------------------------+
       |                                      |                                      |
       v                                      v                                      v
 [Module 1: Trees]                      [Module 2: Graphs]                    [Module 3: DP]
  • Binary Max-Heap                      • Adjacency Matrix A[V][V]            • 0/1 Knapsack Recurrence
  • PosMap O(log V) priority update       • Adjacency List Adj[V]               • Room Capacity Packing
  • Heap Sort O(V log V)                 • BFS Connected Components            • Cohort Partitioning
       |                                      |                                      |
       +--------------------------------------+--------------------------------------+
                                              |
                                              v
                        [Module 4: Backtracking & Verification]
                         • Branch & Bound Exact Solver
                         • Maximum Clique Lower Bound (omega(G) = 6)
                         • Hard Constraint Zero-Clash Audit
```

| Module | Core Concept | Role in Exam Scheduler | Complexity |
| :--- | :--- | :--- | :--- |
| **Module 1 (Trees)** | Binary Max-Heap & Priority Queues | Drives the **DSATUR** algorithm by dynamically tracking maximum saturation degree $O(\log V)$ | Insertion: $O(\log V)$, Extract-Max: $O(\log V)$ |
| **Module 2 (Graphs)** | Adjacency List & Graph Coloring | Models student clashes as conflict edges; colors vertices with minimum time slots ($\chi(G) = 6$) | Graph Construction: $O(S \cdot C^2)$, DSATUR: $O(V^2 \log V)$ |
| **Module 3 (DP)** | 0/1 Knapsack & Bellman Optimality | Packs student cohorts into classrooms and pairs non-conflicting departments on alternating benches | DP Seating: $O(N \cdot W)$, Sorting: $O(R \log R)$ |
| **Module 4 (Backtracking)** | Branch and Bound DFS | Computes the exact minimum chromatic number with pruning; proves $\chi(G) = \omega(G) = 6$ | Worst-Case: $O(k^V)$, Pruned: $< 2$ ms |

---

## 3. Mathematical Proof of Timetable Optimality

By the **Clique Lower Bound Theorem**:
$$\chi(G) \ge \omega(G)$$
Where:
- $\chi(G)$ is the Chromatic Number (minimum days/slots needed).
- $\omega(G)$ is the Clique Number (size of the largest subgraph where every pair of exams conflicts).

In the NIET curriculum conflict graph, the 6-clique formed by mutually overlapping core student registrations requires at least 6 days:
$$\omega(G) = 6 \implies \chi(G) \ge 6$$
The DSATUR heuristic and Exact Backtracking solver both achieve $\chi(G) = 6$, mathematically guaranteeing that **no algorithm can ever schedule this curriculum in fewer than 6 days**.

---

## 4. Verification & Testing Suite (100% Pass Rate)

The system includes an automated test harness covering seven rigorous test cases (TC01 to TC07):

| Test ID | Test Category | Mathematical Invariant | Result |
| :--- | :--- | :--- | :--- |
| **TC01** | Edge Boundary | Disconnected Graph ($V=6, E=0 \implies \chi(G) = 1$) | **PASS** |
| **TC02** | Direct Conflict | Single Edge ($V=2, E=1 \implies \text{slot}(u) \neq \text{slot}(v)$) | **PASS** |
| **TC03** | Complete Graph ($K_3$) | 3-Clique ($V=3, E=3 \implies \chi(G) = \omega(G) = 3$) | **PASS** |
| **TC04** | Star Graph Hub | Central Hub ($V=6, E=5 \implies \text{Hub receives Slot 0}, \chi(G) = 2$) | **PASS** |
| **TC05** | Capacity Overflow | Cohort exceeds single room capacity $\implies$ Multi-hall splitting | **PASS** |
| **TC06** | Heuristic Comparison | Dense graph coloring $\implies \chi(\text{DSATUR}) \le \chi(\text{Welsh-Powell})$ | **PASS** |
| **TC07** | Input Robustness | Corrupt bounds and duplicate entries $\implies$ Graceful rejection | **PASS** |

---

## 5. Quick Start & Execution

### Option 1: Live Interactive Web Portal (Zero External Dependencies)
```bash
# Clone the repository
git clone https://github.com/ankit099776-bit/Intelligent-Examination-Scheduling-Portal.git
cd Intelligent-Examination-Scheduling-Portal

# Launch the server (Python 3.8+)
python app.py
```
Open your browser at `http://localhost:5000` to access the portal.

### Option 2: Core Algorithm Verification & Benchmark Engine
```bash
python scheduler.py
```

### Option 3: Compile ANSI C Core Engine (C99)
```bash
make
./scheduler_c
```

---

## 6. Project Structure

```
Intelligent-Examination-Scheduling-Portal/
├── app.py                      # REST API & Web Application Server (Python 3.12)
├── scheduler.py                # Core Graph Theory & Dynamic Programming Engine
├── Makefile                    # ANSI C build configuration
├── build.bat                   # Windows batch build helper
├── start_web.bat               # 1-click double-click launcher
├── data/                       # NIET curriculum, student enrollments, and room capacity CSVs
│   ├── sample_courses.csv
│   ├── sample_enrollments.csv
│   └── sample_rooms.csv
├── include/                    # ANSI C Header definitions
│   └── scheduler.h
├── src/                        # ANSI C modular implementations
│   ├── conflict_graph.c
│   ├── graph_coloring.c
│   ├── priority_queue.c
│   ├── room_allocation.c
│   ├── validator.c
│   └── main.c
├── web/                        # Web Portal Front-end
│   ├── app.html                # Red-themed NIET Portal UI with Login & Seating Matrix
│   ├── index.html              # Offline bundle
│   └── niet_logo.png           # Official NIET Institution Logo
└── README.md                   # Complete architectural and academic documentation
```

---

## 7. Sustainable Development Goal (SDG) Alignment

- **SDG 4: Quality Education (Target 4.3 & 4.4)**  
  Eliminates human scheduling errors, prevents student academic distress from overlapping examinations, guarantees fair study gaps, and ensures zero cheating via automated non-conflicting bench pairing.
