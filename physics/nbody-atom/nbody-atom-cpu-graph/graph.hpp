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

#ifndef MD_GRAPH_H_
#define MD_GRAPH_H_

#include "atto/opencl/opencl.hpp"
#include "base.hpp"

/**
 * Graph
 * @brief Graph maintains an adjacency list representing the graph of atoms
 * whose pairwise distance is smaller than a specified radius.
 *
 * Each vertex in the graph represents an atom. The data vector is an array
 * of contiguous adjacency lists for each atom, each with a specfied number
 * of neighbours. The index k in the array of neighbour j in atom i is given
 * by the linear relation k = i*n_neighs + j, 0 <= i < n_vertices.
 */
struct Graph {
    /* State flag indicating an empty slot. */
    static const uint32_t m_empty = 0xffffffff;

    /* Graph member variables. */
    uint32_t m_n_vertices;              /* number of vertices in the graph */
    uint32_t m_n_neighbours;            /* neighbour edges per vertex */
    double m_r_cut;                     /* edge cutoff radius */
    double m_r_skin;                    /* edge skin radius */
    std::vector<uint32_t> m_data;       /* adjacency lists of each vertex */
    std::vector<atto::math::vec3d> m_cache; /* atom cache positions */

    /** Clear the graph adjaceny lists. */
    void clear(void);

    /** Compute the adjacency list of the specified atom in the fluid. */
    void compute(
        const uint32_t atom_1,
        const std::vector<Atom> &atoms,
        const Domain &domain);

    /** Compute the adjacency list of all the atoms in the fluid. */
    void compute(const std::vector<Atom> &atoms, const Domain &domain);

    /** Is the graph adjacency stale since last update? */
    bool is_stale(std::vector<Atom> &atoms);

    /** Return the first slot in the adjacency list of the specified atom. */
    uint32_t begin(const uint32_t atom_ix) const;

    /** Return the past-the-end value indicating an empty slot. */
    uint32_t end(void) const { return m_empty; }

    /** Return the next slot in the adjacency list of the specfified atom. */
    uint32_t next(uint32_t slot) const;

    /** Return the vertex index at the specified slot in the graph. */
    uint32_t get(const uint32_t slot) const;

    /** Return neighbour adjacency list of the specified atom. */
    std::vector<uint32_t> neighbours(const uint32_t atom_ix) const;

    /* Constructor/destructor. */
    Graph();
    ~Graph() = default;
};

#endif /* MD_GRAPH_H_ */
