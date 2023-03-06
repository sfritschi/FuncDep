#pragma once
#ifndef SET_H
#define SET_H

#include <stdint.h>

#define MAX_ATTRIBS 26u
#define INVALID_ATTRIB (MAX_ATTRIBS)
#define USED_MASK 0x03FFFFFFu  // mask for bits that are actually used

typedef struct {
    uint32_t set;
    uint8_t size;
    uint8_t cursor;
    uint8_t count;
} Set;

// Set operations
Set Set_union(const Set *s, const Set *t);
Set Set_intersection(const Set *s, const Set *t);
Set Set_difference(const Set *s, const Set *t);
uint8_t Set_contains(const Set *s, const Set *t);

// Initialize set
void Set_init(Set *s);
// Copy contents from other set
void Set_copy(Set *s, const Set *other);

// Initialize set to be full
void Set_full(Set *s, uint8_t n_attribs);
// Check if set contains all attributes
uint8_t Set_is_full(const Set *s, uint8_t n_attribs);
// Insert attribute in set
void Set_insert(Set *s, uint8_t i);
// Delete attribute from set if contained
void Set_remove(Set *s, uint8_t i);
// Clear all attributes in set
void Set_clear(Set *s);
// Return next attribute contained in set and maintain current
// search position/count of attributes already visited
uint8_t Set_next_pos(Set *s);
// Print all attributes belonging to set as characters of alphabet
void Set_print(Set *s);

#endif /* SET_H */
