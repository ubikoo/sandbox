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

#include "lattice.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * Lattice::Lattice.
 * @brief Create a square lattice with n*n sites.
 */
Lattice::Lattice(const int32_t n_sites)
{
    m_n_sites = n_sites;
    m_sites.resize(n_sites * n_sites, Closed);
    m_engine.init();
}

/** ---------------------------------------------------------------------------
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
    return (x + y * m_n_sites);
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

/** ---------------------------------------------------------------------------
 * Lattice::is_open
 * @brief Is site (x,y) open?
 */
bool Lattice::is_open(int32_t x, int32_t y) const
{
    return (site(x,y) == Open);
}

/**
 * Lattice::generate
 * @brief Generate a new lattice state.
 */
void Lattice::generate(const double p_site)
{
    static math::rng::uniform<double> rand;
    for (auto &site : m_sites) {
        site = math::isless(rand(m_engine, 0.0, 1.0), p_site) ? Open : Closed;
    }
}
