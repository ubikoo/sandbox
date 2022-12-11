/*
 * engine.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef SPH_ENGINE_H_
#define SPH_ENGINE_H_

#include "atto/opencl/opencl.hpp"
#include "base.hpp"
#include "generate.hpp"

/**
 * Engine
 * @brief Smooth particle hydrodynamics engine maintaining a collection of
 * particles inside a domain.
 * Domain is a bounded, axis-aligned, geometric region in space with specified
 * boundary conditions.
 */
struct Engine {
    /* Engine member variables. */
    cl_ulong m_step;                    /* integration step */
    cl_float4 m_gravity;                /* gravity direction */
    Domain m_domain;                    /* fluid domain */
    std::vector<Particle> m_particles;  /* fluid particles */

    /* Engine program. */
    cl_context m_context = NULL;
    cl_device_id m_device = NULL;
    cl_command_queue m_queue = NULL;
    cl_program m_program = NULL;

    /* Engine kernels. */
    enum {
        KernelBeginIntegrate = 0,
        KernelEndIntegrate,
        KernelComputeDensity,
        KernelComputeForces,
        KernelUpdateBoundaries,
        KernelCopyVertexData,
        NumKernels,
    };
    std::vector<cl_kernel> m_kernels;

    /* Engine device buffer objects. */
    enum {
        BufferDomain = 0,
        BufferParticles,
        BufferVertexData,
        NumBuffers,
    };
    std::vector<cl_mem> m_buffers;

    /* Engine device image objects. */
    enum {
        NumImages = 0
    };
    std::vector<cl_mem> m_images;

    /** Execute one integration step. */
    void execute(void);

    /** Has the integration finished? */
    bool finished(void) const { return m_step < Params::n_steps; }

    /** Setup/teardown engine object. */
    void setup(
        const cl_context &context,
        const cl_device_id &device,
        const cl_command_queue &queue,
        GLuint gl_point_vbo);

    void teardown(void);

    /* Constructor/destructor. */
    Engine() = default;
    ~Engine() = default;
};

#endif /* SPH_ENGINE_H_ */
