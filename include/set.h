#pragma once
#ifndef SET_H
#define SET_H

#include <stdint.h>

// ONLY x86 architectures
#include <popcntintrin.h>  // _mm_popcnt_u32
// TODO: Add support for alternative architectures

#include <assert.h>

#define MAX_ATTRIBS 26
#define INVALID_ATTRIB (MAX_ATTRIBS)
#define USED_MASK 0x03FFFFFF  // mask for bits that are actually used

typedef struct {
    uint32_t set;
    uint8_t size;
    uint8_t cursor;
    uint8_t count;
} Set;

/*
 * Set operations
 * 
 */
// Union
Set Set_union(const Set *s, const Set *t) {
    uint32_t u = s->set | t->set;
    return (Set) {.set = u,
                  .size = _mm_popcnt_u32(u),
                  .cursor = 0,
                  .count = 0};
}

// Intersection
Set Set_intersection(const Set *s, const Set *t) {
    uint32_t i = s->set & t->set;
    return (Set) {.set = i,
                  .size = _mm_popcnt_u32(i),
                  .cursor = 0,
                  .count = 0};
}

// Difference
Set Set_difference(const Set *s, const Set *t) {
    uint32_t d = s->set & ((~t->set) & USED_MASK);
    return (Set) {.set = d,
                  .size = _mm_popcnt_u32(d),
                  .cursor = 0,
                  .count = 0};
}

// Contains (subset)
bool Set_contains(const Set *s, const Set *t) {
    return (s->set & t->set) == t->set;
}

// Initialize set
void Set_init(Set *s) {
    s->set = 0x0;
    s->size = 0;
    s->cursor = 0;
    s->count = 0;
}

// Copy contents from other set
void Set_copy(Set *s, const Set *other) {
    s->set = other->set;
    s->size = other->size;
    s->cursor = 0;
    s->count = 0;
}

// Initialize set to be full
void Set_full(Set *s, uint8_t n_attribs) {
    assert(n_attribs <= MAX_ATTRIBS);
    // Set first n_attribs bits to 1
    s->set = (1 << n_attribs) - 1;
    s->size = n_attribs;
    s->cursor = 0;
    s->count = 0;
}

// Check if set contains all attributes
bool Set_is_full(const Set *s, uint8_t n_attribs) {
    assert(n_attribs <= MAX_ATTRIBS);
    
    return s->size == n_attribs;
}

// Insert attribute in set
void Set_insert(Set *s, uint8_t i) {
    // Check if attribute is valid
    assert(i < MAX_ATTRIBS);
    // Check if attribute is already contained in set
    if ((s->set & (1 << i)))
        return;
    // Set bit position of i-th attribute to 1
    s->set |= (1 << i);
    // Increment set size
    ++s->size;
}

// Delete attribute from set if contained
void Set_remove(Set *s, uint8_t i) {
    // Check if attribute is valid and if it is actually contained in set
    assert(i < MAX_ATTRIBS && (s->set & (1 << i)));
    // Set bit position of i-th attribute to 0 (assuming already set)
    s->set ^= (1 << i);
    // Decrement set size
    --s->size;
}

// Clear all attributes in set
void Set_clear(Set *s) {
    s->set = 0x0;
    s->size = 0;
    s->cursor = 0;
    s->count = 0;
}

// Return next attribute contained in set and maintain current
// search position/count of attributes already visited
//
// Calling Set_insert/Set_remove before Set_next_pos has visited all
// attributes is undefined behavior
uint8_t Set_next_pos(Set *s) {
    // Check if set is non-empty
    assert(s->size > 0);
    
    uint8_t i;
    uint32_t j;
    for (i = s->cursor, j = 1 << i; i < MAX_ATTRIBS; ++i, j <<= 1) {
        // Check if i-th position is set
        if ((s->set & j)) {
            s->cursor = i + 1;
            // Check if last attribute was reached
            if (++s->count == s->size) {
                // Reset cursor position
                s->cursor = 0;
                // Reset count
                s->count = 0;
            }
            return i;
        }
    }
    // Can never be reached...
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

#endif /* SET_H */
