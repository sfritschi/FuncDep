/*
 * Prints list of all existing candidate keys given functional
 * dependencies read from file. 
 * 
 * Format of functional dependencies (FDs):
 * - (Required) precede FD list with total number of attributes used.
 * - May only use attribute names starting from 'A' to #attributes
 *   next letters in the alphabet. 
 * 
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "set.h"
#include "queue.h"

#define MAX_LINE_LEN 256
#define DELIM ","
#define SEP "->"

uint8_t is_valid_attrib(char attrib) {
    return 'A' <= attrib && attrib <= 'Z';
}

void exit_on_error(FILE *fp, Queue *q) {
    // Close file
    fclose(fp);
    // Clean up queues
    Q_free(q);
    // Exit with error code
    exit(EXIT_FAILURE);
}

int8_t parse_attrib_list(char *attrib_list, uint8_t n_attribs,
    char **save_attrib, Set *attribs) {
    
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
Set compute_closure(const Set *s, const Queue *q, uint8_t n_attribs) {
    // Output
    Set closure;
    Set_copy(&closure, s);
    
    uint8_t is_new_attrib = 1;
    while (is_new_attrib) {
        // Set no new attrib found
        is_new_attrib = 0;
        // Iterate through all functional dependencies
        Q_iterator_t iter = Q_iterator(q);
        
        while (iter) {
            // Check if left-hand side is already contained in closure
            // while right-side is not
            if (Set_contains(&closure, &iter->key.lhs) &&
                !Set_contains(&closure, &iter->key.rhs)) {
                
                // Add right-hand side to out
                closure = Set_union(&closure, &iter->key.rhs);
                // Check if closure is already full
                if (Set_is_full(&closure, n_attribs)) {
                    goto end;  // nothing left to add
                }
                // Found new attribute(s)
                is_new_attrib = 1;
            }
            // Advance iterators
            iter = iter->next;
        }
    }

end:
    return closure;
}

// Check if set of attributes s is a super-key given functional
// dependencies in l -> r
uint8_t is_superkey(const Set *s, const Queue *q, uint8_t n_attribs) {
    // Output
    Set closure;
    Set_copy(&closure, s);
    
    uint8_t is_new_attrib = 1;
    while (is_new_attrib) {
        // Set no new attrib found
        is_new_attrib = 0;
        // Iterate through all functional dependencies
        Q_iterator_t iter = Q_iterator(q);
        
        while (iter) {
            // Check if left-hand side is already contained in closure
            // while right-side is not
            if (Set_contains(&closure, &iter->key.lhs) &&
                !Set_contains(&closure, &iter->key.rhs)) {
                
                // Add right-hand side to out
                closure = Set_union(&closure, &iter->key.rhs);
                if (Set_is_full(&closure, n_attribs)) {
                    return 1;  // Set s is super-key
                }
                // Found new attribute(s)
                is_new_attrib = 1;
            }
            // Advance iterators
            iter = iter->next;
        }
    }
    
    return 0;
}

// From paper: Candidate Keys for Relations (journal of computer and
// system sciences 1978) by Claudio Lucchesi and Sylvia Osborn.
// Algorithm. Minimal Key (A, D[0], K)
Set candidate_key_from_super_key(Set *skey, const Queue *q, uint8_t n_attribs) {
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
        if (is_superkey(&temp, q, n_attribs)) {
            // Attribute attrib is non-essential to ckey -> remove
            Set_copy(&ckey, &temp);
        }
    }
    return ckey;
}

// From paper: Candidate Keys for Relations (journal of computer and
// system sciences 1978) by Claudio Lucchesi and Sylvia Osborn.
// Algorithm. Set of Minimal Keys (A, D[0])
void print_all_candidate_keys(const Queue *q, uint8_t n_attribs) {
    // Queues for ckeys and work left
    Queue ckeys, work;
    Q_init(&ckeys);
    Q_init(&work);
    
    // Initialize set of all attributes
    Set attribs;
    Set_full(&attribs, n_attribs);
    // Compute first ckey using all attributes
    q_key_t qkey;
    Set ckey = candidate_key_from_super_key(&attribs, q, n_attribs);
    // Print first candidate key
    Set_print(&ckey);
    // Add this ckey as key element of queue to ckeys and work
    // Note: These queues only have a lhs
    qkey = (q_key_t) {.lhs = ckey, .rhs = (Set) {0}};
    Q_insert(&ckeys, qkey);
    Q_insert(&work, qkey);
    // Iterate until no work left (no more candidates to check)
    while (work.size != 0) {
        // Fetch current key from work queue
        const q_key_t key = Q_pop(&work);
        // Iterate over all FDs
        Q_iterator_t iter = Q_iterator(q);
        
        while (iter) {
            // Obtain sets of attributes of individual side of current
            // functional dependency
            const Set s_left  = iter->key.lhs;
            const Set s_right = iter->key.rhs;
            // Compute S
            const Set diff = Set_difference(&key.lhs, &s_right);
            Set S = Set_union(&s_left, &diff);
            // Test for inclusion of any already found candidate key
            uint8_t test = 1;
            // Iterate through all already found candidate keys
            Q_iterator_t ckey_iter = Q_iterator(&ckeys);
            while (ckey_iter) {
                // Only consider lhs
                const Set J = ckey_iter->key.lhs;
                // Check for inclusion
                if (Set_contains(&S, &J)) {
                    test = 0;
                    break;
                }
                // Advance
                ckey_iter = ckey_iter->next;
            }
            
            if (test) {
                // Set S is a super-key and does not contain any already
                // found candidate keys -> compute new candidate key
                ckey = candidate_key_from_super_key(&S, q, n_attribs);
                // Add newly found key to both queues
                qkey = (q_key_t) {.lhs = ckey, .rhs = (Set) {0}};
                Q_insert(&ckeys, qkey);
                Q_insert(&work, qkey);
                // Print candidate key
                Set_print(&ckey);
            }
            // Advance iterators
            iter = iter->next;
        }
    }
    // Print number of candidate keys found
    printf("Number of candidate keys: %u\n", ckeys.size);
    // Cleanup
    Q_free(&ckeys);
    Q_free(&work);
}

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <functional dependecy file>\n", argv[0]);
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
    
    // Parse contents of file
    uint32_t line_num = 0;
    char line_buf[MAX_LINE_LEN];
    
    // Sets for unique vertices on left/right expression sides
    Set s_left, s_right;
    // Queues to store attributes on left/right side of expression
    Queue q;
    Q_init(&q);
    
    // Save pointers (re-entrant)
    char *save_attrib, *save_attrib_list;
    while (fgets(line_buf, MAX_LINE_LEN, fp)) {
        // DEBUG Print current line
        //printf("%u> %s", line_num, line_buf);
        // Make sure current FD is fully contained in buffer
        size_t length = strlen(line_buf);
        assert(length > 0);
        
        if (line_buf[length-1] != '\n') {
            fprintf(stderr, "Error parsing functional Dependency on line %u\n",
                line_num+2);
            exit_on_error(fp, &q);
        }
        // Error code
        int8_t ierr;
        // Search for tokens
        // Parse left-hand side
        char *attrib_list = strtok_r(line_buf, SEP, &save_attrib_list);
        // Check for missing ->
        if (attrib_list == NULL) {
            fprintf(stderr, "Missing '->'\n");
            exit_on_error(fp, &q);
        }
        // Parse left-hand side
        ierr = parse_attrib_list(attrib_list, n_attribs, &save_attrib, &s_left);
        if (ierr) {
            exit_on_error(fp, &q);
        }
        
        // Move to next delimited item
        attrib_list = strtok_r(NULL, SEP, &save_attrib_list);
        // Check for missing right-hand side
        if (attrib_list == NULL) {
            fprintf(stderr, "Right-hand side empty\n");
            exit_on_error(fp, &q);
        }
        // Parse right-hand side
        ierr = parse_attrib_list(attrib_list, n_attribs, &save_attrib, &s_right);
        if (ierr) {
            exit_on_error(fp, &q);
        }
        // Add sets of attributes to queue
        Q_insert(&q, (q_key_t) { .lhs = s_left, .rhs = s_right});
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
    print_all_candidate_keys(&q, n_attribs);
    // Elapsed CPU time
    elapsed = clock() - start;
    const double seconds = (double)elapsed / CLOCKS_PER_SEC;
    printf("Took: %.3e s\n", seconds);
    // Cleanup queue
    Q_free(&q);
    
    return EXIT_SUCCESS;
}
