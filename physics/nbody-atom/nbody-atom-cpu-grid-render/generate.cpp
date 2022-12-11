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
std::vector<math::vec3d> points_random(
    const size_t n_points,
    const double xlo,
    const double ylo,
    const double zlo,
    const double xhi,
    const double yhi,
    const double zhi)
{
    core_assert(n_points > 0, "invalid number of points");
    core_assert(xlo < xhi && ylo < yhi && zlo < zhi, "invalid range");

    math::rng::Kiss engine(true);           /* rng engine */
    math::rng::uniform<double> rand;        /* rng sampler */

    std::vector<math::vec3d> points(n_points);
    for (auto &it : points) {
        it = math::vec3d{
            rand(engine, xlo, xhi),
            rand(engine, ylo, yhi),
            rand(engine, zlo, zhi)};
    }
    return points;
}

/**
 * points_cubic
 * @brief Create a collection of points inside a simple cubic lattice with
 * the specified dimensions.
 */
std::vector<math::vec3d> points_cubic(
    const size_t n_points,
    const double xlo,
    const double ylo,
    const double zlo,
    const double xhi,
    const double yhi,
    const double zhi)
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
    size_t n_cells = 0;       /* number of unit cells in the lattice */
    size_t n_sites = 0;       /* number of sites in the lattice */
    while (n_sites < n_points) {
        ++n_cells;
        n_sites = n_cells * n_cells * n_cells;
    }

    /* Specify the unit cell basis vectors. */
    const math::vec3d cell = math::vec3d{
        (xhi - xlo) / (double) n_cells,
        (yhi - ylo) / (double) n_cells,
        (zhi - zlo) / (double) n_cells};

    /*
     * Construct the fcc cubic lattice. Create an array of boolean values
     * for each existing site. Shuffle the array using Knuth's algorithm.
     */
    std::vector<bool> is_valid(n_sites, false); /* is site valid? */
    {
        math::rng::Kiss engine(true);           /* rng engine */
        math::rng::uniform<uint64_t> rand;      /* rng sampler */

        for (size_t i = 0; i < n_points; ++i) {
            is_valid[i] = true;
        }

        for (size_t i = 0; i < n_sites-1; ++i) {
            size_t j = rand(engine, 0, n_sites-i);
            std::swap(is_valid[i], is_valid[j]);
        }
    }

    std::vector<math::vec3d> points;             /* lattice site positions */
    {
        size_t site_ix = 0;
        for (size_t i = 0; i < n_cells; i++) {
            for (size_t j = 0; j < n_cells; j++) {
                for (size_t k = 0; k < n_cells; k++) {
                    /* Store particle position */
                    if (is_valid[site_ix]) {
                        math::vec3d pos{
                            (double) i * cell.x,
                            (double) j * cell.y,
                            (double) k * cell.z};
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
std::vector<math::vec3d> points_fcc(
    const size_t n_points,
    const double xlo,
    const double ylo,
    const double zlo,
    const double xhi,
    const double yhi,
    const double zhi)
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
    size_t n_cells = 0;       /* number of unit cells in the lattice */
    size_t n_sites = 0;       /* number of sites in the lattice */
    while (n_sites < n_points) {
        ++n_cells;
        n_sites = 4 * n_cells * n_cells * n_cells;
    }

    /* Specify the unit cell basis vectors. */
    const math::vec3d cell = math::vec3d{
        (xhi - xlo) / (double) n_cells,
        (yhi - ylo) / (double) n_cells,
        (zhi - zlo) / (double) n_cells};

    std::array<math::vec3d,4> basis{
        math::vec3d{         0.0,         0.0,         0.0},
        math::vec3d{         0.0, 0.5f*cell.y, 0.5f*cell.z},
        math::vec3d{ 0.5f*cell.x,         0.0, 0.5f*cell.z},
        math::vec3d{ 0.5f*cell.x, 0.5f*cell.y,         0.0}};

    /*
     * Construct the fcc cubic lattice. Create an array of boolean values
     * for each existing site. Shuffle the array using Knuth's algorithm.
     */
    std::vector<bool> is_valid(n_sites, false); /* is site valid? */
    {
        math::rng::Kiss engine(true);           /* rng engine */
        math::rng::uniform<uint64_t> rand;      /* rng sampler */

        for (size_t i = 0; i < n_points; ++i) {
            is_valid[i] = true;
        }

        for (size_t i = 0; i < n_sites-1; ++i) {
            size_t j = rand(engine, 0, n_sites-i);
            std::swap(is_valid[i], is_valid[j]);
        }
    }

    std::vector<math::vec3d> points;             /* lattice site positions */
    {
        size_t site_ix = 0;
        for (size_t i = 0; i < n_cells; i++) {
            for (size_t j = 0; j < n_cells; j++) {
                for (size_t k = 0; k < n_cells; k++) {
                    for (size_t l = 0; l < 4; l++) {
                        /* Store particle position */
                        if (is_valid[site_ix]) {
                            math::vec3d pos{
                                (double) i * cell.x,
                                (double) j * cell.y,
                                (double) k * cell.z};
                            pos += basis[l];
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
