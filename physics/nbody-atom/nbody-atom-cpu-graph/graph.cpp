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
#include "compute.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * Graph::Graph
 * @brief Create a graph with a n vertices, each with a maximum n neighbours
 * in its adjacency list.
 */
Graph::Graph()
{
    /* Get graph parameters. */
    m_n_vertices = Params::n_atoms;
    m_n_neighbours = Params::n_neighbours;
    m_r_cut = Params::pair_r_cut;
    m_r_skin = Params::pair_r_skin;

    /* Setup all vertices in the adjacency list to empty */
    m_data.resize(m_n_vertices * m_n_neighbours, 0xffffffff);

    /* Setup atom cache positions. */
    m_cache.resize(m_n_vertices, math::vec3d{});
}

/** ---------------------------------------------------------------------------
 * Graph::clear
 * @brief Clear the graph adjaceny lists.
 */
void Graph::clear(void)
{
    core_pragma_omp(parallel for default(none) shared(m_data) schedule(static))
    for (auto &vertex : m_data) {
        vertex = m_empty;
    }
}

/**
 * Graph::compute
 * @brief Compute the adjacency list of the specified atom in the fluid.
 */
void Graph::compute(
    const uint32_t atom_1,
    const std::vector<Atom> &atoms,
    const Domain &domain)
{
    const double radius_sq = (m_r_cut + m_r_skin) * (m_r_cut + m_r_skin);

    uint32_t start = atom_1 * m_n_neighbours;
    uint32_t count = 0;
    for (uint32_t atom_2 = 0; atom_2 < atoms.size(); ++atom_2) {
        if (atom_1 == atom_2) {
            continue;
        }

        math::vec3d r_12 = atoms[atom_1].pos - atoms[atom_2].pos;
        r_12 = compute::pbc(r_12, domain);

        if (math::dot(r_12, r_12) < radius_sq) {
            if (count == m_n_neighbours) {
                core_debug("adjacency list overflow");
                break;
            }
            m_data[start + count++] = atom_2;
        }
    }
}

/**
 * Graph::compute
 * @brief Compute the adjacency list of all the atoms in the fluid.
 * Add an edge connecting any two atoms whose pairwise distance is
 * smaller than the specified radius.
 */
void Graph::compute(const std::vector<Atom> &atoms, const Domain &domain)
{
    /* Clear the graph before computing the adjacency lists. */
    clear();

    /* Compute the adjacency lists. */
    core_pragma_omp(parallel for default(none)
        shared(atoms, domain) schedule(dynamic))
    for (uint32_t atom_ix = 0; atom_ix < atoms.size(); ++atom_ix) {
        compute(atom_ix, atoms, domain);
    }

    /* Cache the atom positions until next update */
    core_pragma_omp(parallel for default(none) \
        shared(m_cache, atoms) schedule(dynamic))
    for (uint32_t atom_ix = 0; atom_ix < atoms.size(); ++atom_ix) {
        m_cache[atom_ix] = atoms[atom_ix].pos;
    }
}

/**
 * Graph::is_stale
 * @brief Is the graph adjacency stale since last update?
 */
bool Graph::is_stale(std::vector<Atom> &atoms)
{
    const double r_half_sq = 0.25 * m_r_skin * m_r_skin;

    for (uint32_t atom_ix = 0; atom_ix < atoms.size(); ++atom_ix) {
        math::vec3d pos = atoms[atom_ix].pos - m_cache[atom_ix];
        if (math::dot(pos, pos) > r_half_sq) {
            return true;
        }
    }
    return false;
}

/** ---------------------------------------------------------------------------
 * Graph::begin
 * @brief Return the first slot in the adjacency list of the specified atom.
 * If the atom has no neighbours in its adjacency list, return end() to notify
 * we are at the end of the atom adjacency list.
 */
uint32_t Graph::begin(const uint32_t atom_ix) const
{
    uint32_t slot = atom_ix * m_n_neighbours;
    if (get(slot) == end()) {
        return end();
    }
    return slot;
}

/**
 * Graph::next
 * @brief Return the next slot in the adjacency list of the specfified atom.
 * Iteration over neighbours of a specified atom is of the following form:
 *
 *  input: index of an atom
 *  output: list of all the neighbour vertices adjacent to the atom
 *
 *  slot = begin(atom)
 *  while (slot != end()) {
 *      neighbour = get(slot)
 *      slot = next(slot)
 *  }
 *
 * The adjacency list of an atom has at most n neighbours vertices.
 * Assuming slot started at the initial position in the atom adjacency list,
 *  slot = (atom * n_neighbours) => (slot % n_neighbours) = 0.
 * The next time the modulo operation (slot % n_neighbours) is zero is when
 *  slot = ((atom + 1) * n_neighbours),
 * i.e., at the start of next atom's adjancency list.
 * Return end() to notify we reached the end of the atom adjacency list.
 */
uint32_t Graph::next(uint32_t slot) const
{
    slot++;
    if (slot % m_n_neighbours == 0) {
        return end();
    }
    if (get(slot) == end()) {
        return end();
    }
    return slot;
}

/**
 * Graph::get
 * @brief Return the vertex index at the specified slot in the graph.
 */
uint32_t Graph::get(const uint32_t slot) const
{
    if (slot < m_data.size()) {
        return m_data[slot];
    }
    return end();
}

/**
 * Graph::neighbours
 * @brief Return neighbour adjacency list of the specified atom.
 */
std::vector<uint32_t> Graph::neighbours(const uint32_t atom_ix) const
{
    std::vector<uint32_t> adj;
    uint32_t slot = begin(atom_ix);
    while (slot != end()) {
        adj.push_back(get(slot));       /* Get slot vertex. */
        slot = next(slot);              /* Next vertex slot. */
    }
    return adj;
}
