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
 * @brief Create a new model and associated OpenCL/OpenGL objects.
 */
Model Model::Create()
{
    Model model;

    /* Setup Model. */
    { /* Empty */ }

    /* OpenGL objects. */
    { /* Empty */ }

    /* OpenCL objects. */
    {
#if 1
        /* Setup OpenCL context with a command queue on the specified device. */
        // m_cl.context = cl::CreateContext(CL_DEVICE_TYPE_GPU);
        // m_cl.device = cl::GetContextDevice(m_cl.context, Params::kDeviceIndex);
        // m_cl.queue = cl::CreateCommandQueue(m_cl.context, m_cl.device);
        // std::cout << cl::GetDeviceInfoStr(device) << "\n";
#else
        /* Setup OpenCL context based on the OpenGL context in the device. */
        // m_cl.device = cl::Context::get_device(m_cl.context, Params::kDeviceIndex);
        // m_cl.context = cl::Context::create_cl_gl_shared(m_cl.device);
        // m_cl.queue = cl::Queue::create(m_cl.context, m_cl.device);
        // std::cout << cl::GetDeviceInfoStr(device) << "\n";
#endif

#if 1
        /* Create OpenCL program from source. */
        // std::string source;
        // source.append(cl::LoadProgramSource("data/empty.cl"));
        // m_cl.program = cl::CreateProgramWithSource(m_cl.context, source);
        // cl::BuildProgram(m_cl.program, m_cl.device, "");
#else
        /* Create OpenCL program from file. */
        // m_cl.program = cl::CreateProgramFromFile(m_cl.context, "data/empty.cl");
        // cl::BuildProgram(m_cl.program, m_cl.device, "");
#endif

        /* Create OpenCL kernel. */
        // m_cl.kernels.resize(NumKernels, NULL);
        // m_cl.kernels[KernelEmpty] = cl::Kernel::create(m_cl.program, "empty");
    }

    return model;
}

/**
 * @brief Destroy a model and associated OpenCL/OpenGL objects.
 */
void Model::Destroy(Model &model)
{
    /* OpenGL objects. */
    {}

    /* OpenCL objects. */
    {
    //     for (auto &it : m_cl.images) {
    //         cl::Memory::release(it);
    //     }
    //     for (auto &it : m_cl.buffers) {
    //         cl::Memory::release(it);
    //     }
    //     for (auto &it : m_cl.kernels) {
    //         cl::Kernel::release(it);
    //     }
    //     cl::Program::release(m_cl.program);
    }
}

/**
 * @brief Handle the event in the model.
 */
void Model::Handle(gl::Renderer::Event &event)
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
    GLFWwindow *window = gl::Renderer::Window();
    if (window == nullptr) {
        return;
    }
}
