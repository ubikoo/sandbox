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

#include <vector>
#include "ito/opengl.hpp"
#include "ito/opencl.hpp"

struct Model {
    struct {} m_gl;

    struct {
        // cl_context context = NULL;
        // cl_device_id device = NULL;
        // cl_command_queue queue = NULL;
        // cl_program program = NULL;
        // enum {
        //     KernelEmpty = 0
        //     NumKernels,
        // };
        // std::vector<cl_kernel> kernels;
        // enum {
        //     NumBuffers = 0,
        // };
        // std::vector<cl_mem> buffers;
        // enum {
        //     NumImages = 0
        // };
        // std::vector<cl_mem> images;
    } m_cl;

    static void Handle(ito::gl::Renderer::Event &event);
    static void Update(void);
    static void Render(void);

    static Model Create(void);
    static void Destroy(Model &model);
};

#endif /* MODEL_H_ */
