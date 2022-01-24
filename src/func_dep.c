/*
 * Computes closure of given set of attributes and list of functional 
 * dependencies found in specified file.
 * 
 * (Suggested) format of functional dependencies (FDs):
 * - Precede FD list with total number of attributes used.
 * - Unique left-hand sides, i.e. A,B->C & A,B->D becomes A,B->C,D.
 * - This is only recommended for multi-attribute left-hand sides, as
 *   this avoids unnecessary generation of a new auxiliary attribute 
 *   graph vertex.
 * 
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "graph.h"
#include "set.h"
#include "queue.h"

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

int8_t parse_attrib_list(char *attrib_list, uint8_t n_attribs,
    char **save_attrib, Set *vertices, enum EXPR_SIDES side) {
    
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
    Set_init(vertices);
    // First attribute within attribute list
    char *attrib = strtok_r(attrib_list, DELIM, save_attrib);
    while (attrib != NULL) {
        char *iter = attrib;
        uint8_t index = MAX_ATTRIBS;  // invalid index
        // Tokens from strtok(_r) are NULL-terminated
        while (*iter != '\0') {
            // Check if valid attribute found
            char c = *iter;
            if (is_valid_attrib(c)) {
                // Convert attribute character to index (vertex id)
                index = (uint8_t)(c - 'A');
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
        // Put index in set
        Set_insert(vertices, index);
        // Move to next attribute within list
        attrib = strtok_r(NULL, DELIM, save_attrib);
    }
    return 0;
}

bool check_super_key(const Graph *g, Set *attrib,
    uint32_t *visited_buf, const uint32_t *visited_thresh,
    uint8_t n_attribs) {
    
    // Initialize visited_buf to known state
    uint8_t j;
    for (j = 0; j < g->n_vert; ++j) {
        visited_buf[j] = 0;
    }
    
    for (j = 0; j < attrib->size; ++j) {
        // Fetch attribute index
        uint8_t index = Set_next_pos(attrib);
        // Compute closure of current attribute -> update visited_buf
        Graph_BFS_closure(g, index, visited_buf, visited_thresh);
    }
    // Check if any attribute was NOT reached
    for (j = 0; j < n_attribs; ++j) {
        if (visited_buf[j] != 1) {
            return false;
        }
    }
    return true;  // attribute set is super-key
}

// From paper: Candidate Keys for Relations (journal of computer and
// system sciences 1978) by Claudio Lucchesi and Sylvia Osborn.
// Algorithm. Minimal Key (A, D[0], K)
Set candidate_key_from_super_key(const Graph *g, Set *skey,
    uint32_t *visited_buf, const uint32_t *visited_thresh, 
    uint8_t n_attribs) {
    
    Set ckey, temp;
    // Copy attributes
    Set_copy(&ckey, skey);
    
    // Iterate over attributes of super-key
    uint8_t i, attrib;
    for (i = 0; i < skey->size; ++i) {
        // Fetch current attribute
        attrib = Set_next_pos(skey);
        // Copy ckey into temp
        Set_copy(&temp, &ckey);
        // Remove attribute from temp
        Set_remove(&temp, attrib);
        // Check if ckey - attrib is still a super-key
        if (check_super_key(g, &temp, visited_buf, 
                                visited_thresh, n_attribs))
        {
            // Attribute attrib is non-essential to ckey -> remove
            Set_copy(&ckey, &temp);
        }
    }
    return ckey;
}

// From paper: Candidate Keys for Relations (journal of computer and
// system sciences 1978) by Claudio Lucchesi and Sylvia Osborn.
// Algorithm. Set of Minimal Keys (A, D[0])
void print_all_candidate_keys(const Graph *g, const Queue *L,
    const Queue *R, uint32_t *visited_buf, 
    const uint32_t *visited_thresh, uint8_t n_attribs) {
    
    // Make sure attribute queues of FDs are of the same size
    assert(Q_size(L) == Q_size(R));
    // Queues for ckeys and work left
    Queue ckeys, work;
    Q_init(&ckeys);
    Q_init(&work);
    
    // Initialize set of all attributes
    Set attribs;
    Set_full(&attribs, n_attribs);
    // Compute first ckey using all attributes
    Set ckey = candidate_key_from_super_key(g, &attribs, visited_buf,
        visited_thresh, n_attribs);
    // Print first key
    Set_print(&ckey);
    // Add this ckey to ckeys and work
    Q_insert(&ckeys, ckey);
    Q_insert(&work, ckey);
    // Iterate until no work left (no candidates to check
    while (Q_size(&work) != 0) {
        // Fetch kurrent key from work queue
        Set key = Q_pop(&work);
        // Iterate over all FDs
        Q_iterator_t left = Q_iterator(L);
        Q_iterator_t right = Q_iterator(R);
        
        while (left != NULL && right != NULL) {
            // Obtain sets of attributes of individual side of current
            // functional dependency
            Set s_left = left->key;
            Set s_right = right->key;
            // Compute S
            Set diff = Set_difference(&key, &s_right);
            Set S = Set_union(&s_left, &diff);
            // Test for inclusion
            bool test = true;
            // Iterate through already found candidate keys
            Q_iterator_t ckey_iter = Q_iterator(&ckeys);
            while (ckey_iter != NULL) {
                Set J = ckey_iter->key;
                // Check for inclusion
                if (Set_contains(&S, &J)) {
                    test = false;
                    break;
                }
                // Advance
                ckey_iter = ckey_iter->next;
            }
            
            if (test) {
                // Compute new candidate key
                Set new_key = candidate_key_from_super_key(g, &S, visited_buf,
                    visited_thresh, n_attribs);
                // Add newly found key to both queues
                Q_insert(&ckeys, new_key);
                Q_insert(&work, new_key);
                // Print candidate key
                Set_print(&new_key);
            }
            // Advance iterators
            left = left->next;
            right = right->next;
        }
    }
    
    // Cleanup
    Q_free(&ckeys);
    Q_free(&work);
}

// Compute closure of a given set of attributes
void print_attribute_closure(const Graph *g, Set *attrib,
    uint32_t *visited_buf, const uint32_t *visited_thresh,
    uint8_t n_attribs) {
    
    // Initialize visited_buf to known state
    uint8_t j;
    for (j = 0; j < g->n_vert; ++j) {
        visited_buf[j] = 0;
    }
    
    printf("Closure of: ");
    for (j = 0; j < attrib->size; ++j) {
        uint8_t index = Set_next_pos(attrib);
        printf("%c ", (char)(index + 'A'));
        // Compute of current attribute -> update visited_buf
        Graph_BFS_closure(g, index, visited_buf, visited_thresh);
    }
    printf("\nis...\n");
    
    bool is_super_key = true;
    for (j = 0; j < n_attribs; ++j) {
        if (visited_buf[j] == 1) {
            printf("%c ", (char)(j + 'A'));
        } else {
            is_super_key = false;
        }
    }
    printf("\nSuper-key? ");
    if (is_super_key) {
        printf("Yes\n");
    } else {
        printf("No\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./func_dep <functional dependecy file>" 
                " <list of attributes>\n");
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
    uint8_t n_attribs;
    if (fscanf(fp, "%hhu\n", &n_attribs) == EOF) {
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
    /*
    // Read attributes
    Set attrib_cml;
    Set_init(&attrib_cml);
    
    int arg_offset = 2;
    for (int j = arg_offset; j < argc; ++j) {
        char c = argv[j][0];
        // Check if valid attribute
        if (!is_valid_attrib(c)) {
            fprintf(stderr, "Invalid attribute, must be <A-Z>\n");
            fclose(fp);
            exit(EXIT_FAILURE);
        }
        uint8_t index = (uint8_t)(c - 'A');
        // Check if index is in valid range
        if (index >= n_attribs) {
            fprintf(stderr, "Invalid attribute: %c, expected attributes"
                " from A to %c\n", c, (char)('A' + (n_attribs - 1)));
            fclose(fp);
            exit(EXIT_FAILURE);
        }
        // Insert attribute in hash map
        Set_insert(&attrib_cml, index);
    }
    */
    // Initialize graph
    Graph g;
    Graph_init(&g, n_attribs);
    // Initialize linked list for visited thresholds
    LinkedList ll_visited_thresh;
    LL_init(&ll_visited_thresh);
    // Insert all known vertices (attributes) with threshold of 1
    uint8_t i_attrib;
    for (i_attrib = 0; i_attrib < n_attribs; ++i_attrib) {
        LL_insert(&ll_visited_thresh, 1);
    }
    // Parse contents of file
    uint32_t line_num = 0;
    char line_buf[MAX_LINE_LEN];
    
    // Sets for unique vertices on left/right expression sides
    Set s_left, s_right;
    // Queues to store attributes on left/right side of expression
    Queue q_left, q_right;
    Q_init(&q_left);
    Q_init(&q_right);
    
    // Save pointers (re-entrant)
    char *save_attrib, *save_attrib_list;
    while (fgets(line_buf, MAX_LINE_LEN, fp) != NULL) {
        // Error code
        int8_t ierr;
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
            &save_attrib, &s_left, LEFT);
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
            &s_right, RIGHT);
        if (ierr) {
            fclose(fp);
            Graph_free(&g);
            exit(EXIT_FAILURE);
        }
        // Add sets of attributes to respective queues
        Q_insert(&q_left, s_left);
        Q_insert(&q_right, s_right);
        // Create new vertex if multiple attributes on left side
        uint8_t left = s_left.size;
        uint8_t right = s_right.size;
        // DEBUG
        assert(left > 0 && right > 0);
        // Connect relevant edges to build directed graph and
        // add newly created vertices with their respective 'visitation
        // threshold'
        uint8_t i;
        uint8_t other_index;
        if (left > 1) {
            // DEBUG Print new vertex
            //printf("Creating new vertex...\n");
            uint32_t new_index = Graph_new_vertex(&g);
            // Connect all vertices in 'h_left' with new vertex
            for (i = 0; i < left; ++i) {
                other_index = Set_next_pos(&s_left);
                Graph_add_edge(&g, other_index, new_index);
            }
            // Connect new vertex to every vertex in 'h_right'
            for (i = 0; i < right; ++i) {
                other_index = Set_next_pos(&s_right);
                Graph_add_edge(&g, new_index, other_index);
            }
            // Set visited thresh for new vertex to 'left'
            LL_insert(&ll_visited_thresh, left);
        } else {
            // Connect (single) vertex on the left with all vertices
            // on the right
            uint8_t left_index = Set_next_pos(&s_left);
            for (i = 0; i < right; ++i) {
                other_index = Set_next_pos(&s_right);
                Graph_add_edge(&g, left_index, other_index);
            }
        }
        // Increment line number
        ++line_num;
    }
    // Close file
    fclose(fp);
    // Dump visited thresh into random access array
    uint32_t *visited_thresh = LL_dump_to_array(&ll_visited_thresh);
    // Visited buffer for each vertex
    uint32_t *visited_buf = (uint32_t *) malloc(g.n_vert *
        sizeof(uint32_t));
    assert(visited_buf != NULL);
    // Print closure of attributes from command line
    //print_attribute_closure(&g, &attrib_cml, visited_buf, 
    //    visited_thresh, n_attribs);
    printf("Candidate keys for FDs in '%s':\n", file_name);
    print_all_candidate_keys(&g, &q_left, &q_right, visited_buf,
        visited_thresh, n_attribs);
    // DEBUG
    /*
    // Print visited thresh
    for (uint32_t i = 0; i < g.n_vert; ++i) {
        printf("%u %u\n", visited_thresh[i], visited_buf[i]);
    }
    printf("\n");
    // Print adjacency list of graph
    for (uint32_t i = 0; i < g.n_vert; ++i) {
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
    */
    // Cleanup queues
    Q_free(&q_left);
    Q_free(&q_right);
    // Cleanup visited thresh & buffer
    free(visited_thresh);
    free(visited_buf);
    // Cleanup graph
    Graph_free(&g);
    
    return 0;
}
