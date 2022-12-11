/*
 * graph.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "atto/opencl/opencl.hpp"
#include "graph.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * Graph::Graph
 * @brief Graph constructor.
 */
Graph::Graph()
    : m_n_vertices(0)
    , m_n_edges(0)
{}

Graph::Graph(const uint32_t n_vertices)
    : m_n_vertices(n_vertices)
    , m_n_edges(0)
{
    m_adj.resize(n_vertices);
}

/**
 * Graph::degree
 * @brief Return the degree of the specified vertex.
 */
uint32_t Graph::degree(const uint32_t v) const
{
    core_assert(is_valid(v), "invalid vertex");
    return m_adj[v].size();
}

/**
 * Graph::adj
 * @brief Return the adjacency list of the specified vertex.
 */
const std::vector<uint32_t> &Graph::adj(const uint32_t v) const
{
    core_assert(is_valid(v), "invalid vertex");
    return m_adj[v];
}

/**
 * Graph::add_edge
 * @brief Add an edge connective vertex v to vertex w.
 */
void Graph::add_edge(const size_t v, const uint32_t w)
{
    core_assert(is_valid(v) && is_valid(w), "invalid vertices");
    m_adj[v].push_back(w);
    m_adj[w].push_back(v);
    m_n_edges++;
}

/**
 * Graph::clear
 * @brief Clear the graph adjacency lists.
 */
void Graph::clear(void)
{
    for (auto &it : m_adj) {
        it.clear();
    }
    m_n_edges = 0;
}


/** ---------------------------------------------------------------------------
 * GraphCC::GraphCC
 * Create a connected component object with n vertices.
 */
GraphCC::GraphCC(const uint32_t n_vertices)
{
    m_state.resize(n_vertices, NEW);
    m_id.resize(n_vertices, (uint32_t) -1);
    m_count = 0;
}

/**
 * GraphCC::is_visited
 * @brief Has the specified vertex been visited?
 */
bool GraphCC::is_visited(const uint32_t v) const
{
    return m_state[v] == VISITED;
}

/**
 * GraphCC::connected
 * @brief Are verticecs v and w connected?
 */
bool GraphCC::connected(const uint32_t v, const uint32_t w) const
{
    return id(v) == id(w);
}

/**
 * GraphCC::id
 * @brief Return the component id for the specified vertex
 */
uint32_t GraphCC::id(const uint32_t v) const
{
    return m_id[v];
}

/**
 * GraphCC::count
 * @brief Return the number of connnected components
 */
uint32_t GraphCC::count(void) const
{
    return m_count;
}

/**
 * GraphCC::compute
 * @brief Compute the connected components of the specified graph.
 */
void GraphCC::compute(const Graph &graph)
{
    std::fill(m_state.begin(), m_state.end(), NEW);
    std::fill(m_id.begin(), m_id.end(), (uint32_t) - 1);
    m_count = 0;

    for (uint32_t s = 0; s < graph.n_vertices(); ++s) {
        if (m_state[s] == NEW) {
            bfs(graph, s);
            m_count++;
        }
    }
}

/**
 * GraphCC::bfs
 * @brief Compute the vertices reachable from a source vertex s and assign
 * the associated connected component identifier.
 */
void GraphCC::bfs(const Graph &graph, const uint32_t s)
{
    std::queue<uint32_t> queue;
    m_state[s] = VISITED;
    m_id[s] = m_count;
    queue.push(s);
    while (!queue.empty()) {
        uint32_t v = queue.front();
        queue.pop();
        for (auto &w : graph.adj(v)) {
            if (m_state[w] == NEW) {
                m_state[w] = VISITED;
                m_id[w] = m_count;
                queue.push(w);
            }
        }
    }
}
