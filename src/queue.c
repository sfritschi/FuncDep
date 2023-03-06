#include "queue.h"
#include "set.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

// Initialize linked list datastructure
void Q_init(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

Q_iterator_t Q_iterator(const Queue *q) {
    return q->tail;
}

// Insert key into linked list (into head)
void Q_insert(Queue *q, q_key_t key) {
    struct q_node_t *new_node = (struct q_node_t *) 
                                malloc(sizeof(struct q_node_t));
    assert(new_node != NULL);
    
    new_node->key = key;
    new_node->next = NULL;
    // Increment size
    ++q->size;
    if (q->head == NULL)  { // first insert, set tail
        q->head = new_node;
        q->tail = new_node;
        return;
    }
    
    q->head->next = new_node;
    q->head = new_node;
}

// Remove first key in linked list (from tail)
q_key_t Q_pop(Queue *q) {
    assert(q->size > 0);
    
    q_key_t key = q->tail->key;
    struct q_node_t *temp = q->tail;
    q->tail = q->tail->next;
    free(temp);
    if (q->tail == NULL) {
        // Head points to invalid memory address at this point
        // Set NULL
        q->head = NULL;
    }
    // Decrement size
    --q->size;
    return key;
}

// Free all data associated with linked list (start at tail)
void Q_free(Queue *q) {
    struct q_node_t *temp;
    while (q->tail != NULL) {
        temp = q->tail;
        q->tail = q->tail->next;
        free(temp);
    }
    // Reset all members
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}
