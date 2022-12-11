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

#ifndef MD_ENGINE_H_
#define MD_ENGINE_H_

#include "atto/opencl/opencl.hpp"
#include "base.hpp"
#include "compute.hpp"
#include "generate.hpp"
#include "io.hpp"

/**
 * Engine
 * @brief Molecular dynamics physics engine maintaining a collection of atoms
 * inside a domain.
 *
 * Atom is a plain data structure holding atom attributes - mass, inverse mass,
 * position, momentum and force.
 *
 * The graph structure is represented by an adjacency list where each atom has
 * a pre-specified maximum number of neighbours. The struct Pair represents any
 * edge connecting two neighbour atoms whose pairwise distance is smaller than
 * than a specified radius (>= cutoff radius).
 *
 * Domain is an axis-aligned geometric region centred at (0,0,0) bounded by
 * (-length / 2, length / 2).
 */
struct Engine {
    /* Engine member variables. */
    cl_ulong m_step;                    /* integration step */
    Domain m_domain;                    /* fluid domain */
    Field m_field;                      /* fluid force field */
    Thermostat m_thermostat;            /* fluid thermostat */
    std::vector<Atom> m_atoms;          /* fluid atoms */
    List m_list;                        /* neighbour list parameters */
    Grid m_grid;                        /* grid map parameters */

    /* Engine program. */
    cl_context m_context = NULL;
    cl_device_id m_device = NULL;
    cl_command_queue m_queue = NULL;
    cl_program m_program = NULL;

    /* Engine kernels. */
    enum {
        KernelBeginIntegrate = 0,
        KernelEndIntegrate,
        KernelUpdateAtoms,
        KernelComputeForces,
        KernelCopyAtomPoints,
        KernelThermostatForce,
        KernelThermostatIntegrate,
        KernelClearGrid,
        KernelBuildGrid,
        KernelClearNList,
        KernelBuildNList,
        KernelCheckNList,
        NumKernels
    };
    std::vector<cl_kernel> m_kernels;

    /* Engine device buffer objects. */
    enum {
        BufferDomain = 0,
        BufferField,
        BufferAtoms,
        BufferThermostat,
        BufferThermostatGradSq,
        BufferThermostatLaplace,
        BufferNList,
        BufferGrid,
        BufferGLPointVbo,
        NumBuffers
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

    /** Setup/teardown engine. */
    void setup(
        const cl_context &context,
        const cl_device_id &device,
        const cl_command_queue &queue,
        const GLuint &gl_vertex_buffer);

    void teardown(void);

    /* Constructor/destructor. */
    Engine() = default;
    ~Engine() = default;
};

#endif /* MD_ENGINE_H_ */
