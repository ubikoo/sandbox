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

#include "atto/opencl/opencl.hpp"
#include "engine.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * Engine::setup
 * @brief Setup engine data.
 */
void Engine::setup(
    const cl_context &context,
    const cl_device_id &device,
    const cl_command_queue &queue,
    const GLuint &gl_vertex_buffer)
{
    /* ------------------------------------------------------------------------
     * Setup engine data.
     */
    {
        /* Reset integration step. */
        m_step = 0;
    }

    {
        /* Create fluid domain. */
        cl_double volume = (cl_double) Params::n_atoms / Params::density;
        cl_double length = std::pow(volume, 1.0 / 3.0);
        cl_double length_half = 0.5 * length;
        m_domain = Domain{
            .length = cl_double4{length, length, length, 0.0 /*unused*/},
            .length_half = cl_double4{
                length_half, length_half, length_half, 0.0 /*unused*/}
        };
    }

    {
        /* Create fluid field */
        m_field = Field{
            .epsilon = Params::pair_epsilon,
            .sigma = Params::pair_sigma,
            .r_cut = Params::pair_r_cut,
            .r_hard = Params::pair_r_hard
        };
    }

    {
        /* Setup thermostat */
        m_thermostat = Thermostat{
            .mass = Params::thermostat_mass,        /* mass */
            .eta = 0.0,                             /* velocity */
            .deta_dt = 0.0,                         /* acceleration */
            .temperature = Params::temperature};    /* temperature */
        std::cout << "m_thermostat.mass " << m_thermostat.mass << "\n"
                  << "m_thermostat.temperature " << m_thermostat.temperature << "\n";
    }

    {
        /* Create fluid atoms. */
        Atom atom = Atom{
            .mass = Params::atom_mass,
            .rmass = 1.0 / Params::atom_mass,
            .pos = cl_double4{},
            .upos = cl_double4{},
            .mom = cl_double4{},
            .force = cl_double4{},
            .energy = 0.0,
            .virial = cl_double16{}
        };
        m_atoms.resize(Params::n_atoms, atom);

        /* Generate atom positions at the specified density. */
        const cl_double epsilon = 0.9;  /* <= 1.0 */
        const cl_double4 half = m_domain.length_half * epsilon;
        std::vector<cl_double4> positions = generate::points_fcc(
            Params::n_atoms,
            -half.s[0],
            -half.s[1],
            -half.s[2],
             half.s[0],
             half.s[1],
             half.s[2]);

        cl_uint idx = 0;
        for (auto &atom : m_atoms) {
            atom.pos = positions[idx];
            atom.upos = positions[idx];
            idx++;
        }

        /* Generate momenta from a Maxwell-Boltzmann distribution. */
        math::rng::Kiss engine(true);           /* rng engine */
        math::rng::gauss<cl_double> rand;       /* rng sampler */

        for (auto &atom : m_atoms) {
            cl_double sdev = std::sqrt(Params::temperature * atom.mass);
            // atom.mom = cl_double4{};
            atom.mom = cl_double4{
                rand(engine, 0.0, sdev),
                rand(engine, 0.0, sdev),
                rand(engine, 0.0, sdev),
                0.0 /*unused*/
            };
        }

        /* Reset the fluid atom positions and momenta. */
        cl_double4 pos = compute::com_pos(m_atoms);
        cl_double4 upos = compute::com_upos(m_atoms);
        cl_double4 vel = compute::com_vel(m_atoms);
        for (auto &atom : m_atoms) {
            atom.pos  -= pos;
            atom.upos -= upos;
            atom.mom  -= vel * atom.mass;
        }
    }

    {
        /* Setup neighbour list parameters. */
        cl_double radius = Params::list_radius;
        cl_double skin = Params::list_radius - Params::pair_r_cut;
        cl_double volume = 4.0 * M_PI * radius * radius * radius / 3.0;

        cl_uint n_neighbours = (cl_uint) (Params::density * volume);
        n_neighbours *= Params::list_scale;
        cl_uint capacity = n_neighbours * Params::n_atoms;

        m_list = List{
            .radius = radius,
            .skin = skin,
            .n_neighbours = n_neighbours,
            .capacity = capacity,
        };
        std::cout << "m_list.radius " << m_list.radius << "\n"
                  << "m_list.skin " << m_list.skin << "\n"
                  << "m_list.n_neighbours " << m_list.n_neighbours << "\n"
                  << "m_list.capacity " << m_list.capacity << "\n";
    }

    {
        /* Setup grid map parameters. */
        cl_double4 length = m_domain.length;
        cl_uint4 n_cells = cl_uint4{
            (cl_uint) (length.s[0] / Params::pair_r_cut),
            (cl_uint) (length.s[1] / Params::pair_r_cut),
            (cl_uint) (length.s[2] / Params::pair_r_cut),
            0 /* unused */
        };

        cl_double cell_length = Params::pair_r_cut;
        cl_double cell_volume = cell_length * cell_length * cell_length;
        cl_uint n_nodes = (cl_uint) (Params::density * cell_volume);
        n_nodes *= Params::list_scale;
        cl_uint capacity = n_nodes * n_cells.s[0] * n_cells.s[1] * n_cells.s[2];

        m_grid = Grid{
            .length = length,
            .n_cells = n_cells,
            .n_nodes = n_nodes,
            .capacity = capacity
        };
        std::cout << "m_grid.length "
                  << m_grid.length.s[0] << " "
                  << m_grid.length.s[1] << " "
                  << m_grid.length.s[2] << "\n"
                  << "m_grid.n_cells "
                  << m_grid.n_cells.s[0] << " "
                  << m_grid.n_cells.s[1] << " "
                  << m_grid.n_cells.s[2] << "\n"
                  << "m_grid.n_nodes " << m_grid.n_nodes << "\n"
                  << "m_grid.capacity " << m_grid.capacity << "\n";
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
        source.append(cl::Program::load_source_from_file("data/atom.cl"));
        source.append(cl::Program::load_source_from_file("data/neighbour.cl"));
        source.append(cl::Program::load_source_from_file("data/thermostat.cl"));

        m_program = cl::Program::create_from_source(m_context, source);
        cl::Program::build(m_program, m_device, "");
        // std::cout << cl::Program::get_source(m_program) << "\n";

        /* Create engine kernels. */
        m_kernels.resize(NumKernels, NULL);

        m_kernels[KernelBeginIntegrate] =  cl::Kernel::create(m_program, "begin_integrate");
        m_kernels[KernelEndIntegrate] =  cl::Kernel::create(m_program, "end_integrate");
        m_kernels[KernelUpdateAtoms] =  cl::Kernel::create(m_program, "update_atoms");
        m_kernels[KernelComputeForces] =  cl::Kernel::create(m_program, "compute_forces");
        m_kernels[KernelCopyAtomPoints] =  cl::Kernel::create(m_program, "copy_atom_points");
        m_kernels[KernelThermostatForce] =  cl::Kernel::create(m_program, "thermostat_force");
        m_kernels[KernelThermostatIntegrate] =  cl::Kernel::create(m_program, "thermostat_integrate");
        m_kernels[KernelClearNList] =  cl::Kernel::create(m_program, "clear_nlist");
        m_kernels[KernelBuildNList] =  cl::Kernel::create(m_program, "build_nlist");

        /* Create engine device buffer objects. */
        m_buffers.resize(NumBuffers, NULL);

        m_buffers[BufferDomain] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            sizeof(Domain),
            (void *) NULL);

        m_buffers[BufferField] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            sizeof(Field),
            (void *) NULL);

        m_buffers[BufferAtoms] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            m_atoms.size() * sizeof(m_atoms[0]),
            (void *) NULL);

        m_buffers[BufferThermostat] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            sizeof(Thermostat),
            (void *) NULL);

        m_buffers[BufferThermostatGradSq] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            Params::num_work_groups * sizeof(cl_double),
            (void *) NULL);

        m_buffers[BufferThermostatLaplace] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            Params::num_work_groups * sizeof(cl_double),
            (void *) NULL);

        m_buffers[BufferNList] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            m_list.capacity * sizeof(cl_uint),
            (void *) NULL);

        m_buffers[BufferGLPointVbo] = cl::gl::create_from_gl_buffer(
            m_context,
            CL_MEM_WRITE_ONLY,
            gl_vertex_buffer);
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
            m_buffers[BufferField],
            sizeof(m_field),
            (void *) &m_field);

        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferAtoms],
            m_atoms.size() * sizeof(m_atoms[0]),
            (void *) &m_atoms[0]);

        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferThermostat],
            sizeof(m_thermostat),
            (void *) &m_thermostat);
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
    const cl_double t_step = Params::t_step;
    const cl_double half_t_step = 0.5 * Params::t_step;

    /*
     * Update atom positions and apply periodic boundary conditions.
     */
    {
        /* Update atom positions and associated data structures. */
        cl::Kernel::set_arg(m_kernels[KernelUpdateAtoms], 0, sizeof(cl_uint), (void *) &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelUpdateAtoms], 1, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelUpdateAtoms], 2, sizeof(cl_mem), &m_buffers[BufferDomain]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelUpdateAtoms],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);
    }

    /*
     * Update the neigbour list.
     */
    if (m_step % Params::list_freq == 0)
    {
        /* Clear the neighbour list. */
        cl::Kernel::set_arg(m_kernels[KernelClearNList], 0, sizeof(cl_uint), &m_list.capacity);
        cl::Kernel::set_arg(m_kernels[KernelClearNList], 1, sizeof(cl_mem), &m_buffers[BufferNList]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelClearNList],
            cl::NDRange::Null,
            cl::NDRange::Make(m_list.capacity, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Build the neighbour list. */
        cl::Kernel::set_arg(m_kernels[KernelBuildNList], 0, sizeof(cl_uint), (void *) &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelBuildNList], 1, sizeof(cl_uint), (void *) &m_list.n_neighbours);
        cl::Kernel::set_arg(m_kernels[KernelBuildNList], 2, sizeof(cl_double), (void *) &m_list.radius);
        cl::Kernel::set_arg(m_kernels[KernelBuildNList], 3, sizeof(cl_mem), &m_buffers[BufferNList]);
        cl::Kernel::set_arg(m_kernels[KernelBuildNList], 4, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelBuildNList], 5, sizeof(cl_mem), &m_buffers[BufferDomain]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelBuildNList],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);
    }

    /*
     * Begin integration - first half of the integration step.
     */
    {
        /* Integrate the atoms at half time step. */
        cl::Kernel::set_arg(m_kernels[KernelBeginIntegrate], 0, sizeof(cl_double), (void *) &t_step);
        cl::Kernel::set_arg(m_kernels[KernelBeginIntegrate], 1, sizeof(cl_uint), (void *) &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelBeginIntegrate], 2, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelBeginIntegrate], 3, sizeof(cl_mem), &m_buffers[BufferThermostat]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelBeginIntegrate],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Compute thermostat force using the updated atom momenta. */
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 0, sizeof(cl_uint), (void *) &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 1, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 2, sizeof(cl_mem), &m_buffers[BufferThermostatGradSq]);
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 3, sizeof(cl_mem), &m_buffers[BufferThermostatLaplace]);
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 4, Params::work_group_size * sizeof(cl_double), NULL);
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 5, Params::work_group_size * sizeof(cl_double), NULL);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelThermostatForce],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Integrate thermostat using the computed forces. */
        cl::Kernel::set_arg(m_kernels[KernelThermostatIntegrate], 0, sizeof(cl_double), (void *) &half_t_step);
        cl::Kernel::set_arg(m_kernels[KernelThermostatIntegrate], 1, sizeof(cl_mem), &m_buffers[BufferThermostatGradSq]);
        cl::Kernel::set_arg(m_kernels[KernelThermostatIntegrate], 2, sizeof(cl_mem), &m_buffers[BufferThermostatLaplace]);
        cl::Kernel::set_arg(m_kernels[KernelThermostatIntegrate], 3, sizeof(cl_mem), &m_buffers[BufferThermostat]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelThermostatIntegrate],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);
    }

    /*
     * Compute fluid forces.
     */
    {
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 0, sizeof(cl_uint), (void *) &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 1, sizeof(cl_uint), (void *) &m_list.n_neighbours);
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 2, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 3, sizeof(cl_mem), &m_buffers[BufferNList]);
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 4, sizeof(cl_mem), &m_buffers[BufferDomain]);
        cl::Kernel::set_arg(m_kernels[KernelComputeForces], 5, sizeof(cl_mem), &m_buffers[BufferField]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelComputeForces],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);
    }

    /*
     * End integration - second half of the integration step.
     */
    {
        /* Compute thermostat force using the updated atom momenta. */
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 0, sizeof(cl_uint), (void *) &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 1, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 2, sizeof(cl_mem), &m_buffers[BufferThermostatGradSq]);
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 3, sizeof(cl_mem), &m_buffers[BufferThermostatLaplace]);
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 4, Params::work_group_size * sizeof(cl_double), NULL);
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 5, Params::work_group_size * sizeof(cl_double), NULL);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelThermostatForce],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Integrate thermostat using the computed forces. */
        cl::Kernel::set_arg(m_kernels[KernelThermostatIntegrate], 0, sizeof(cl_double), (void *) &half_t_step);
        cl::Kernel::set_arg(m_kernels[KernelThermostatIntegrate], 1, sizeof(cl_mem), &m_buffers[BufferThermostatGradSq]);
        cl::Kernel::set_arg(m_kernels[KernelThermostatIntegrate], 2, sizeof(cl_mem), &m_buffers[BufferThermostatLaplace]);
        cl::Kernel::set_arg(m_kernels[KernelThermostatIntegrate], 3, sizeof(cl_mem), &m_buffers[BufferThermostat]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelThermostatIntegrate],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Integrate the momenta for half time step. */
        cl::Kernel::set_arg(m_kernels[KernelEndIntegrate], 0, sizeof(cl_double), (void *) &t_step);
        cl::Kernel::set_arg(m_kernels[KernelEndIntegrate], 1, sizeof(cl_uint), (void *) &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelEndIntegrate], 2, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelEndIntegrate], 3, sizeof(cl_mem), &m_buffers[BufferThermostat]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelEndIntegrate],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);
    }


    /*
     * Copy atom positions onto the shared OpenGL vertex buffer object.
     */
    {
        /* Wait for OpenGL to finish and acquire the gl objects. */
        cl::gl::enqueue_acquire_gl_objects(
            m_queue, 1, &m_buffers[BufferGLPointVbo], NULL, NULL);

        /* Enqueue the OpenCL kernel for execution. */
        cl::Kernel::set_arg(m_kernels[KernelCopyAtomPoints], 0, sizeof(cl_uint), (void *) &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelCopyAtomPoints], 1, sizeof(cl_mem), &m_buffers[BufferGLPointVbo]);
        cl::Kernel::set_arg(m_kernels[KernelCopyAtomPoints], 2, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelCopyAtomPoints],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Wait for OpenCL to finish and release the gl objects. */
        cl::gl::enqueue_release_gl_objects(
            m_queue, 1, &m_buffers[BufferGLPointVbo], NULL, NULL);
    }

    /*
     * Next time step.
     */
    {
        // std::cout << m_step << "\n";
        m_step++;
    }
}
