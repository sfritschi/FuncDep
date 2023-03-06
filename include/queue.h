/*
 * Simple Queue Datastructure (FIFO)
 * 
 */
#pragma once
#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

#include "set.h"

typedef struct q_node_t * Q_iterator_t;

typedef struct {
    Set lhs;
    Set rhs;    
} q_key_t;

// Node within linked list
struct q_node_t {
    struct q_node_t *next;
    q_key_t key;
};

// Linked list data type
typedef struct {
    struct q_node_t *head;  // add elements at head
    struct q_node_t *tail;  // remove elements from tail
    uint32_t size;
} Queue;

// Initialize linked list datastructure
void Q_init(Queue *q);
// Return pointer to node at beginning of queue (tail)
Q_iterator_t Q_iterator(const Queue *q);
// Insert key into linked list (into head)
void Q_insert(Queue *q, q_key_t key);
// Remove first key in linked list (from tail) and return it
q_key_t Q_pop(Queue *q);
// Free all data associated with linked list (start at tail)
void Q_free(Queue *q);

#endif /* QUEUE_H */
