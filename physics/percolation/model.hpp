/*
 * model.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <set>
#include <vector>
#include "base.hpp"
#include "lattice.hpp"
#include "unionfind.hpp"

struct Model {
    std::unique_ptr<Lattice> m_lattice;
    std::unique_ptr<UnionFind> m_uf;
    std::array<uint64_t, 4> m_percolate;
    double m_prob_site;

    /** @brief Solve the percolation problem over a random lattice. */
    void execute();

    /** Compute the percolation clusters. */
    void compute();

    /** Reset model counters. */
    void reset();

    /** Sample model counters. */
    void sample();

    /** @brief Model thread execute function. */
    static void *thread(void *ptr);

    Model();
    ~Model();
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
};

#endif /* MODEL_H_ */
