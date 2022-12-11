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
#include <random>
#include <chrono>
#include "base.hpp"

struct Model {
    /* ---- Model OpenCL data ---------------------------------------------- */
    cl_context m_context = NULL;
    cl_device_id m_device = NULL;
    cl_command_queue m_queue = NULL;
    cl_program m_program = NULL;

    enum {
        KernelPi = 0,
        NumKernels
    };
    std::vector<cl_kernel> m_kernels;

    enum {
        BufferGroupSums = 0,
        NumBuffers
    };
    std::vector<cl_mem> m_buffers;

    enum {
        NumImages = 0
    };
    std::vector<cl_mem> m_images;

    /* ---- Model data ----------------------------------------------------- */
    struct Data {
        cl_ulong num_steps;
        cl_ulong num_iters;
        cl_double step_size;
        std::vector<cl_double> group_sums;
        cl_double pi_cpu;
        cl_double pi_gpu;
    } m_data;

    /* ---- Model member functions ----------------------------------------- */
    void execute(void);
    void execute_gpu(void);
    void execute_cpu(void);

    Model();
    ~Model();
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
};

#endif /* MODEL_H_ */
