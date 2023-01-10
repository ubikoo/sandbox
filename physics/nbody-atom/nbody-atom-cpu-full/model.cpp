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

#include "atto/opencl/opencl.hpp"
#include "model.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * @brief Create OpenCL context and associated objects.
 */
Model::Model(void)
{
    /* Initialize time step. */
    m_step = 0;

    /* Setup engine object. */
    m_engine.setup();
    m_engine.generate();
    m_engine.reset(0.5 * Params::pair_sigma);
}

/**
 * @brief Destroy the OpenCL context and associated objects.
 */
Model::~Model()
{
    /* Teardown engine. */
    m_engine.teardown();
}

/** ---------------------------------------------------------------------------
 * @brief Execute the model.
 */
bool Model::execute(void)
{
    /* Model pre-execution. */
    if (m_step == Params::n_min_steps) {
        m_engine.reset(Params::pair_r_hard * Params::pair_sigma);
    }

    /* Execute an engine step. */
    m_engine.execute();

    /* Model post-execution. */
    if (++m_step%Params::sample_frequency == 0) {
        std::cout << "step " << m_step << "\n" << m_engine.sample() << "\n";
    }

    return (m_step < Params::n_run_steps);
}

/** ---------------------------------------------------------------------------
 * @brief Handle an event.
 */
void Model::handle(const gl::Event &event)
{}
