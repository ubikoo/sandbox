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

#ifndef LATTICE_H_
#define LATTICE_H_

#include "base.hpp"
#include "unionfind.hpp"

/**
 * @brief Lattice maintains a square lattice of n_sites. Each site can be in
 * on of the mutually exclusive states {Open = 0, Close = 1} with probabilities
 * p and (1-p) respectively.
 */
struct Lattice {
    /** Lattice site state. */
    enum : uint32_t {
        Closed = 0,
        Open,
    };

    std::vector<int8_t> m_sites;    /* array of (n * n) sites */
    int32_t m_n_sites;              /* number of sites along each dimension */
    atto::math::rng::Kiss m_engine; /* random number generator */

    /** Return a reference/const reference to site (x,y). */
    int8_t &site(int32_t x, int32_t y);
    const int8_t &site(int32_t x, int32_t y) const;

    /** Return the periodic image of position x. */
    int32_t image(int32_t x) const;

    /** Return the index of the site (x,y). */
    int32_t index(int32_t x, int32_t y) const;

    /** Is site (x,y) open? */
    bool is_open(int32_t x, int32_t y) const;

    /** Generate a new lattice state. */
    void generate(const double p_site);

    /** Constructor/destructor. */
    explicit Lattice(const int32_t n_sites);
    ~Lattice() = default;
};

#endif /* LATTICE_H_ */
