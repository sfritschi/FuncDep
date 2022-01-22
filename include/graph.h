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

void Graph_free(Graph *graph) {
    // Free adjacency list
    DArray_free(&graph->adjList);
}

#endif /* GRAPH_H */
