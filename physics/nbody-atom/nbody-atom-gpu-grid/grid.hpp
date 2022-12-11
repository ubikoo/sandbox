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
        cl_uint key;
        cl_uint value;
    };

    /* Capacity range as a power of two */
    static const cl_uint m_min_size = 1 << 3;      /* 8 items */
    static const cl_uint m_max_size = 1 << 31;     /* 2147483648 items */

    /* State flag indicating an empty key. */
    static const cl_uint m_empty = 0xffffffff;

    /* Grid member variables. */
    cl_double4 m_length;        /* grid length in each direction */
    cl_int4 m_cells;            /* number of cells in each direction */
    cl_uint m_items;            /* max number of items per cell */
    cl_uint m_capacity;         /* max number of items in the table */

    /* Constructor/destructor. */
    Grid() = default;
    Grid(const cl_double4 &grid_length,
         const double cell_length,
         const cl_uint cell_items);
    ~Grid() = default;
};

#endif /* MD_GRID_H_ */
