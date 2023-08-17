#include "set.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Set operations
 * 
 */
// Union
Set Set_union(const Set *s, const Set *t) {
    const uint32_t u = s->set | t->set;
    return (Set) { .set    = u, 
                   .size   = __builtin_popcount(u),
                   .cursor = 0,
                   .count  = 0 };
}

// Intersection
Set Set_intersection(const Set *s, const Set *t) {
    const uint32_t i = s->set & t->set;
    return (Set) { .set    = i, 
                   .size   = __builtin_popcount(i),
                   .cursor = 0,
                   .count  = 0 };
}

// Difference
Set Set_difference(const Set *s, const Set *t) {
    const uint32_t d = s->set & (~t->set);
    return (Set) { .set    = d, 
                   .size   = __builtin_popcount(d),
                   .cursor = 0,
                   .count  = 0 };
}

// Contains (subset)
uint8_t Set_contains(const Set *s, const Set *t) {
    return (s->set & t->set) == t->set;
}

// Initialize set
void Set_init(Set *s) {
    s->set    = 0x0;
    s->size   = 0;
    s->cursor = 0;
    s->count  = 0;
}

// Copy contents from other set
void Set_copy(Set *s, const Set *other) {
    s->set    = other->set;
    s->size   = other->size;
    s->cursor = 0;
    s->count  = 0;
}

// Initialize set to be full
void Set_full(Set *s, uint8_t n_attribs) {
    assert(n_attribs <= MAX_ATTRIBS);
    // Set first n_attribs bits to 1
    s->set    = (1u << n_attribs) - 1;
    s->size   = n_attribs;
    s->cursor = 0;
    s->count  = 0;
}

// Check if set contains all attributes
uint8_t Set_is_full(const Set *s, uint8_t n_attribs) {
    assert(n_attribs <= MAX_ATTRIBS);
    
    return s->size == n_attribs;
}

// Insert attribute in set
void Set_insert(Set *s, uint8_t i) {
    // Check if attribute is valid
    assert(i < MAX_ATTRIBS && "Tried to insert invalid attribute");
    assert(s->cursor == 0 && "Tried to insert attribute while iterating");
    // Check if attribute is already contained in set
    if ((s->set & (1u << i)))
        return;  // do nothing
    // Set bit position of i-th attribute to 1
    s->set |= (1u << i);
    // Increment set size
    ++s->size;
}

// Delete attribute from set if contained
void Set_remove(Set *s, uint8_t i) {
    // Check if attribute is valid and if it is actually contained in set
    assert(i < MAX_ATTRIBS && "Tried to remove invalid attribute");
    assert(s->set & (1u << i) && "Tried to remove attribute that is not contained");
    assert(s->cursor == 0 && "Tried to remove attribute while iterating");
    // Set bit position of i-th attribute to 0 (assuming already set)
    s->set ^= (1u << i);
    // Decrement set size
    --s->size;
}

// Clear all attributes in set
void Set_clear(Set *s) {
    s->set    = 0x0;
    s->size   = 0;
    s->cursor = 0;
    s->count  = 0;
}

// Return next attribute contained in set and maintain current
// search position/count of attributes already visited
uint8_t Set_next_pos(Set *s) {
    uint8_t i;
    for (i = s->cursor; i < MAX_ATTRIBS; ++i) {
        // Check if i-th position is set
        if ((s->set & (1u << i))) {
            // Advance cursor
            s->cursor = i + 1;
            // Check if last attribute was reached
            if (++s->count == s->size) {
                // Reset cursor position
                s->cursor = 0;
                // Reset count
                s->count = 0;
            }
            // Return found element of set
            return i;
        }
    }
    // Set is empty
    return INVALID_ATTRIB;
}

// Print all attributes belonging to set
void Set_print(Set *s) {
    uint8_t i, pos;
    for (i = 0; i < s->size; ++i) {
        pos = Set_next_pos(s);
        printf("%c ", (char)(pos + 'A'));
    }
    printf("\n");
}
