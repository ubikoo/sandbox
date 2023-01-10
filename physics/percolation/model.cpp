/*
 * model.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "model.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * @brief Create OpenCL context and associated objects.
 */
Model::Model()
{
    /*
     * Setup Model data.
     */
    {
        m_lattice = std::make_unique<Lattice>(Params::n_sites);
        m_uf = std::make_unique<UnionFind>(Params::n_sites * Params::n_sites);
        m_percolate = {0, 0, 0, 0};
        m_prob_site = 0.0;
    }
}

/**
 * @brief Destroy the OpenCL context and associated objects.
 */
Model::~Model()
{}

/** ---------------------------------------------------------------------------
 * @brief Solve the percolation problem over a random lattice.
 */
void Model::execute()
{
    reset();
    for (size_t iter = 0; iter < Params::n_iters; ++iter) {
        compute();
        sample();
        /* DEBUG */
        if (10*iter % Params::n_iters  == 0) {
            double ns = static_cast<double>(m_percolate[0]);
            double px = static_cast<double>(m_percolate[1]) / ns;
            double py = static_cast<double>(m_percolate[2]) / ns;
            double pb = static_cast<double>(m_percolate[3]) / ns;
            // std::cout << " (thread) m_prob_site " << m_prob_site;
            // std::cout << " m_percolate " << px << " " << py << " " << pb << "\n";
        }
    }
}

/**
 * @brief Compute the percolation clusters
 */
void Model::compute()
{
    /* Generate a new lattice state */
    m_lattice->generate(m_prob_site);

    /* Compute connected components using a unionfind data structure */
    m_uf->clear();
    for (int32_t x = 0; x < Params::n_sites; ++x) {
        for (int32_t y = 0; y < Params::n_sites; ++y) {
            if (m_lattice->is_open(x, y)) {
                int32_t ix_0 = m_lattice->index(x, y);

                if (x > 0 && m_lattice->is_open(x-1, y)) {
                    int32_t ix_1 = m_lattice->index(x-1, y);
                    m_uf->join(ix_0, ix_1);
                }

                if (y > 0 && m_lattice->is_open(x, y-1)) {
                    int32_t ix_1 = m_lattice->index(x, y-1);
                    m_uf->join(ix_0, ix_1);
                }
            }
        }
    }
}

/**
 * @brief Reset model counters.
 */
void Model::reset()
{
    m_percolate = {0, 0, 0, 0};
}

/**
 * @brief Sample model counters.
 */
void Model::sample()
{
    /* Does the lattice percolate in the x-direction? */
    bool percolate_x = false;
    {
        std::set<size_t> components_xlo;
        for (int32_t y = 0; y < Params::n_sites; ++y) {
            if (m_lattice->is_open(0, y)) {
                int32_t ix = m_lattice->index(0, y);
                components_xlo.insert(m_uf->find(ix));
            }
        }

        std::set<size_t> components_xhi;
        for (int32_t y = 0; y < Params::n_sites; ++y) {
            if (m_lattice->is_open(Params::n_sites-1, y)) {
                int32_t ix = m_lattice->index(Params::n_sites-1, y);
                components_xhi.insert(m_uf->find(ix));
            }
        }

        for (auto &it : components_xlo) {
            if (components_xhi.find(it) != components_xhi.end()) {
                percolate_x = true;
                break;
            }
        }
    }

    /* Does the lattice percolate in the y-direction? */
    bool percolate_y = false;
    {
        std::set<size_t> components_ylo;
        for (int32_t x = 0; x < Params::n_sites; ++x) {
            if (m_lattice->is_open(x, 0)) {
                int32_t ix = m_lattice->index(x, 0);
                components_ylo.insert(m_uf->find(ix));
            }
        }

        std::set<size_t> components_yhi;
        for (int32_t x = 0; x < Params::n_sites; ++x) {
            if (m_lattice->is_open(x, Params::n_sites-1)) {
                int32_t ix = m_lattice->index(x, Params::n_sites-1);
                components_yhi.insert(m_uf->find(ix));
            }
        }

        for (auto &it : components_ylo) {
            if (components_yhi.find(it) != components_yhi.end()) {
                percolate_y = true;
                break;
            }
        }
    }

    /* Accumulate percolation counters */
    uint8_t cx = percolate_x ? 1 : 0;
    uint8_t cy = percolate_y ? 1 : 0;

    m_percolate[0] += 1;
    m_percolate[1] += cx;
    m_percolate[2] += cy;
    m_percolate[3] += cx * cy;
}

/**
 * @brief Model thread execute function.
 */
void *Model::thread(void *ptr)
{
    Model *model = static_cast<Model *>(ptr);
    model->execute();
    pthread_exit(NULL);
}
