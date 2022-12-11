/*
 * generate.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "atto/opencl/opencl.hpp"
#include "generate.hpp"
using namespace atto;

namespace generate {

/** ---------------------------------------------------------------------------
 * points_random
 * @brief Create a set of points uniformly distributed inside a box with
 * the specified dimensions.
 */
std::vector<cl_float4> points_random(
    const cl_ulong n_points,
    const cl_float xlo,
    const cl_float ylo,
    const cl_float zlo,
    const cl_float xhi,
    const cl_float yhi,
    const cl_float zhi)
{
    core_assert(n_points > 0, "invalid number of points");
    core_assert(xlo < xhi && ylo < yhi && zlo < zhi, "invalid range");

    math::rng::Kiss engine(true);               /* rng engine */
    math::rng::uniform<cl_float> rand;          /* rng sampler */

    std::vector<cl_float4> points(n_points);
    for (auto &it : points) {
        it = cl_float4{
            rand(engine, xlo, xhi),
            rand(engine, ylo, yhi),
            rand(engine, zlo, zhi),
            0.0 /*unused*/};
    }
    return points;
}

/**
 * points_cubic
 * @brief Create a collection of points inside a simple cubic lattice with
 * the specified dimensions.
 */
std::vector<cl_float4> points_cubic(
    const cl_ulong n_points,
    const cl_float xlo,
    const cl_float ylo,
    const cl_float zlo,
    const cl_float xhi,
    const cl_float yhi,
    const cl_float zhi)
{
    core_assert(n_points > 0, "invalid number of points");
    core_assert(xlo < xhi && ylo < yhi && zlo < zhi, "invalid range");

    /*
     * Compute the minimum number of lattice cells able to accomodate the
     * specified number of points. The unit cell of an ssc lattice contains
     * a single site located at
     *      r1 = (0, 0, 0)
     * A scc lattice with unit length has a cell size determined by the
     * total number of cells along each dimension a = 1.0 / n_cells.
     */
    cl_ulong n_cells = 0;       /* number of unit cells in the lattice */
    cl_ulong n_sites = 0;       /* number of sites in the lattice */
    while (n_sites < n_points) {
        ++n_cells;
        n_sites = n_cells * n_cells * n_cells;
    }

    /* Specify the unit cell basis vectors. */
    const cl_float4 cell = cl_float4{
        (xhi - xlo) / (cl_float) n_cells,
        (yhi - ylo) / (cl_float) n_cells,
        (zhi - zlo) / (cl_float) n_cells,
        0.0 /*unused*/};

    /*
     * Construct the fcc cubic lattice. Create an array of boolean values
     * for each existing site. Shuffle the array using Knuth's algorithm.
     */
    std::vector<bool> is_valid(n_sites, false); /* is site valid? */
    {
        math::rng::Kiss engine(true);           /* rng engine */
        math::rng::uniform<uint64_t> rand;      /* rng sampler */

        for (cl_ulong i = 0; i < n_points; ++i) {
            is_valid[i] = true;
        }

        for (cl_ulong i = 0; i < n_sites-1; ++i) {
            cl_ulong j = rand(engine, 0, n_sites-i);
            std::swap(is_valid[i], is_valid[j]);
        }
    }

    std::vector<cl_float4> points;             /* lattice site positions */
    {
        cl_ulong site_ix = 0;
        for (cl_ulong i = 0; i < n_cells; i++) {
            for (cl_ulong j = 0; j < n_cells; j++) {
                for (cl_ulong k = 0; k < n_cells; k++) {
                    /* Store particle position */
                    if (is_valid[site_ix]) {
                        cl_float4 pos{
                            (cl_float) i * cell.s[0],
                            (cl_float) j * cell.s[1],
                            (cl_float) k * cell.s[2]};
                        points.push_back(pos);
                    }
                    site_ix++;  /* next lattice site */
                }
            }
        }
    }
    return points;
}

/**
 * points_fcc
 * @brief Create a collection of points inside a face centred cubic lattice
 * with specified dimensions.
 */
std::vector<cl_float4> points_fcc(
    const cl_ulong n_points,
    const cl_float xlo,
    const cl_float ylo,
    const cl_float zlo,
    const cl_float xhi,
    const cl_float yhi,
    const cl_float zhi)
{
    core_assert(n_points > 0, "invalid number of points");
    core_assert(xlo < xhi && ylo < yhi && zlo < zhi, "invalid range");

    /*
     * Compute the minimum number of lattice cells able to accomodate the
     * specified number of points. The unit cell of an fcc lattice contains
     * four sites located at:
     *      r1 = (0,     0,   0)
     *      r2 = (0,   a/2, a/2)
     *      r3 = (a/2,   0, a/2)
     *      r4 = (a/2, a/2,   0)
     * where a is the size of the unit cell.
     * A fcc lattice with unit length has a cell size determined by the
     * total number of cells along each dimension a = 1.0 / n_cells.
     */
    cl_ulong n_cells = 0;       /* number of unit cells in the lattice */
    cl_ulong n_sites = 0;       /* number of sites in the lattice */
    while (n_sites < n_points) {
        ++n_cells;
        n_sites = 4 * n_cells * n_cells * n_cells;
    }

    /* Specify the unit cell basis vectors. */
    const cl_float4 cell = cl_float4{
        (xhi - xlo) / (cl_float) n_cells,
        (yhi - ylo) / (cl_float) n_cells,
        (zhi - zlo) / (cl_float) n_cells,
        0.0 /*unused*/};

    std::array<cl_float4,4> basis{
        cl_float4{            0.0,            0.0,            0.0, 0.0 /*unused*/},
        cl_float4{            0.0, 0.5f*cell.s[1], 0.5f*cell.s[2], 0.0 /*unused*/},
        cl_float4{ 0.5f*cell.s[0],            0.0, 0.5f*cell.s[2], 0.0 /*unused*/},
        cl_float4{ 0.5f*cell.s[0], 0.5f*cell.s[1],            0.0, 0.0 /*unused*/}};

    /*
     * Construct the fcc cubic lattice. Create an array of boolean values
     * for each existing site. Shuffle the array using Knuth's algorithm.
     */
    std::vector<bool> is_valid(n_sites, false); /* is site valid? */
    {
        math::rng::Kiss engine(true);           /* rng engine */
        math::rng::uniform<uint64_t> rand;      /* rng sampler */

        for (cl_ulong i = 0; i < n_points; ++i) {
            is_valid[i] = true;
        }

        for (cl_ulong i = 0; i < n_sites-1; ++i) {
            cl_ulong j = rand(engine, 0, n_sites-i);
            std::swap(is_valid[i], is_valid[j]);
        }
    }

    std::vector<cl_float4> points;             /* lattice site positions */
    {
        cl_float4 offset{xlo, ylo, zlo, 0.0f};

        cl_ulong site_ix = 0;
        for (cl_ulong i = 0; i < n_cells; i++) {
            for (cl_ulong j = 0; j < n_cells; j++) {
                for (cl_ulong k = 0; k < n_cells; k++) {
                    for (cl_ulong l = 0; l < 4; l++) {
                        /* Store particle position */
                        if (is_valid[site_ix]) {
                            cl_float4 pos{
                                (cl_float) i * cell.s[0],
                                (cl_float) j * cell.s[1],
                                (cl_float) k * cell.s[2],
                                0.0 /*unused*/};

                            pos += offset + basis[l];
                            points.push_back(pos);
                        }
                        site_ix++;  /* next lattice site */
                    }
                }
            }
        }
    }
    return points;
}

} /* generate */
