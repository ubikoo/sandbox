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

#ifndef MD_MODEL_H_
#define MD_MODEL_H_

#include "atto/opencl/opencl.hpp"
#include "engine.hpp"

struct Model {
    /* ---- Model data ----------------------------------------------------- */
    size_t m_step;
    Engine m_engine;

    /* ---- Model member functions ----------------------------------------- */
    bool execute(void);
    void handle(const atto::gl::Event &event);

    Model();
    ~Model();
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
};

#endif /* MD_MODEL_H_ */
