/*
 * Simple Graph Datastructure using Adjacency List Representation 
 * 
 * Supports: (for both directed and undirected UNWEIGHTED graphs)
 * - BFS shortest paths for single vertex
 * - BFS shortest paths for all pairs of vertices using OMP
 * - DFS reachability for source target pair and fixed distance
 * - DFS Hash Map containing all (unique) vertices reachable from source
 *   using fixed number of steps
 * 
 * Depends on:
 * - Linked List datastructure (FIFO)
 * - Hash Map datastructure
 */

#pragma once
#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <omp.h>

#include "linked_list.h"
#include "hashmap.h"
#include "darray.h"

typedef struct {
    DArray adjList;
    unsigned int n_vert;
    unsigned int n_edge;
} Graph;

void Graph_init(Graph *graph, unsigned int n_vert) {
    // Initialize all fields
    DArray_init_reserve(&graph->adjList, n_vert);
    graph->n_vert = n_vert;
    graph->n_edge = 0;
}

void Graph_add_edge(Graph *graph, unsigned int i, unsigned int j) {
    unsigned int size = DArray_size(&graph->adjList);
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

unsigned int Graph_new_vertex(Graph *graph) {
    // Insert new vertex
    DArray_insert(&graph->adjList);
    // Update number of vertices and return index of new vertex
    return graph->n_vert++;
}

void Graph_BFS_closure(const Graph *graph, unsigned int source, 
    unsigned int **visited_buf, const unsigned int *visited_thresh) {
        
    assert(source < graph->n_vert);
    // Check if source vertex was visited previously
    if ((*visited_buf)[source] == 1) {
        // nothing left to do
        return;
    }
    // Search list containing vertices to explore
    LinkedList search_list;
    LL_init(&search_list);
    // Initialize visited buf for source
    (*visited_buf)[source] = 1;
    // Add source to search list
    LL_insert(&search_list, source);
    
    while (LL_size(&search_list) != 0) {
        unsigned int current = LL_pop(&search_list);
        assert(current != LL_INVALID_KEY);
        
        // Get iterator for adjacency list
        LinkedList *ll = DArray_get(&graph->adjList, current);
        LL_iterator_t iter = LL_iterator(ll);
        
        while (iter != NULL) {
            unsigned int neighbor = iter->key;
            // Get threshold for current neighbor
            const unsigned int thresh = visited_thresh[neighbor];
            // If threshold was already reached previously, move on
            // to next neighbor
            if ((*visited_buf)[neighbor] == thresh) {
                iter = iter->next;
                continue;
            }
            // Increment visited buffer entry for neighbor
            (*visited_buf)[neighbor]++;
            // Check if threshold has been reached
            if ((*visited_buf)[neighbor] == thresh) {
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
