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

/**
 * Grid::Grid
 * @brief Create a grid with a specified length and with a maximum number
 * of items per cell.
 */
Grid::Grid(
    const cl_double4 &grid_length,
    const double cell_length,
    const cl_uint cell_items)
{
    m_length = grid_length;
    m_cells = cl_int3{
        (cl_int) (grid_length.s[0] / cell_length),
        (cl_int) (grid_length.s[1] / cell_length),
        (cl_int) (grid_length.s[2] / cell_length)};
    m_items = cell_items;
    m_capacity = m_items * m_cells.s[0] * m_cells.s[1] * m_cells.s[2];

    // DEBUG
    std::cout << "grid with m_cells "
              << m_cells.s[0] << " "
              << m_cells.s[1] << " "
              << m_cells.s[2] << " "
              << "m_items "
              << m_items << "\n";
}
