#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "graph.h"
#include "hashmap.h"

#define MAX_ATTRIBS 26
#define MAX_LINE_LEN 256
#define DELIM ","
#define SEP "->"

enum EXPR_SIDES {
    LEFT,
    RIGHT
};

bool is_valid_attrib(char attrib) {
    return 'A' <= attrib && attrib <= 'Z';
}

int parse_attrib_list(char *attrib_list, unsigned int n_attribs,
    char **save_attrib, HashMap *vertices, enum EXPR_SIDES side) {
    
    (void)side;
    /* DEBUG
    if (side == LEFT) {
        printf("LEFT\n");
    } else {
        printf("RIGHT\n");
    }
    */
    // Hash map for all unique vertices found on one expression side
    // (Re-)set to known state
    HashMap_init(vertices);
    // First attribute within attribute list
    char *attrib = strtok_r(attrib_list, DELIM, save_attrib);
    while (attrib != NULL) {
        char *iter = attrib;
        unsigned int index = MAX_ATTRIBS;  // invalid index
        // Tokens from strtok(_r) are NULL-terminated
        while (*iter != '\0') {
            // Check if valid attribute found
            char c = *iter;
            if (is_valid_attrib(c)) {
                // Convert attribute character to index (vertex id)
                index = (unsigned int)(c - 'A');
                if (index >= n_attribs) {
                    fprintf(stderr, "Invalid attribute %c: Expected "
                        "attributes from A to %c\n",
                            c, (char)('A' + (n_attribs-1)));
                    return 1;
                }
                break; 
            }
            ++iter;
        }
        // Check if valid attribute was found
        if (index == MAX_ATTRIBS) {
            fprintf(stderr, "Missing valid attribute <A-Z>\n");
            return 1;
        }
        // Put index in hashmap
        HashMap_insert(vertices, index);
        // Move to next attribute within list
        attrib = strtok_r(NULL, DELIM, save_attrib);
    }
    // DEBUG
    /*
    size_t n_vertices = HashMap_size(vertices);
    size_t current;
    for (current = 0; current < n_vertices; ++current) {
        unsigned int j = HashMap_get(vertices, current);
        printf("%u\n", j);
    }
    */
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./func_dep <functional dependecy file>\n");
        exit(EXIT_FAILURE);
    }
    
    const char *file_name = argv[1];
    // Open file containing information about functional dependencies
    FILE *fp = fopen(file_name, "r");
    // Check for error while opening file
    if (fp == NULL) {
        fprintf(stderr, "Could not open file at '%s'!\n", file_name);
        exit(EXIT_FAILURE);
    }
    // Fetch number of vertices/attributes
    unsigned int n_attribs;
    if (fscanf(fp, "%u\n", &n_attribs) == EOF) {
        fprintf(stderr, "File is empty!\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    } else if (n_attribs == 0 || n_attribs > MAX_ATTRIBS)  {
        fprintf(stderr, "Invalid attribute count: Must be between %u and %u\n",
            1, MAX_ATTRIBS);
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    
    printf("Number of attributes: %u\n", n_attribs);
    
    // Initialize graph
    Graph g;
    Graph_init(&g, n_attribs);
    // Initialize linked list (queue) for visited thresholds
    LinkedList ll_visited_thresh;
    LL_init(&ll_visited_thresh);
    // Insert all known vertices (attributes) with threshold of 1
    unsigned int i_attrib;
    for (i_attrib = 0; i_attrib < n_attribs; ++i_attrib) {
        LL_insert(&ll_visited_thresh, 1);
    }
    // Parse contents of file
    unsigned int line_num = 0;
    char line_buf[MAX_LINE_LEN];
    
    // HashMaps for unique vertices on left/right expression sides
    HashMap h_left, h_right;
    // Save pointers (re-entrant)
    char *save_attrib, *save_attrib_list;
    while (fgets(line_buf, MAX_LINE_LEN, fp) != NULL) {
        // Error code
        int ierr;
        // DEBUG Print current line
        //printf("%u> %s\n", line_num, line_buf);
        
        // Search for tokens
        // Parse left-hand side
        char *attrib_list = strtok_r(line_buf, SEP, &save_attrib_list);
        // Check for missing ->
        if (attrib_list == NULL) {
            fprintf(stderr, "Missing '->'\n");
            fclose(fp);
            Graph_free(&g);
            exit(EXIT_FAILURE);
        }
        // Parse left-hand side
        ierr = parse_attrib_list(attrib_list, n_attribs, 
            &save_attrib, &h_left, LEFT);
        if (ierr) {
            fclose(fp);
            Graph_free(&g);
            exit(EXIT_FAILURE);
        }
        
        // Move to next delimited item
        attrib_list = strtok_r(NULL, SEP, &save_attrib_list);
        // Check for missing right-hand side
        if (attrib_list == NULL) {
            fprintf(stderr, "Right-hand side empty\n");
            fclose(fp);
            Graph_free(&g);
            exit(EXIT_FAILURE);
        }
        // Parse right-hand side
        ierr = parse_attrib_list(attrib_list, n_attribs, &save_attrib, 
            &h_right, RIGHT);
        if (ierr) {
            fclose(fp);
            Graph_free(&g);
            exit(EXIT_FAILURE);
        }
        // Create new vertex if multiple attributes on left
        size_t left = HashMap_size(&h_left);
        size_t right = HashMap_size(&h_right);
        // DEBUG
        assert(left > 0 && right > 0);
        
        size_t i;
        unsigned int other_index;
        if (left > 1) {
            // DEBUG Print new vertex
            //printf("Creating new vertex...\n");
            unsigned int new_index = Graph_new_vertex(&g);
            // Connect all vertices in 'h_left' with new vertex
            for (i = 0; i < left; ++i) {
                other_index = HashMap_get(&h_left, i);
                Graph_add_edge(&g, other_index, new_index);
            }
            // Connect new vertex to every vertex in 'h_right'
            for (i = 0; i < right; ++i) {
                other_index = HashMap_get(&h_right, i);
                Graph_add_edge(&g, new_index, other_index);
            }
            // Set visited thresh for new vertex to 'left'
            LL_insert(&ll_visited_thresh, left);
        } else {
            // Connect (single) vertex on the left with all vertices
            // on the right
            unsigned int left_index = HashMap_get(&h_left, 0);
            for (i = 0; i < right; ++i) {
                other_index = HashMap_get(&h_right, i);
                Graph_add_edge(&g, left_index, other_index);
            }
        }
        // Increment line number
        ++line_num;
    }
    // Close file
    fclose(fp);
    // Dump visited thresh into random access array
    unsigned int *visited_thresh = LL_dump_to_array(&ll_visited_thresh);
    // Visited buffer for each vertex
    unsigned int *visited_buf = (unsigned int *) calloc(g.n_vert,
        sizeof(unsigned int));
    assert(visited_buf != NULL);
    
    // DEBUG
    // Print visited thresh
    for (unsigned int i = 0; i < g.n_vert; ++i) {
        printf("%u %u\n", visited_thresh[i], visited_buf[i]);
    }
    printf("\n");
    // Print adjacency list of graph
    for (unsigned int i = 0; i < g.n_vert; ++i) {
        LinkedList *ll = DArray_get(&g.adjList, i);
        LL_iterator_t iter = LL_iterator(ll);
        if (i < n_attribs) {
            printf("%c > ", (char)(i + 'A'));
        } else {
            printf("%u > ", i);
        }
        while (iter != NULL) {
            if (iter->key < n_attribs) {
                printf("%c ", (char)(iter->key + 'A'));
            } else {
                printf("%u ", iter->key);
            }
            iter = iter->next;
        }
        printf("\n");
    }
    printf("\n");
    // Cleanup visited thresh & buffer
    free(visited_thresh);
    free(visited_buf);
    // Cleanup graph
    Graph_free(&g);
    
    return 0;
}
