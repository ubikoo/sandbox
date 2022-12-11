/*
 * lattice.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef ISING_LATTICE_H_
#define ISING_LATTICE_H_

#include "atto/opencl/opencl.hpp"

/**
 * Lattice
 */
struct Lattice {
    std::vector<int8_t> m_sites;   /* array of (n * n) sites */
    size_t m_n_sites;               /* number of sites along each dimension */
    double m_J;                     /* Ising energy coefficient */
    double m_h;                     /* Ising external field coefficient */

    /** Return the lattice size */
    size_t n_sites(void) const { return m_n_sites; }

    /** Set lattice parameters */
    void ising_params(const double J, const double h) {
        m_J = J;
        m_h = h;
    }

    /** Return the periodic image of position x */
    int32_t image(int32_t x) const;

    /** Return the index of the site (x,y) */
    int32_t index(int32_t x, int32_t y) const;

    /** Return a reference to site (x,y). */
    int8_t &site(int32_t x, int32_t y);

    /** Return a const reference to site (x,y). */
    const int8_t &site(int32_t x, int32_t y) const;

    /** Return the energy between a pair of sites (x1,y1) and (x2,y2). */
    double energy(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

    /** Return the energy of site (x,y). */
    double energy(int32_t x, int32_t y);

    /** Return the energy of the entire lattice. */
    double energy(void);

    /** Return the magnetization of the entire lattice. */
    double magnetic(void);

    /** Generate a random state. */
    void generate(void);

    /** Constructor/destructor. */
    Lattice() = default;
    Lattice(const size_t n_sites, const double J, const double h);
    ~Lattice();
};

#endif /* ISING_LATTICE_H_ */
