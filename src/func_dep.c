/*
 * Computes closure of given set of attributes and list of functional 
 * dependencies found in specified file.
 * 
 * (Suggested) format of functional dependencies (FDs):
 * - (Required) precede FD list with total number of attributes used.
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
#include <time.h>

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

void handle_error(FILE *fp, Queue *L, Queue *R) {
    // Close file
    fclose(fp);
    // Clean up queues
    Q_free(L);
    Q_free(R);
    // Exit with error code
    exit(EXIT_FAILURE);
}

int8_t parse_attrib_list(char *attrib_list, uint8_t n_attribs,
    char **save_attrib, Set *attribs, enum EXPR_SIDES side) {
    
    (void)side;
    /* DEBUG
    if (side == LEFT) {
        printf("LEFT\n");
    } else {
        printf("RIGHT\n");
    }
    */
    // Set for all unique attributes found on one expression side
    // (Re-)set to known state
    Set_init(attribs);
    // First attribute within attribute list
    char *attrib = strtok_r(attrib_list, DELIM, save_attrib);
    while (attrib != NULL) {
        char *iter = attrib;
        uint8_t index = INVALID_ATTRIB;  // invalid index
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
        if (index == INVALID_ATTRIB) {
            fprintf(stderr, "Missing valid attribute <A-Z>\n");
            return 1;
        }
        // Put index in set
        Set_insert(attribs, index);
        // Move to next attribute within list
        attrib = strtok_r(NULL, DELIM, save_attrib);
    }
    
    return 0;
}

// Compute closure of a set of attributes for given queues consisting
// of attributes of left/right sides of all functional dependencies
Set compute_closure(const Set *s, const Queue *L, const Queue *R,
    uint8_t n_attribs) {
    
    // Make sure queues l and r are of same size
    assert(Q_size(L) == Q_size(R));
    
    // Output
    Set closure;
    Set_copy(&closure, s);
    
    bool is_new_attrib = true;
    while (is_new_attrib) {
        // Set no new attrib found
        is_new_attrib = false;
        // Iterate through all functional dependencies
        Q_iterator_t left = Q_iterator(L);
        Q_iterator_t right = Q_iterator(R);
        
        while (left != NULL && right != NULL) {
            // Check if left-hand side is already contained in closure
            // while right-side is not
            if (Set_contains(&closure, &left->key) &&
                !Set_contains(&closure, &right->key)) {
                
                // Add right-hand side to out
                closure = Set_union(&closure, &right->key);
                // Check if closure is already full
                if (Set_is_full(&closure, n_attribs)) {
                    goto end;  // nothing left to add
                }
                // Found new attribute(s)
                is_new_attrib = true;
            }
            // Advance iterators
            left = left->next;
            right = right->next;
        }
    }

end:
    return closure;
}

// Check if set of attributes s is a super-key given functional
// dependencies in l -> r
bool is_superkey(const Set *s, const Queue *L, const Queue *R,
    uint8_t n_attribs) {
    
    // Make sure queues l and r are of same size
    assert(Q_size(L) == Q_size(R));
    
    // Output
    Set closure;
    Set_copy(&closure, s);
    
    bool is_new_attrib = true;
    while (is_new_attrib) {
        // Set no new attrib found
        is_new_attrib = false;
        // Iterate through all functional dependencies
        Q_iterator_t left = Q_iterator(L);
        Q_iterator_t right = Q_iterator(R);
        
        while (left != NULL && right != NULL) {
            // Check if left-hand side is already contained in closure
            // while right-side is not
            if (Set_contains(&closure, &left->key) &&
                !Set_contains(&closure, &right->key)) {
                
                // Add right-hand side to out
                closure = Set_union(&closure, &right->key);
                if (Set_is_full(&closure, n_attribs)) {
                    return true;  // Set s is super-key
                }
                // Found new attribute(s)
                is_new_attrib = true;
            }
            // Advance iterators
            left = left->next;
            right = right->next;
        }
    }
    
    return false;
}

// From paper: Candidate Keys for Relations (journal of computer and
// system sciences 1978) by Claudio Lucchesi and Sylvia Osborn.
// Algorithm. Minimal Key (A, D[0], K)
Set candidate_key_from_super_key(Set *skey, const Queue *L,
    const Queue *R, uint8_t n_attribs) {
    
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
        if (is_superkey(&temp, L, R, n_attribs)) {
            // Attribute attrib is non-essential to ckey -> remove
            Set_copy(&ckey, &temp);
        }
    }
    return ckey;
}

// From paper: Candidate Keys for Relations (journal of computer and
// system sciences 1978) by Claudio Lucchesi and Sylvia Osborn.
// Algorithm. Set of Minimal Keys (A, D[0])
void print_all_candidate_keys(const Queue *L, const Queue *R,
    uint8_t n_attribs) {
    
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
    Set ckey = candidate_key_from_super_key(&attribs, L, R, n_attribs);
    // Print first key
    Set_print(&ckey);
    // Add this ckey to ckeys and work
    Q_insert(&ckeys, ckey);
    Q_insert(&work, ckey);
    // Iterate until no work left (no more candidates to check)
    while (Q_size(&work) != 0) {
        // Fetch current key from work queue
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
            // Test for inclusion of any already found candidate key
            bool test = true;
            // Iterate through all already found candidate keys
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
                // Set S is a super-key and does not contain any already
                // found candidate keys -> compute new candidate key
                ckey = candidate_key_from_super_key(&S, L, R, n_attribs);
                // Add newly found key to both queues
                Q_insert(&ckeys, ckey);
                Q_insert(&work, ckey);
                // Print candidate key
                Set_print(&ckey);
            }
            // Advance iterators
            left = left->next;
            right = right->next;
        }
    }
    // Print number of candidate keys found
    printf("Number of candidate keys: %u\n", Q_size(&ckeys));
    // Cleanup
    Q_free(&ckeys);
    Q_free(&work);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
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
            handle_error(fp, &q_left, &q_right);
        }
        // Parse left-hand side
        ierr = parse_attrib_list(attrib_list, n_attribs, 
            &save_attrib, &s_left, LEFT);
        if (ierr) {
            handle_error(fp, &q_left, &q_right);
        }
        
        // Move to next delimited item
        attrib_list = strtok_r(NULL, SEP, &save_attrib_list);
        // Check for missing right-hand side
        if (attrib_list == NULL) {
            fprintf(stderr, "Right-hand side empty\n");
            handle_error(fp, &q_left, &q_right);
        }
        // Parse right-hand side
        ierr = parse_attrib_list(attrib_list, n_attribs, &save_attrib, 
            &s_right, RIGHT);
        if (ierr) {
            handle_error(fp, &q_left, &q_right);
        }
        // Add sets of attributes to respective queues
        Q_insert(&q_left, s_left);
        Q_insert(&q_right, s_right);
        // Increment line number
        ++line_num;
    }
    // Close file
    fclose(fp);
    // Print closure of attributes from command line
    //print_attribute_closure(&g, &attrib_cml, visited_buf, 
    //    visited_thresh, n_attribs);
    printf("Candidate keys for FDs in '%s':\n", file_name);
    // Measure CPU time
    clock_t start = clock(), elapsed;
    // Print all candidate keys of functional dependencies to console
    print_all_candidate_keys(&q_left, &q_right, n_attribs);
    // Elapsed CPU time
    elapsed = clock() - start;
    double seconds = (double)elapsed / CLOCKS_PER_SEC;
    printf("Took: %.3e s\n", seconds);
    // Cleanup queues
    Q_free(&q_left);
    Q_free(&q_right);
    
    return EXIT_SUCCESS;
}
