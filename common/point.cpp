/*
 * point.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include <vector>
#include "ito/math.hpp"
using namespace ito;
#include "point.hpp"

namespace common {

/**
 * @brief Create a collection of points with coordinates (x,y,z) uniformly
 * distributed inside a box:
 *  xlo < x < xhi
 *  ylo < y < yhi
 *  zlo < z < zhi
 */
std::vector<math::vec3f> CreateBox(
    const size_t n_points,
    const float xlo,
    const float ylo,
    const float zlo,
    const float xhi,
    const float yhi,
    const float zhi)
{
    ito_assert(n_points > 0, "invalid number of points");
    ito_assert(xlo < xhi && ylo < yhi && zlo < zhi, "invalid range");

    math::random_engine engine = math::make_random();
    math::random_uniform<float> urandom;

    std::vector<math::vec3f> points(n_points);
    for (auto &it : points) {
        it = math::vec3f{
            urandom(engine, xlo, xhi),
            urandom(engine, ylo, yhi),
            urandom(engine, zlo, zhi)
        };
    }
    return points;
}

/**
 * @brief Create a collection of points with coordinates (x,y,z) uniformly
 * distributed inside a sphere:
 *  rx = x - xctr
 *  ry = y - yctr
 *  rz = z - zctr
 *  sqrt(rx * rx + ry * ry + rz * rz) < radius
 */
std::vector<math::vec3f> CreateSphere(
    const size_t n_points,
    const float radius,
    const float xctr,
    const float yctr,
    const float zctr)
{
    ito_assert(n_points > 0, "invalid number of points");
    ito_assert(radius > 0.0f, "invalid sphere radius");

    math::random_engine engine = math::make_random();
    math::random_uniform<float> urandom;

    std::vector<math::vec3f> points(n_points);
    for (auto &it : points) {
        float phi = urandom(engine, 0.0f, 2.0f * M_PI);
        float cos_theta = urandom(engine, -1.0f, 1.0f);
        float r_cube = urandom(engine, 0.0f, 1.0f);

        float theta = std::acos(cos_theta);
        float radius = std::cbrt(r_cube);

        it = math::vec3f{
            radius * std::sin(theta) * std::cos(phi),
            radius * std::sin(theta) * std::sin(phi),
            radius * std::cos(theta)
        };
    }

    return points;
}

/**
 * @brief Create a collection of points inside a simple cubic lattice.
 */
std::vector<math::vec3f> CreateSccLattice(
    const size_t n_points,
    const float xlo,
    const float ylo,
    const float zlo,
    const float xhi,
    const float yhi,
    const float zhi)
{
    ito_assert(n_points > 0, "invalid number of points");
    ito_assert(xlo < xhi && ylo < yhi && zlo < zhi, "invalid range");

    math::random_engine engine = math::make_random();
    math::random_uniform<uint64_t> urandom64;

    /*
     * Compute the minimum number of lattice cells able to accomodate the
     * specified number of points. The unit cell of an ssc lattice contains
     * a single site located at
     *      r1 = (0,     0,   0)
     * A scc lattice with unit length has a cell size determined by the
     * total number of cells along each dimension a = 1.0 / n_cells.
     */
    size_t n_cells = 0;         /* number of unit cells in the lattice */
    size_t n_sites = 0;         /* number of sites in the lattice */
    while (n_sites < n_points) {
        ++n_cells;
        n_sites = n_cells * n_cells * n_cells;
    }

    /* Specify the unit cell basis vectors. */
    const math::vec3f cell = math::vec3f{
        (xhi - xlo) / (float) n_cells,
        (yhi - ylo) / (float) n_cells,
        (zhi - zlo) / (float) n_cells};

    /*
     * Construct the fcc cubic lattice. Create an array of boolean values
     * for each existing site. Shuffle the array using Knuth's algorithm.
     */
    std::vector<bool> is_valid(n_sites, false);  /* is site valid? */
    {
        for (size_t i = 0; i < n_points; ++i) {
            is_valid[i] = true;
        }

        for (size_t i = 0; i < n_sites-1; ++i) {
            size_t j = urandom64(engine, 0, n_sites - i);
            std::swap(is_valid[i], is_valid[j]);
        }
    }

    std::vector<math::vec3f> points;      /* lattice site positions */
    {
        size_t site_ix = 0;
        for (size_t i = 0; i < n_cells; i++) {
            for (size_t j = 0; j < n_cells; j++) {
                for (size_t k = 0; k < n_cells; k++) {
                    /* Store particle position */
                    if (is_valid[site_ix]) {
                        math::vec3f pos{
                            (float) i * cell[0],
                            (float) j * cell[1],
                            (float) k * cell[2]};
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
 * @brief Create a collection of points inside a face centred cubic lattice.
 */
std::vector<math::vec3f> CreateFccLattice(
    const size_t n_points,
    const float xlo,
    const float ylo,
    const float zlo,
    const float xhi,
    const float yhi,
    const float zhi)
{
    ito_assert(n_points > 0, "invalid number of points");
    ito_assert(xlo < xhi && ylo < yhi && zlo < zhi, "invalid range");

    math::random_engine engine = math::make_random();
    math::random_uniform<uint64_t> urandom64;

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
    size_t n_cells = 0;     /* number of unit cells in the lattice */
    size_t n_sites = 0;     /* number of sites in the lattice */
    while (n_sites < n_points) {
        ++n_cells;
        n_sites = 4 * n_cells * n_cells * n_cells;
    }

    /* Specify the unit cell basis vectors. */
    const math::vec3f cell = math::vec3f{
        (xhi - xlo) / (float) n_cells,
        (yhi - ylo) / (float) n_cells,
        (zhi - zlo) / (float) n_cells};

    std::array<math::vec3f,4> basis{
        math::vec3f{        0.0f,        0.0f,        0.0f},
        math::vec3f{        0.0f, 0.5f*cell.y, 0.5f*cell.z},
        math::vec3f{ 0.5f*cell.x,        0.0f, 0.5f*cell.z},
        math::vec3f{ 0.5f*cell.x, 0.5f*cell.y,        0.0f}};

    /*
     * Construct the fcc cubic lattice. Create an array of boolean values
     * for each existing site. Shuffle the array using Knuth's algorithm.
     */
    std::vector<bool> is_valid(n_sites, false);  /* is site valid? */
    {
        for (size_t i = 0; i < n_points; ++i) {
            is_valid[i] = true;
        }

        for (size_t i = 0; i < n_sites-1; ++i) {
            size_t j = urandom64(engine, 0, n_sites - i);
            std::swap(is_valid[i], is_valid[j]);
        }
    }

    std::vector<math::vec3f> points;          /* lattice site positions */
    {
        size_t site_ix = 0;
        for (size_t i = 0; i < n_cells; i++) {
            for (size_t j = 0; j < n_cells; j++) {
                for (size_t k = 0; k < n_cells; k++) {
                    for (size_t l = 0; l < 4; l++) {
                        /* Store particle position */
                        if (is_valid[site_ix]) {
                            math::vec3f pos{
                                (float) i * cell[0],
                                (float) j * cell[1],
                                (float) k * cell[2]};
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

} /* common */
