/*
 * main.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include <memory>
#include <chrono>
#include <numeric>
#include <vector>

#include "atto/opencl/opencl.hpp"
#include "model.hpp"
using namespace atto;

/**
 * main test client
 */
int main(int argc, char const *argv[])
{
    /* Read arguments */
    core_assert(argc == 2, "insufficient arguments");
    double p_site = std::stod(argv[1]);

    /*
     * Execute a percolation model on each thread.
     * For portability, explicitly create threads in a joinable state.
     */
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

    std::vector<Model> models(Params::n_threads);
    for (auto &it : models) {
        it.m_prob_site = p_site;
    }

    std::vector<pthread_t> threads;
    for (size_t i = 0; i < Params::n_threads; ++i) {
        threads.emplace_back();
        pthread_create(
            &threads.back(),
            &thread_attr,
            Model::thread,
            (void *) &models[i]);
    }

    for (auto &it : threads) {
        pthread_join(it, NULL);
    }

    /* Compute model statistics */
    std::array<uint64_t, 4> percolate = {0, 0, 0, 0};
    for (auto &it : models) {
        percolate[0] += it.m_percolate[0];
        percolate[1] += it.m_percolate[1];
        percolate[2] += it.m_percolate[2];
        percolate[3] += it.m_percolate[3];
    }

    double ns = static_cast<double>(percolate[0]);
    double px = static_cast<double>(percolate[1]) / ns;
    double py = static_cast<double>(percolate[2]) / ns;
    double pb = static_cast<double>(percolate[3]) / ns;
    std::cout << " p_site " << p_site;
    std::cout << " percolate " << px << " " << py << " " << pb << "\n";

    exit(EXIT_SUCCESS);
}
