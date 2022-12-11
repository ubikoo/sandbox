/*
 * lattice.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "atto/opencl/opencl.hpp"
#include "lattice.hpp"
using namespace atto;

/**
 * Lattice::Lattice.
 * @brief Create a square lattice with n*n sites.
 */
Lattice::Lattice(const size_t n_sites, const double J, const double h)
{
    m_sites.resize(n_sites * n_sites, 0);
    m_n_sites = n_sites;
    m_J = J;
    m_h = h;
}

Lattice::~Lattice()
{}

/**
 * Lattice::image
 * @brief Return the periodic image of position x.
 */
int32_t Lattice::image(int32_t x) const
{
    if (x < 0) {
        x += m_n_sites;
    }
    if (x >= (int32_t) m_n_sites) {
        x -= m_n_sites;
    }
    return x;
}

/**
 * Lattice::index
 * @brief Return the index of the site (x,y)
 */
int32_t Lattice::index(int32_t x, int32_t y) const
{
    x = image(x);
    y = image(y);
    return x * m_n_sites + y;
}

/**
 * Lattice::site
 * @brief Return a reference to site (x,y).
 */
int8_t &Lattice::site(int32_t x, int32_t y)
{
    return m_sites[index(x, y)];
}

/**
 * Lattice::site
 * @brief Return a const reference to site (x,y).
 */
const int8_t &Lattice::site(int32_t x, int32_t y) const
{
    return m_sites[index(x, y)];
}

/**
 * Lattice::energy
 * @brief Return the energy between a pair of sites (x1,y1) and (x2,y2).
 */
double Lattice::energy(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    return -m_J * site(x1, y1) * site(x2, y2);
}

/**
 * Lattice::energy
 * @brief Return the energy of site (x,y).
 */
double Lattice::energy(int32_t x, int32_t y)
{
    return (energy(x, y, x - 1,     y)
          + energy(x, y, x + 1,     y)
          + energy(x, y,     x, y - 1)
          + energy(x, y,     x, y + 1)
          - m_h * site(x,y));
}

/**
 * Lattice::energy
 * @brief Return the energy of the entire lattice.
 */
double Lattice::energy(void)
{
    /*
     * Sum the energy of every site in the lattice.
     * Divide by 2 because every pair is counted twice.
     */
    double result = 0.0;
    for (size_t x = 0; x < m_n_sites; ++x) {
        for (size_t y = 0; y < m_n_sites; ++y) {
            result += energy(x, y);
        }
    }

    return 0.5*result;
}

/**
 * Lattice::magnetic
 * @brief Return the magnetization of the entire lattice.
 */
double Lattice::magnetic(void)
{
    double result = 0.0;
    for (size_t x = 0; x < m_n_sites; ++x) {
        for (size_t y = 0; y < m_n_sites; ++y) {
            result += site(x, y);
        }
    }
    return result;
}

/**
 * Lattice::generate
 * @brief Generate a random state.
 */
void Lattice::generate(void)
{
    math::rng::Kiss engine(true);
    math::rng::uniform<double> rand;

    for (size_t x = 0; x < m_n_sites; ++x) {
        for (size_t y = 0; y < m_n_sites; ++y) {
            site(x, y) = math::isless(rand(engine, 0.0, 1.0), 0.5) ? -1 : 1;
        }
    }
}
