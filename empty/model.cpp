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

#include <vector>
#include "ito/opengl.hpp"
#include "ito/opencl.hpp"
using namespace ito;
#include "model.hpp"
#include "params.hpp"

/**
 * @brief Create a new model and associated objects.
 */
Model Model::Create()
{
    Model model;

    /* Setup Model. */
    { /* Empty */ }

    /* OpenGL data. */
    { /* Empty */ }

    /* OpenCL data. */
    {
        /* OpenCL context, device and queue data. */
        model.m_cl.context = clfw::Context();
        model.m_cl.device = clfw::Device();
        model.m_cl.queue = clfw::Queue();

        /* Create OpenCL program. */
#if 1
        // std::string source;
        // source.append(cl::LoadProgramSource("data/empty.cl"));
        // m_cl.program = cl::CreateProgramWithSource(m_cl.context, source);
#else
        // m_cl.program = cl::CreateProgramFromFile(m_cl.context, "data/empty.cl");
#endif
        // cl::BuildProgram(m_cl.program, m_cl.device, "");

        /* Create OpenCL kernel. */
        // m_cl.kernels.resize(NumKernels, NULL);
        // m_cl.kernels[KernelEmpty] = cl::CreateKernel(m_cl.program, "empty");
    }

    return model;
}

/**
 * @brief Destroy the model and associated objects.
 */
void Model::Destroy(Model &model)
{
    /* OpenGL data. */
    {}

    /* OpenCL data. */
    {
        for (auto &it : model.m_cl.images) {
            cl::ReleaseMemObject(it);
        }
        for (auto &it : model.m_cl.buffers) {
            cl::ReleaseMemObject(it);
        }
        for (auto &it : model.m_cl.kernels) {
            cl::ReleaseKernel(it);
        }
        cl::ReleaseProgram(model.m_cl.program);
    }
}

/**
 * @brief Handle the event in the model.
 */
void Model::Handle(glfw::Event &event)
{}

/**
 * @brief Update the model.
 */
void Model::Update()
{}

/**
 * @brief Render the model.
 */
void Model::Render(void)
{
    GLFWwindow *window = glfw::Window();
    if (window == nullptr) {
        return;
    }
}
