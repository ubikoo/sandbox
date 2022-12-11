/*
 * engine.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include <random>
#include "atto/opencl/opencl.hpp"
#include "engine.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * Engine::setup
 * @brief Setup compute object state.
 */
void Engine::setup(
    const cl_context &context,
    const cl_device_id &device,
    const cl_command_queue &queue,
    GLuint gl_point_vbo)
{
    /* ------------------------------------------------------------------------
     * Setup engine data.
     */
    {
        /* Set integration step. */
        m_step = 0;

        /* Set gravity force */
        m_gravity = cl_float4{Params::gravity_coeff, 0.0f, 0.0f, 0.0f};
    }

    {
        /* Create fluid domain. */
        cl_float volume = (cl_float) Params::n_particles / Params::eos_density;
        cl_float length = std::pow(volume, 1.0/3.0);
        cl_float half = 0.5 * length;
        cl_float4 scale{1.0f, 3.0f, 1.0f, 0.0f};

        cl_float4 bound_lo{-half, -half, -half, 0.0f /*unused*/};
        cl_float4 bound_hi{ half,  half,  half, 0.0f /*unused*/};
        m_domain = Domain{scale * bound_lo, scale * bound_hi};
    }

    {
        /* Create fluid particles. */
        Particle particle{
            .prev = cl_float4{},
            .pos = cl_float4{},
            .vel = cl_float4{},
            .force = cl_float4{},
            .mass = Params::particle_mass,
            .dens = 0.0f,
            .pres = 0.0f,
        };
        m_particles.resize(Params::n_particles, particle);

        /* Generate particle positions inside the domain */
        const cl_float epsilon = 0.9;  /* < 1.0, strictly inside the domain */
        cl_float4 half = m_domain.bound_hi - m_domain.bound_lo;
        half *= (0.5 * epsilon);

        std::vector<cl_float4> positions = generate::points_fcc(
            Params::n_particles,
            -half.s[0],
            -half.s[1],
            -half.s[2],
             half.s[0],
             half.s[1],
             half.s[2]);

        cl_uint idx = 0;
        cl_float4 com_pos = cl_float4{};
        for (auto &particle : m_particles) {
            particle.pos = positions[idx++];
            com_pos += particle.pos;
        }

        com_pos /= static_cast<cl_float>(Params::n_particles);
        for (auto &particle : m_particles) {
            particle.pos -= com_pos;
            particle.prev = particle.pos;
        }

        std::random_device seed;    /* rng device */
        std::mt19937 rng(seed());   /* rng engine */
        std::uniform_real_distribution<cl_float> dist(-1.0f,1.0f);
        cl_float4 com_vel = cl_float4{};
        for (auto &particle : m_particles) {
            particle.vel = cl_float4{dist(rng), dist(rng), dist(rng), 0.0f};
            com_vel += particle.vel;
        }
        com_vel /= static_cast<cl_float>(Params::n_particles);
        for (auto &particle : m_particles) {
            particle.vel -= com_vel;
        }
    }

    {
        /* Setup grid map parameters. */
        cl_uint capacity = Params::load_factor * Params::n_particles;
        m_grid = Grid{.capacity = capacity};
    }

   /* ------------------------------------------------------------------------
    * Setup engine OpenCL data.
    */
    {
        /* Setup engine context and device queue. */
        m_context = context;
        m_device = device;
        m_queue = queue;

        /* Create engine program object. */
        std::string source;
        source.append(cl::Program::load_source_from_file("data/base.cl"));
        source.append(cl::Program::load_source_from_file("data/grid.cl"));
        source.append(cl::Program::load_source_from_file("data/kernel.cl"));
        source.append(cl::Program::load_source_from_file("data/compute.cl"));
        source.append(cl::Program::load_source_from_file("data/integrate.cl"));
        source.append(cl::Program::load_source_from_file("data/particles.cl"));

        m_program = cl::Program::create_from_source(m_context, source);
        cl::Program::build(m_program, m_device, "");
        std::cout << cl::Program::get_source(m_program) << "\n";

        /* Create engine kernels. */
        m_kernels.resize(NumKernels, NULL);
        m_kernels[KernelBeginIntegrate] = cl::Kernel::create(m_program, "begin_integrate");
        m_kernels[KernelEndIntegrate] = cl::Kernel::create(m_program, "end_integrate");
        m_kernels[KernelClearGrid] = cl::Kernel::create(m_program, "clear_grid");
        m_kernels[KernelBuildGrid] = cl::Kernel::create(m_program, "build_grid");
        m_kernels[KernelComputeDensity] = cl::Kernel::create(m_program, "compute_density");
        m_kernels[KernelComputeForces] = cl::Kernel::create(m_program, "compute_forces");
        m_kernels[KernelUpdateBoundaries] = cl::Kernel::create(m_program, "update_boundaries");
        m_kernels[KernelCopyVertexData] = cl::Kernel::create(m_program, "copy_vertex_data");

        /* Create engine device buffer objects. */
        m_buffers.resize(NumBuffers, NULL);

        m_buffers[BufferDomain] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            sizeof(Domain),
            (void *) NULL);

        m_buffers[BufferParticles] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            m_particles.size() * sizeof(m_particles[0]),
            (void *) NULL);

        m_buffers[BufferGrid] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            m_grid.capacity * sizeof(Grid::Node),
            (void *) NULL);

        m_buffers[BufferVertexData] = cl::gl::create_from_gl_buffer(
            m_context,
            CL_MEM_WRITE_ONLY,
            gl_point_vbo);
    }

    /*
     * Copy engine data to the device.
     */
    {
        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferDomain],
            sizeof(m_domain),
            (void *) &m_domain);

        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferParticles],
            m_particles.size() * sizeof(m_particles[0]),
            (void *) &m_particles[0]);
    }
}

/**
 * Engine::teardown
 * @brief Teardown OpenCL data and cleanup object state.
 */
void Engine::teardown(void)
{
    /* Teardown model data. */
    {}

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
    }
}

/** ---------------------------------------------------------------------------
 * Engine::execute
 * @brief Engine integration step.
 */
void Engine::execute(void)
{
    /*
     * Copy particle positions onto the shared OpenGL vertex buffer object.
     */
    {
        /* Wait for OpenGL to finish and acquire the gl objects. */
        cl::gl::enqueue_acquire_gl_objects(
            m_queue, 1, &m_buffers[BufferVertexData], NULL, NULL);

        /* Enqueue the OpenCL kernel for execution. */
        cl::Kernel::set_arg(m_kernels[KernelCopyVertexData], 0, sizeof(cl_uint), (void *) &Params::n_particles);
        cl::Kernel::set_arg(m_kernels[KernelCopyVertexData], 1, sizeof(cl_mem), (void *) &m_buffers[BufferVertexData]);
        cl::Kernel::set_arg(m_kernels[KernelCopyVertexData], 2, sizeof(cl_mem), (void *) &m_buffers[BufferParticles]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelCopyVertexData],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_particles, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Wait for OpenCL to finish and release the gl objects. */
        cl::gl::enqueue_release_gl_objects(
            m_queue, 1, &m_buffers[BufferVertexData], NULL, NULL);
    }

    /*
     * Update particles.
     */
    {
        /* Apply boundary conditions and update particle positions. */
        cl::Kernel::set_arg(m_kernels[KernelUpdateBoundaries], 0, sizeof(cl_uint), (void *) &Params::n_particles);
        cl::Kernel::set_arg(m_kernels[KernelUpdateBoundaries], 1, sizeof(cl_float), (void *) &Params::friction_coeff);
        cl::Kernel::set_arg(m_kernels[KernelUpdateBoundaries], 2, sizeof(cl_float), (void *) &Params::elastic_coeff);
        cl::Kernel::set_arg(m_kernels[KernelUpdateBoundaries], 3, sizeof(cl_mem), (void *) &m_buffers[BufferParticles]);
        cl::Kernel::set_arg(m_kernels[KernelUpdateBoundaries], 4, sizeof(cl_mem), (void *) &m_buffers[BufferDomain]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelUpdateBoundaries],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_particles, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Clear the grid map. */
        cl::Kernel::set_arg(m_kernels[KernelClearGrid], 0, sizeof(cl_uint), (void *) &m_grid.capacity);
        cl::Kernel::set_arg(m_kernels[KernelClearGrid], 1, sizeof(cl_mem), (void *) &m_buffers[BufferGrid]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelClearGrid],
            cl::NDRange::Null,
            cl::NDRange::Make(m_grid.capacity, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Build the grid map of particle positions. */
        cl::Kernel::set_arg(m_kernels[KernelBuildGrid], 0, sizeof(cl_uint), (void *) &Params::n_particles);
        cl::Kernel::set_arg(m_kernels[KernelBuildGrid], 1, sizeof(cl_uint), (void *) &m_grid.capacity);
        cl::Kernel::set_arg(m_kernels[KernelBuildGrid], 2, sizeof(cl_float), (void *) &Params::kernel_radius);
        cl::Kernel::set_arg(m_kernels[KernelBuildGrid], 3, sizeof(cl_mem), (void *) &m_buffers[BufferGrid]);
        cl::Kernel::set_arg(m_kernels[KernelBuildGrid], 4, sizeof(cl_mem), (void *) &m_buffers[BufferParticles]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelBuildGrid],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_particles, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);
    }

    /*
     * Begin integration - first half of the integration step.
     */
    {
        /* Integrate the atoms at half time step. */
        cl::Kernel::set_arg(m_kernels[KernelBeginIntegrate], 0, sizeof(cl_float), (void *) &Params::t_step);
        cl::Kernel::set_arg(m_kernels[KernelBeginIntegrate], 1, sizeof(cl_uint), (void *) &Params::n_particles);
        cl::Kernel::set_arg(m_kernels[KernelBeginIntegrate], 2, sizeof(cl_mem), (void *) &m_buffers[BufferParticles]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelBeginIntegrate],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_particles, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);
    }

    /*
     * Compute fluid forces.
     */
    {
        /* Compute particle density and pressure.. */
        cl::Kernel::set_arg(m_kernels[KernelComputeDensity], 0, sizeof(cl_uint), (void *) &Params::n_particles);
        cl::Kernel::set_arg(m_kernels[KernelComputeDensity], 1, sizeof(cl_uint), (void *) &m_grid.capacity);
        cl::Kernel::set_arg(m_kernels[KernelComputeDensity], 2, sizeof(cl_float), (void *) &Params::kernel_radius);
        cl::Kernel::set_arg(m_kernels[KernelComputeDensity], 3, sizeof(cl_float), (void *) &Params::eos_kappa);
        cl::Kernel::set_arg(m_kernels[KernelComputeDensity], 4, sizeof(cl_float), (void *) &Params::eos_density);
        cl::Kernel::set_arg(m_kernels[KernelComputeDensity], 5, sizeof(cl_mem), (void *) &m_buffers[BufferParticles]);
        cl::Kernel::set_arg(m_kernels[KernelComputeDensity], 6, sizeof(cl_mem), (void *) &m_buffers[BufferGrid]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelComputeDensity],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_particles, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Compute pressure, viscosity and external forces. */
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 0, sizeof(cl_uint), (void *) &Params::n_particles);
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 1, sizeof(cl_uint), (void *) &m_grid.capacity);
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 2, sizeof(cl_float), (void *) &Params::kernel_radius);
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 3, sizeof(cl_float), (void *) &Params::viscosity);
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 4, sizeof(cl_float4), (void *) &m_gravity);
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 5, sizeof(cl_mem), (void *) &m_buffers[BufferParticles]);
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 6, sizeof(cl_mem), (void *) &m_buffers[BufferGrid]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelComputeForces],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_particles, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);
    }

    /*
     * End integration - second half of the integration step.
     */
    {
        /* Integrate the momenta for half time step. */
        cl::Kernel::set_arg(m_kernels[KernelEndIntegrate], 0, sizeof(cl_float), (void *) &Params::t_step);
        cl::Kernel::set_arg(m_kernels[KernelEndIntegrate], 1, sizeof(cl_uint), (void *) &Params::n_particles);
        cl::Kernel::set_arg(m_kernels[KernelEndIntegrate], 2, sizeof(cl_mem), (void *) &m_buffers[BufferParticles]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelEndIntegrate],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_particles, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);
    }

    /*
     * Next time step.
     */
    {
        if (m_step % 1000 == 0) {
            std::cout << m_step << "\n";
        }
        m_step++;
    }
}
