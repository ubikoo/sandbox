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
 * Model::Model
 * @brief Create OpenCL context and associated objects.
 */
Model::Model()
{
    /*
     * Setup OpenCL program.
     */
    {
        /* Create a context with a command queue on the specified device. */
        m_context = cl::Context::create(CL_DEVICE_TYPE_GPU);
        m_device = cl::Context::get_device(m_context, Params::device_index);
        m_queue = cl::Queue::create(m_context, m_device);
        std::cout << cl::Device::get_info_string(m_device) << "\n";

        /* Create the program object. */
        m_program = cl::Program::create_from_file(m_context, "data/pi.cl");
        cl::Program::build(m_program, m_device, "");
        // std::cout << cl::Program::get_source(m_program) << "\n";
    }

    /*
     * Setup Model data.
     */
    {
        std::cout << "work_group_size " << Params::work_group_size << "\n";
        std::cout << "num_work_items " << Params::num_work_items << "\n";
        std::cout << "num_work_groups " << Params::num_work_groups << "\n";

        m_data.num_steps = Params::num_iters * Params::num_work_items;
        m_data.num_iters = Params::num_iters;
        m_data.step_size = 1.0 / (cl_double) Params::num_steps;
        m_data.group_sums.resize(Params::num_work_groups, 0.0);
        m_data.pi_cpu = 0.0;
        m_data.pi_gpu = 0.0;
    }

    /*
     * Setup Pi Kernel data.
     */
    {
        /* Create the pi integration in the program object. */
        m_kernels.resize(NumKernels, NULL);
        m_kernels[KernelPi] = cl::Kernel::create(m_program, "pi");

        /* Create memory buffers. */
        m_buffers.resize(NumBuffers, NULL);
        m_buffers[BufferGroupSums] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_WRITE_ONLY,
            m_data.group_sums.size() * sizeof(m_data.group_sums[0]),
            (void *) NULL);
    }
}

/**
 * Model::~Model
 * @brief Destroy the OpenCL context and associated objects.
 */
Model::~Model()
{
    /* Teardown OpenCL data. */
    {
        for (auto &it : m_images) {
            cl::Memory::release(it);
        }
        for (auto &it : m_buffers) {
            cl::Memory::release(it);
        }
        for (auto &it : m_kernels) {
            cl::Kernel::release(it);
        }
        cl::Program::release(m_program);
        cl::Queue::release(m_queue);
        cl::Device::release(m_device);
        cl::Context::release(m_context);
    }
}

/** ---------------------------------------------------------------------------
 * Model::execute
 * @brief Execute the model.
 */
void Model::execute(void)
{
    execute_cpu();
    execute_gpu();
    std::cout << core::str_format("pi_cpu %.15lf, err %.15lf\n",
        m_data.pi_cpu, std::fabs(M_PI - m_data.pi_cpu));
    std::cout << core::str_format("pi_gpu %.15lf, err %.15lf\n",
        m_data.pi_gpu, std::fabs(M_PI - m_data.pi_gpu));
}

/**
 * Model::execute_cpu
 * @brief Integrate 4/(1+x*x) dx from 0 to 1 to return pi.
 */
void Model::execute_cpu(void)
{
    cl_double sum = 0.0;
    for (size_t i = 0; i < m_data.num_steps; ++i) {
        cl_double x = (i + 0.5) * m_data.step_size;
        sum += 4.0 / (1.0 + x*x);
    }
    m_data.pi_cpu = sum * m_data.step_size;
}

/**
 * Model::execute_gpu
 */
void Model::execute_gpu(void)
{
    /* Run the pi kernel. */
    {
        /* Set kernel arguments. */
        cl::Kernel::set_arg(m_kernels[KernelPi], 0, sizeof(cl_mem), &m_buffers[BufferGroupSums]);
        cl::Kernel::set_arg(m_kernels[KernelPi], 1, Params::work_group_size * sizeof(cl_double), NULL);
        cl::Kernel::set_arg(m_kernels[KernelPi], 2, sizeof(cl_uint), &m_data.num_iters);
        cl::Kernel::set_arg(m_kernels[KernelPi], 3, sizeof(cl_double), &m_data.step_size);

        /* Set the size of the NDRange workgroups */
        cl::NDRange local_ws(Params::work_group_size);
        cl::NDRange global_ws(Params::num_work_items);

        /* Run the kernel */
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelPi],
            cl::NDRange::Null,
            global_ws,
            local_ws,
            NULL,
            NULL);

        /* Wait for kernel to compute */
        cl::Queue::finish(m_queue);

        /* Read the partial sums back to the host. */
        cl::Queue::enqueue_read_buffer(
            m_queue,
            m_buffers[BufferGroupSums],
            CL_TRUE,
            0,
            m_data.group_sums.size() * sizeof(m_data.group_sums[0]),
            (void *) &m_data.group_sums[0],
            NULL,
            NULL);
    }

    /* Compute final integral value from the kernel partial sums. */
    {
        cl_double sum = 0.0;
        for (size_t i = 0; i < Params::num_work_groups; ++i) {
            sum += m_data.group_sums[i];
        }
        m_data.pi_gpu = sum;
    }
}

