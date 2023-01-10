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

#include "ito/opengl.hpp"
#include "ito/opencl.hpp"
#include "model-cpu.hpp"
#include "model-gpu.hpp"

struct Model {
    std::vector<Point> m_points;
    ModelCPU m_cpu;
    ModelGPU m_gpu;

    struct {} m_gl;
    struct {} m_cl;

    void Handle(ito::glfw::Event &event);
    void Update(void);
    void Render(void);

    static Model Create(void);
    static void Destroy(Model &model);
};

#endif /* MODEL_H_ */
