/**
 * @file graph.c
 * @brief Implementation of Graph Data Structures (Adjacency Matrix & List).
 * @course Data Structures and Algorithms - II (CCSE0301) - Module 2
 * @author Ankit Pandey (ERP ID: 2501331520029)
 */

#include "../include/graph.h"

Graph* create_graph(int num_vertices) {
    if (num_vertices <= 0) return NULL;

    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) {
        fprintf(stderr, "[ERROR] Memory allocation failed for Graph.\n");
        return NULL;
    }

    graph->num_vertices = num_vertices;
    graph->num_edges = 0;

    graph->courses = (Course*)calloc(num_vertices, sizeof(Course));
    graph->adj_list = (AdjList*)calloc(num_vertices, sizeof(AdjList));
    
    // Allocate Adjacency Matrix
    graph->adj_matrix = (int**)malloc(num_vertices * sizeof(int*));
    for (int i = 0; i < num_vertices; i++) {
        graph->adj_matrix[i] = (int*)calloc(num_vertices, sizeof(int));
        graph->courses[i].id = i;
        graph->courses[i].timeslot = -1;
        graph->courses[i].room_id = -1;
        graph->courses[i].enrolled_students = 0;
        graph->adj_list[i].head = NULL;
        graph->adj_list[i].degree = 0;
        graph->adj_list[i].saturation_degree = 0;
    }

    return graph;
}

void free_graph(Graph* graph) {
    if (!graph) return;

    if (graph->adj_matrix) {
        for (int i = 0; i < graph->num_vertices; i++) {
            free(graph->adj_matrix[i]);
        }
        free(graph->adj_matrix);
    }

    if (graph->adj_list) {
        for (int i = 0; i < graph->num_vertices; i++) {
            AdjListNode* curr = graph->adj_list[i].head;
            while (curr) {
                AdjListNode* tmp = curr;
                curr = curr->next;
                free(tmp);
            }
        }
        free(graph->adj_list);
    }

    if (graph->courses) {
        free(graph->courses);
    }

    free(graph);
}

void add_conflict_edge(Graph* graph, int src, int dest, int clash_count) {
    if (!graph || src < 0 || src >= graph->num_vertices || dest < 0 || dest >= graph->num_vertices) return;
    if (src == dest) return; // No self loops

    // Check if edge already exists in matrix
    if (graph->adj_matrix[src][dest] > 0) {
        // Update clash count weight
        graph->adj_matrix[src][dest] += clash_count;
        graph->adj_matrix[dest][src] += clash_count;

        // Update list weight
        AdjListNode* curr = graph->adj_list[src].head;
        while (curr) {
            if (curr->dest == dest) { curr->weight += clash_count; break; }
            curr = curr->next;
        }
        curr = graph->adj_list[dest].head;
        while (curr) {
            if (curr->dest == src) { curr->weight += clash_count; break; }
            curr = curr->next;
        }
        return;
    }

    // Set matrix entries (Undirected edge)
    graph->adj_matrix[src][dest] = clash_count;
    graph->adj_matrix[dest][src] = clash_count;

    // Insert into Adjacency List (src -> dest)
    AdjListNode* node_dest = (AdjListNode*)malloc(sizeof(AdjListNode));
    node_dest->dest = dest;
    node_dest->weight = clash_count;
    node_dest->next = graph->adj_list[src].head;
    graph->adj_list[src].head = node_dest;
    graph->adj_list[src].degree++;

    // Insert into Adjacency List (dest -> src)
    AdjListNode* node_src = (AdjListNode*)malloc(sizeof(AdjListNode));
    node_src->dest = src;
    node_src->weight = clash_count;
    node_src->next = graph->adj_list[dest].head;
    graph->adj_list[dest].head = node_src;
    graph->adj_list[dest].degree++;

    graph->num_edges++;
}

bool has_conflict(const Graph* graph, int u, int v) {
    if (!graph || u < 0 || u >= graph->num_vertices || v < 0 || v >= graph->num_vertices) return false;
    return (graph->adj_matrix[u][v] > 0);
}

int get_clash_count(const Graph* graph, int u, int v) {
    if (!graph || u < 0 || u >= graph->num_vertices || v < 0 || v >= graph->num_vertices) return 0;
    return graph->adj_matrix[u][v];
}

int find_connected_components(const Graph* graph, int* component_map) {
    if (!graph || !component_map) return 0;

    int n = graph->num_vertices;
    bool* visited = (bool*)calloc(n, sizeof(bool));
    int component_count = 0;

    int* queue = (int*)malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            int front = 0, rear = 0;
            queue[rear++] = i;
            visited[i] = true;
            component_map[i] = component_count;

            while (front < rear) {
                int u = queue[front++];
                AdjListNode* curr = graph->adj_list[u].head;
                while (curr) {
                    int v = curr->dest;
                    if (!visited[v]) {
                        visited[v] = true;
                        component_map[v] = component_count;
                        queue[rear++] = v;
                    }
                    curr = curr->next;
                }
            }
            component_count++;
        }
    }

    free(queue);
    free(visited);
    return component_count;
}

void print_graph_summary(const Graph* graph) {
    if (!graph) return;

    printf("\n+========================================================================+\n");
    printf("|                ACADEMIC CONFLICT GRAPH SUMMARY                         |\n");
    printf("+========================================================================+\n");
    printf("|  Total Courses (|V|)     : %-42d |\n", graph->num_vertices);
    printf("|  Total Conflict Edges (|E|): %-40d |\n", graph->num_edges);
    
    double max_edges = (double)graph->num_vertices * (graph->num_vertices - 1) / 2.0;
    double density = (max_edges > 0) ? (double)graph->num_edges / max_edges : 0.0;
    printf("|  Graph Density (D)       : %-42.4f |\n", density);

    int* components = (int*)malloc(graph->num_vertices * sizeof(int));
    int comp_count = find_connected_components(graph, components);
    printf("|  Connected Components    : %-42d |\n", comp_count);
    free(components);

    printf("+----+----------+------------------------------+------------+--------+---+\n");
    printf("| ID | Code     | Course Title                 | Department | Enrolled | Deg |\n");
    printf("+----+----------+------------------------------+------------+--------+---+\n");
    for (int i = 0; i < graph->num_vertices; i++) {
        printf("| %2d | %-8s | %-28s | %-10s | %6d | %3d |\n",
               graph->courses[i].id,
               graph->courses[i].code,
               graph->courses[i].title,
               graph->courses[i].department,
               graph->courses[i].enrolled_students,
               graph->adj_list[i].degree);
    }
    printf("+----+----------+------------------------------+------------+--------+---+\n");
}

/* CSV Ingestion Helpers */
static int find_course_id_by_code(const Graph* graph, const char* code) {
    for (int i = 0; i < graph->num_vertices; i++) {
        if (strcmp(graph->courses[i].code, code) == 0) {
            return i;
        }
    }
    return -1;
}

Graph* load_dataset_from_csv(const char* courses_file, const char* enrollments_file) {
    FILE* fc = fopen(courses_file, "r");
    if (!fc) {
        fprintf(stderr, "[ERROR] Could not open courses file: %s\n", courses_file);
        return NULL;
    }

    char line[512];
    int course_count = 0;

    // Count non-empty lines (excluding header)
    if (fgets(line, sizeof(line), fc)) {
        while (fgets(line, sizeof(line), fc)) {
            if (strlen(line) > 3) course_count++;
        }
    }
    rewind(fc);

    Graph* graph = create_graph(course_count);
    if (!graph) { fclose(fc); return NULL; }

    // Read course lines
    if (fgets(line, sizeof(line), fc)) { // Skip header
        int idx = 0;
        while (fgets(line, sizeof(line), fc) && idx < course_count) {
            line[strcspn(line, "\r\n")] = '\0';
            if (strlen(line) < 3) continue;

            char code[MAX_CODE_LEN] = {0};
            char title[MAX_NAME_LEN] = {0};
            char dept[MAX_DEPT_LEN] = {0};
            int enrolled = 0;

            // Format: code,title,department,enrolled
            char* token = strtok(line, ",");
            if (token) strncpy(code, token, sizeof(code) - 1);
            token = strtok(NULL, ",");
            if (token) strncpy(title, token, sizeof(title) - 1);
            token = strtok(NULL, ",");
            if (token) strncpy(dept, token, sizeof(dept) - 1);
            token = strtok(NULL, ",");
            if (token) enrolled = atoi(token);

            graph->courses[idx].id = idx;
            strncpy(graph->courses[idx].code, code, sizeof(graph->courses[idx].code) - 1);
            strncpy(graph->courses[idx].title, title, sizeof(graph->courses[idx].title) - 1);
            strncpy(graph->courses[idx].department, dept, sizeof(graph->courses[idx].department) - 1);
            graph->courses[idx].enrolled_students = enrolled;
            idx++;
        }
    }
    fclose(fc);

    // Read enrollments file: format: student_id,course_code
    FILE* fe = fopen(enrollments_file, "r");
    if (!fe) {
        fprintf(stderr, "[ERROR] Could not open enrollments file: %s\n", enrollments_file);
        return graph;
    }

    // Structure to hold student -> courses registered
    typedef struct StudentReg {
        char student_id[32];
        int course_ids[32];
        int count;
    } StudentReg;

    int max_students = 5000;
    StudentReg* students = (StudentReg*)calloc(max_students, sizeof(StudentReg));
    int total_students = 0;

    if (fgets(line, sizeof(line), fe)) { // Skip header
        while (fgets(line, sizeof(line), fe)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (strlen(line) < 3) continue;

            char stu_id[32] = {0};
            char c_code[16] = {0};
            char* t1 = strtok(line, ",");
            char* t2 = strtok(NULL, ",");
            if (t1 && t2) {
                strncpy(stu_id, t1, sizeof(stu_id) - 1);
                strncpy(c_code, t2, sizeof(c_code) - 1);

                int cid = find_course_id_by_code(graph, c_code);
                if (cid != -1) {
                    // Find or create student record
                    int s_idx = -1;
                    for (int s = 0; s < total_students; s++) {
                        if (strcmp(students[s].student_id, stu_id) == 0) {
                            s_idx = s;
                            break;
                        }
                    }
                    if (s_idx == -1 && total_students < max_students) {
                        s_idx = total_students++;
                        strncpy(students[s_idx].student_id, stu_id, sizeof(students[s_idx].student_id) - 1);
                        students[s_idx].count = 0;
                    }
                    if (s_idx != -1 && students[s_idx].count < 32) {
                        students[s_idx].course_ids[students[s_idx].count++] = cid;
                    }
                }
            }
        }
    }
    fclose(fe);

    // Build conflict graph: for each student, connect all pairs of their courses
    for (int s = 0; s < total_students; s++) {
        for (int i = 0; i < students[s].count; i++) {
            for (int j = i + 1; j < students[s].count; j++) {
                int c1 = students[s].course_ids[i];
                int c2 = students[s].course_ids[j];
                add_conflict_edge(graph, c1, c2, 1);
            }
        }
    }

    free(students);
    return graph;
}
