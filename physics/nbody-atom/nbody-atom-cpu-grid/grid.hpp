/*
 * grid.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef MD_GRID_H_
#define MD_GRID_H_

#include "atto/opencl/opencl.hpp"
#include "base.hpp"

/**
 * Grid
 * @brief Grid represents a 3-dimensional grid of cells with a load factor
 * corresponding to n atoms per cell.
 *
 * The underlying data structure is an array of key-value pairs using open
 * addressing with linear probing to resolve collisions.
 *
 * The key is a hash representation of the cell where the atom belongs to,
 * with a value holding the atom index.
 * Two atoms belonging to the same cell have equal keys. The collision is
 * resolved using linear probing. The table is searched linearly for the
 * closest free location and the new key is inserted there with a value
 * given by the index of the second atom.
 */
struct Grid {
    /* Item data type holding a key-value pair. */
    struct Item {
        uint32_t key;
        uint32_t value;
    };

    /* Capacity range as a power of two */
    static const uint32_t m_min_size = 1 << 3;      /* 8 items */
    static const uint32_t m_max_size = 1 << 31;     /* 2147483648 items */

    /* State flag indicating an empty key. */
    static const uint32_t m_empty = 0xffffffff;

    /* Member variables. */
    atto::math::vec3d m_length;     /* grid length in each direction */
    atto::math::vec3i m_cells;      /* number of cells in each direction */
    uint32_t m_cell_items;          /* max number of items per cell */
    uint32_t m_capacity;            /* max number of items in the table */
    uint32_t m_n_items;             /* current number of items in the table */
    std::vector<Item> m_data;       /* grid key-value pairs */

    /** Clear all key-value pairs in the grid and reset their count. */
    void clear(void);

    /** Compare key with old value and swap with new value. */
    uint32_t compare_and_swap(
        uint32_t &key,
        const uint32_t oldval,
        const uint32_t newval);

    /** Insert a new key-value pair into the grid. */
    void insert(const uint32_t key, const uint32_t value);

    /** Insert the specified atom positions into the grid. */
    void insert(const std::vector<Atom> &atoms);

    /** Return the first slot containing the specified key. */
    uint32_t begin(const uint32_t key) const;

    /** Return the past-the-end value indicating an empty slot. */
    uint32_t end(void) const { return m_empty; }

    /** Return the next slot containing the specified key. */
    uint32_t next(const uint32_t key, uint32_t slot) const;

    /** Return the value of the current slot. */
    uint32_t get(const uint32_t slot) const { return m_data[slot].value; }

    /** Return the cell coordinates containing the specified position. */
    atto::math::vec3i cell(const atto::math::vec3d &pos) const;

    /** Compute the hash key of the cell coordinates. */
    uint32_t hash(const atto::math::vec3i cell_coord) const;

    /** Return the periodic image of the specfied cell. */
    atto::math::vec3i pbc(const atto::math::vec3i &cell_coord) const;

    /** Return the neighbours centred around the specified cell. */
    std::array<atto::math::vec3i,27> neighbours(
        const atto::math::vec3i &cell_coord) const;

    /** Compute neighbours of the atom with the specified index. */
    std::vector<uint32_t> neighbours(
        const uint32_t atom_1,
        const std::vector<Atom> &atoms) const;

    /* Constructor/destructor. */
    Grid() = default;
    Grid(const atto::math::vec3d &length,
        const double cell_length,
        const uint32_t cell_items);
    ~Grid() = default;
};

#endif /* MD_GRID_H_ */

