/*
 * graph.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef ISING_GRAPH_H_
#define ISING_GRAPH_H_

#include "atto/opencl/opencl.hpp"

/** ---------------------------------------------------------------------------
 * Graph
 * Set of vertices connected by edges in adjacency list represention.
 */
struct Graph {
    /* Graph member variables */
    uint32_t m_n_vertices;
    uint32_t m_n_edges;
    std::vector<std::vector<uint32_t>> m_adj;

    /** Is the specified vertex valid? */
    bool is_valid(const uint32_t v) const { return v < m_n_vertices; }

    /** Return the number of vertices in the graph. */
    uint32_t n_vertices(void) const { return m_n_vertices; }

    /** Return the number of edges in the graph. */
    uint32_t n_edges(void) const { return m_n_edges; }

    /** Return the degree of the specified vertex. */
    uint32_t degree(const uint32_t v) const;

    /** Return the adjacency list of the specified vertex. */
    const std::vector<uint32_t> &adj(const uint32_t v) const;

    /** Add an edge connective vertex v to vertex w. */
    void add_edge(const size_t v, const uint32_t w);

    /** Clear the graph adjacency lists. */
    void clear(void);

    /** Constructor/destructor. */
    Graph();
    Graph(const uint32_t n_vertices);
    ~Graph() = default;
};

/** ---------------------------------------------------------------------------
 * GraphCC
 * Compute the undirected connected components of a graph using breadth first
 * search algorithm.
 */
struct GraphCC {
    /* Vertex state enumerated type. */
    enum : uint32_t {
        NEW = 0,
        VISITED,
    };
    std::vector<uint32_t> m_state;  /* vertex state (NEW or VISITED) */
    std::vector<uint32_t> m_id;     /* component identifier for each vertex */
    uint32_t m_count;               /* number of connected components */

    /** Has the specified vertex been visited? */
    bool is_visited(const uint32_t v) const;

    /** Are verticecs v and w connected? */
    bool connected(const uint32_t v, const uint32_t w) const;

    /** Return the component id for the specified vertex */
    uint32_t id(const uint32_t v) const;

    /** Return the number of connnected components */
    uint32_t count(void) const;

    /** Compute the connected components of the specified graph. */
    void compute(const Graph &graph);

    /** Compute the vertices reachable from a source vertex s. */
    void bfs(const Graph &graph, const uint32_t s);

    /** Constructor/destructor. */
    GraphCC() = default;
    GraphCC(const uint32_t n_vertices);
    ~GraphCC() = default;
};

#endif /* ISING_GRAPH_H_ */
