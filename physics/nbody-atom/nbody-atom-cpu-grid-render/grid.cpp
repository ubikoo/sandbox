/*
 * grid.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "atto/opencl/opencl.hpp"
#include "grid.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * Grid::Grid
 * @brief Create a grid with n cells and a maximum number of items per cell.
 */
Grid::Grid(
    const math::vec3d &length,
    const double cell_length,
    const uint32_t cell_items)
{
    m_length = length;
    m_cells = math::vec3i(
        (int32_t) (length.x / cell_length),
        (int32_t) (length.y / cell_length),
        (int32_t) (length.z / cell_length));
    m_cell_items = cell_items;
    m_capacity = m_cell_items * m_cells.x * m_cells.y * m_cells.z;

    m_n_items = 0;
    m_data.resize(m_capacity, Item{m_empty, m_empty});

    // DEBUG
    std::cout << "grid with cells " << math::to_string(m_cells)
              << ", an cell_items " << m_cell_items << "\n";
}

/** ---------------------------------------------------------------------------
 * Grid::clear
 * @brief Clear all key-value pairs in the grid and reset their count.
 */
void Grid::clear(void)
{
    m_n_items = 0;
    for (auto &item : m_data) {
        item = Item{m_empty, m_empty};
    }
}

/**
 * Grid::compare_and_swap
 * @brief Compare key with old value and swap with new value.
 * Return the value previously held by the key.
 */
uint32_t Grid::compare_and_swap(
    uint32_t &key,
    const uint32_t oldval,
    const uint32_t newval)
{
    uint32_t prev = key;
    if (key == oldval) {
        key = newval;
    }
    return prev;
}

/**
 * Grid::insert
 * @brief Insert a new key-value pair into the grid. Start at the slot given
 * by key hash. Iterate until a non-empty slot is found and set the value.
 */
void Grid::insert(const uint32_t key, const uint32_t value)
{
    /* Do nothing if key is empty */
    if (key == m_empty) {
        return;
    }

    uint32_t slot = key % m_capacity;     // & (m_capacity - 1)
    while (true) {
        uint32_t prev = compare_and_swap(m_data[slot].key, m_empty, key);

        if (prev == m_empty) {
            m_n_items++;
            m_data[slot].value = value;
            return;
        }

        slot = (slot + 1) % m_capacity;   // & (m_capacity - 1)
    }
}

/**
 * Grid::insert
 * @brief Insert the specified atom positions into the grid.
 */
void Grid::insert(const std::vector<Atom> &atoms)
{
    /* Clear the grid before insertion. */
    clear();

    /* Insert each atom in grid. */
    for (uint32_t atom_ix = 0; atom_ix < atoms.size(); ++atom_ix) {
        uint32_t key = hash(cell(atoms[atom_ix].pos));

        /* Invalid hash key. */
        if (key == end()) {
            std::ostringstream ss;
            ss << "invalid hash key " << key << "\n";
            ss << "pos  " << math::to_string(atoms[atom_ix].pos) << "\n";
            ss << "cell " << math::to_string(cell(atoms[atom_ix].pos)) << "\n";
            core_debug(ss.str());
        }

        insert(key, atom_ix);
    }
}

/** ---------------------------------------------------------------------------
 * Grid::begin
 * @brief Return the first slot containing the specified key. If no such key
 * exists, return empty signalling no further slots contain the specified key.
 */
uint32_t Grid::begin(const uint32_t key) const
{
    /* Do nothing if key is empty */
    if (key == m_empty) {
        return m_empty;
    }

    uint32_t slot = key % m_capacity;     // & (m_capacity - 1)
    while (true) {
        if (m_data[slot].key == key) {
            return slot;
        }

        if (m_data[slot].key == m_empty) {
            return m_empty;
        }

        slot = (slot + 1) % m_capacity;   // & (m_capacity - 1)
    }
}

/**
 * Grid::next
 * @brief Return the next slot containing the specified key.
 * Return empty if we reached the end of the list.
 */
uint32_t Grid::next(const uint32_t key, uint32_t slot) const
{
    while (true) {
        slot = (slot + 1) % m_capacity;   // & (m_capacity - 1)

        if (m_data[slot].key == key) {
            return slot;
        }

        if (m_data[slot].key == m_empty) {
            return m_empty;
        }
    }
}

/** ---------------------------------------------------------------------------
 * Grid::cell
 * @brief Return the cell coordinates containing the specified position.
 * The position is assumed to be in the range of (-period/2, period/2):
 *  u_pos = 0.5 + pos / period.
 */
math::vec3i Grid::cell(const math::vec3d &pos) const
{
    /* Get the position in normalized coordinates. */
    math::vec3d u_pos(0.5);
    u_pos += pos / m_length;

    return math::vec3i(
        (int32_t) (u_pos.x * m_cells.x),
        (int32_t) (u_pos.y * m_cells.y),
        (int32_t) (u_pos.z * m_cells.z));
}

/**
 * Grid::hash
 * @brief Compute the hash key of the cell coordinates.
 * Return the key if the cell is inside the grid range. Return empty otherwise.
 */
uint32_t Grid::hash(const math::vec3i cell_coord) const
{
    if (cell_coord.x < 0 || cell_coord.x >= m_cells.x ||
        cell_coord.y < 0 || cell_coord.y >= m_cells.y ||
        cell_coord.z < 0 || cell_coord.z >= m_cells.z ) {
        return m_empty;
    }

    uint32_t slot = m_cell_items * (cell_coord.x * m_cells.y * m_cells.z +
                                    cell_coord.y * m_cells.z +
                                    cell_coord.z);
    return slot % m_capacity;
}

/** ---------------------------------------------------------------------------
 * Grid::pbc
 * @brief Return the periodic image of the specfied cell.
 */
math::vec3i Grid::pbc(const math::vec3i &cell_coord) const
{
    math::vec3i cell_image(cell_coord);

    if (cell_image.x < 0) {
        cell_image.x += m_cells.x;
    }
    if (cell_image.x >= m_cells.x) {
        cell_image.x -= m_cells.x;
    }

    if (cell_image.y < 0) {
        cell_image.y += m_cells.y;
    }
    if (cell_image.y >= m_cells.y) {
        cell_image.y -= m_cells.y;
    }

    if (cell_image.z < 0) {
        cell_image.z += m_cells.z;
    }
    if (cell_image.z >= m_cells.z) {
        cell_image.z -= m_cells.z;
    }

    return cell_image;
}

/**
 * Grid::neighbours
 * @brief Return the neighbours centred around the specified cell.
 */
std::array<math::vec3i,27> Grid::neighbours(const math::vec3i &cell_coord) const
{
    std::array<math::vec3i,27> neighbour_cells;

    uint32_t count = 0;
    for (int32_t ix = cell_coord.x - 1; ix <= cell_coord.x + 1; ++ix) {
        for (int32_t iy = cell_coord.y - 1; iy <= cell_coord.y + 1; ++iy) {
            for (int32_t iz = cell_coord.z - 1; iz <= cell_coord.z + 1; ++iz) {
                neighbour_cells[count++] = pbc(math::vec3i(ix, iy, iz));
            }
        }
    }

    return neighbour_cells;
}

/**
 * Grid::neighbours
 * @brief Compute neighbours of the atom with the specified index. For each
 * neighbour cell of the atom's primary cell, store the atom indices contained
 * in the cell list.
 */
std::vector<uint32_t> Grid::neighbours(
    const uint32_t atom_1,
    const std::vector<Atom> &atoms) const
{
    std::vector<uint32_t> adj;
    math::vec3i cell_1 = cell(atoms[atom_1].pos);
    for (auto &cell_2 : neighbours(cell_1)) {
        uint32_t key = hash(cell_2);
        uint32_t slot = begin(key);
        while (slot != end()) {
            uint32_t atom_2 = get(slot);

            if (atom_2 != atom_1) {
                adj.push_back(atom_2);
            }

            slot = next(key, slot);
        }
    }
    return adj;
}
