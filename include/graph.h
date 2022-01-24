/*
 * Simple Graph Datastructure using Adjacency List Representation 
 * 
 * Depends on:
 * - Linked List datastructure (FIFO)
 */

#pragma once
#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "linked_list.h"
#include "darray.h"

typedef struct {
    DArray adjList;
    uint32_t n_vert;
    uint32_t n_edge;
} Graph;

void Graph_init(Graph *graph, uint32_t n_vert) {
    // Initialize all fields
    DArray_init_reserve(&graph->adjList, n_vert);
    graph->n_vert = n_vert;
    graph->n_edge = 0;
}

void Graph_add_edge(Graph *graph, uint32_t i, uint32_t j) {
    uint32_t size = DArray_size(&graph->adjList);
    // DEBUG
    assert(graph->n_vert == size);
    // Make sure vertices are valid
    assert(i < graph->n_vert && j < graph->n_vert);
    // Add vertices to adjList
    LinkedList *ll;
    ll = DArray_get(&graph->adjList, i);
    // Insert edge from i -> j
    LL_insert(ll, j);
    // Update number of edges
    ++graph->n_edge;
}

uint32_t Graph_new_vertex(Graph *graph) {
    // Insert new vertex
    DArray_insert(&graph->adjList);
    // Update number of vertices and return index of new vertex
    return graph->n_vert++;
}

void Graph_BFS_closure(const Graph *graph, uint32_t source, 
    uint32_t *visited_buf, const uint32_t *visited_thresh) {
        
    assert(source < graph->n_vert);
    // Check if source vertex was visited previously
    if (visited_buf[source] == 1) {
        // nothing left to do
        return;
    }
    // Search list containing vertices to explore
    LinkedList search_list;
    LL_init(&search_list);
    // Initialize visited buf for source
    visited_buf[source] = 1;
    // Add source to search list
    LL_insert(&search_list, source);
    
    while (LL_size(&search_list) != 0) {
        uint32_t current = LL_pop(&search_list);
        assert(current != LL_INVALID_KEY);
        
        // Get iterator for adjacency list
        const LinkedList *ll = DArray_get(&graph->adjList, current);
        LL_iterator_t iter = LL_iterator(ll);
        
        while (iter != NULL) {
            uint32_t neighbor = iter->key;
            // Get threshold for current neighbor
            const uint32_t thresh = visited_thresh[neighbor];
            // If threshold was already reached previously, move on
            // to next neighbor
            if (visited_buf[neighbor] == thresh) {
                iter = iter->next;
                continue;
            }
            // Increment visited buffer entry for neighbor
            visited_buf[neighbor]++;
            // Check if threshold has been reached
            if (visited_buf[neighbor] == thresh) {
                // Add this neighbor to search list
                LL_insert(&search_list, neighbor);
            }
            // Move on to next neighbor
            iter = iter->next;
        }
    }
    // Cleanup
    LL_free(&search_list);    
}

void Graph_free(Graph *graph) {
    // Free adjacency list
    DArray_free(&graph->adjList);
}

#endif /* GRAPH_H */
