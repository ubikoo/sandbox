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
        /* Create fluid atoms. */
        m_atoms.resize(Params::n_atoms, Atom{
            .mass = Params::atom_mass,          /* atom mass */
            .rmass = 1.0 / Params::atom_mass,   /* inverse mass */
            .pos = cl_double4{},                /* periodic image in primary cell */
            .upos = cl_double4{},               /* unfolded position */
            .mom = cl_double4{},                /* momentum */
            .force = cl_double4{}               /* force */
        });

        /* Create fluid domain. */
        cl_double volume = (cl_double) Params::n_atoms / Params::density;
        cl_double length = std::pow(volume, 1.0 / 3.0);
        m_domain = Domain{
            cl_double4{length, length, length, 0.0 /*unused*/},
            cl_double4{0.5*length, 0.5*length, 0.5*length, 0.0 /*unused*/}};

        /* Create fluid force field. */
        m_field = Field{
            Params::pair_epsilon,
            Params::pair_sigma,
            Params::pair_r_cut * Params::pair_sigma,
            Params::pair_r_skin * Params::pair_sigma,
            Params::pair_r_hard * Params::pair_sigma};

        /* Setup thermostat */
        m_thermostat = Thermostat{
            .mass = Params::thermostat_mass,        /* mass */
            .eta = 0.0,                             /* velocity */
            .deta_dt = 0.0,                         /* acceleration */
            .temperature = Params::temperature};    /* temperature */

        /* Setup thermo data */
        m_thermo = Thermo{
            .com_mass = 0.0,                /* Total mass */
            .com_pos = cl_double4{},        /* CoM position */
            .com_upos = cl_double4{},       /* CoM unfolded position */
            .com_vel = cl_double4{},        /* CoM velocity */
            .com_mom = cl_double4{},        /* Total momentum */
            .com_force = cl_double4{},      /* Total force  */
            .density = 0.0,                 /* Mass density */
            .energy_kin = 0.0,              /* Kinetic energy */
            .energy_pot = 0.0,              /* Potential energy */
            .temp_grad_sq = 0.0,            /* Kinetic energy square gradient */
            .temp_laplace = 0.0,            /* Kinetic energy laplacian */
            .temp_kinetic = 0.0,            /* Kinetic temperature */
            .pres_kinetic = cl_double16{},  /* Kinetic pressure */
            .pres_virial = cl_double16{}};   /* Virial pressure */

        /* Setup grid spatial data structure */
        m_grid = Grid(m_domain.length, Params::pair_r_cut, Params::n_neighbours);
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
        source.append(cl::Program::load_source_from_file("data/atom.cl"));
        source.append(cl::Program::load_source_from_file("data/thermostat.cl"));
        m_program = cl::Program::create_from_source(m_context, source);
        cl::Program::build(m_program, m_device, "");
        std::cout << cl::Program::get_source(m_program) << "\n";

        /* Create engine kernels. */
        m_kernels.resize(NumKernels, NULL);
        m_kernels[KernelAtomBeginIntegrate] = cl::Kernel::create(m_program, "atom_begin_integrate");
        m_kernels[KernelAtomEndIntegrate] = cl::Kernel::create(m_program, "atom_end_integrate");
        m_kernels[KernelAtomUpdate] = cl::Kernel::create(m_program, "atom_update");
        m_kernels[KernelAtomForce] = cl::Kernel::create(m_program, "atom_force");
        m_kernels[KernelAtomCopyVertex]= cl::Kernel::create(m_program, "atom_copy_vertex");
        m_kernels[KernelThermostatForce] = cl::Kernel::create(m_program, "thermostat_force");
        m_kernels[KernelThermostatIntegrate] = cl::Kernel::create(m_program, "thermostat_integrate");
        m_kernels[KernelGridClear] = cl::Kernel::create(m_program, "grid_clear");
        m_kernels[KernelGridInsert] = cl::Kernel::create(m_program, "grid_insert");

        /* Create engine device buffer objects. */
        m_buffers.resize(NumBuffers, NULL);
        m_buffers[BufferAtoms] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            Params::n_atoms * sizeof(Atom),
            (void *) NULL);

        m_buffers[BufferField] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            sizeof(Field),
            (void *) NULL);

        m_buffers[BufferDomain] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            sizeof(Domain),
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

        m_buffers[BufferGrid] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            m_grid.m_capacity * sizeof(Grid::Item),
            (void *) NULL);

        m_buffers[BufferGLVertexAtom] = cl::gl::create_from_gl_buffer(
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
            m_buffers[BufferAtoms],
            m_atoms.size() * sizeof(m_atoms[0]),
            (void *) &m_atoms[0]);

        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferField],
            sizeof(m_field),
            (void *) &m_field);

        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferDomain],
            sizeof(m_domain),
            (void *) &m_domain);

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
    {
        /* Write xyz snapshot and sampler statistics. */
        core::FileOut fileout;

        fileout.open("/tmp/out.xyz");
        io::write_xyz(m_atoms, "model", fileout);
        fileout.close();

        m_sampler.statistics();
        fileout.open("/tmp/out.sampler");
        fileout.writeline(m_sampler.to_string());
        fileout.close();
        std::cout << m_sampler.to_string() << "\n";
    }

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
    const cl_double half_t_step = 0.5 * Params::t_step;

    /*
     * Update fluid state and associated data structures.
     */
    {
        /* Update atom positions and associated data structures. */
        cl::Kernel::set_arg(m_kernels[KernelAtomUpdate], 0, sizeof(cl_ulong), &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelAtomUpdate], 1, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelAtomUpdate], 2, sizeof(cl_mem), &m_buffers[BufferDomain]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelAtomUpdate],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Clear all key-value pairs in the grid and reset their count. */
        cl::Kernel::set_arg(m_kernels[KernelGridClear], 0, sizeof(cl_uint), &m_grid.m_capacity);
        cl::Kernel::set_arg(m_kernels[KernelGridClear], 1, sizeof(cl_mem), &m_buffers[BufferGrid]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelGridClear],
            cl::NDRange::Null,
            cl::NDRange::Make(m_grid.m_capacity, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Insert the atom positions into the grid. */
        cl::Kernel::set_arg(m_kernels[KernelGridInsert], 0, sizeof(cl_ulong), &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelGridInsert], 1, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelGridInsert], 2, sizeof(cl_double4), &m_grid.m_length);
        cl::Kernel::set_arg(m_kernels[KernelGridInsert], 3, sizeof(cl_int4), &m_grid.m_cells);
        cl::Kernel::set_arg(m_kernels[KernelGridInsert], 4, sizeof(cl_uint), &m_grid.m_items);
        cl::Kernel::set_arg(m_kernels[KernelGridInsert], 5, sizeof(cl_uint), &m_grid.m_capacity);
        cl::Kernel::set_arg(m_kernels[KernelGridInsert], 6, sizeof(cl_mem), &m_buffers[BufferGrid]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelGridInsert],
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
        cl::Kernel::set_arg(m_kernels[KernelAtomBeginIntegrate], 0, sizeof(cl_double), (void *) &Params::t_step);
        cl::Kernel::set_arg(m_kernels[KernelAtomBeginIntegrate], 1, sizeof(cl_ulong), &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelAtomBeginIntegrate], 2, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelAtomBeginIntegrate], 3, sizeof(cl_mem), &m_buffers[BufferThermostat]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelAtomBeginIntegrate],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Compute thermostat force using the updated atom momenta. */
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 0, sizeof(cl_ulong), &Params::n_atoms);
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
        cl::Kernel::set_arg(m_kernels[KernelAtomForce], 0, sizeof(cl_ulong), &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelAtomForce], 1, sizeof(cl_ulong), &Params::n_neighbours);
        cl::Kernel::set_arg(m_kernels[KernelAtomForce], 2, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelAtomForce], 3, sizeof(cl_mem), &m_buffers[BufferDomain]);
        cl::Kernel::set_arg(m_kernels[KernelAtomForce], 4, sizeof(cl_mem), &m_buffers[BufferField]);
        cl::Kernel::set_arg(m_kernels[KernelAtomForce], 5, sizeof(cl_double4), &m_grid.m_length);
        cl::Kernel::set_arg(m_kernels[KernelAtomForce], 6, sizeof(cl_int4), &m_grid.m_cells);
        cl::Kernel::set_arg(m_kernels[KernelAtomForce], 7, sizeof(cl_uint), &m_grid.m_items);
        cl::Kernel::set_arg(m_kernels[KernelAtomForce], 8, sizeof(cl_uint), &m_grid.m_capacity);
        cl::Kernel::set_arg(m_kernels[KernelAtomForce], 9, sizeof(cl_mem), &m_buffers[BufferGrid]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelAtomForce],
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
        cl::Kernel::set_arg(m_kernels[KernelThermostatForce], 0, sizeof(cl_ulong), &Params::n_atoms);
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
        cl::Kernel::set_arg(m_kernels[KernelAtomEndIntegrate], 0, sizeof(cl_double), (void *) &Params::t_step);
        cl::Kernel::set_arg(m_kernels[KernelAtomEndIntegrate], 1, sizeof(cl_ulong), &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelAtomEndIntegrate], 2, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelAtomEndIntegrate], 3, sizeof(cl_mem), &m_buffers[BufferThermostat]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelAtomEndIntegrate],
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
        cl::gl::enqueue_acquire_gl_objects(m_queue, 1, &m_buffers[BufferGLVertexAtom], NULL, NULL);

        /* Enqueue the OpenCL kernel for execution. */
        cl::Kernel::set_arg(m_kernels[KernelAtomCopyVertex],
            0, sizeof(cl_ulong), &Params::n_atoms);
        cl::Kernel::set_arg(m_kernels[KernelAtomCopyVertex],
            1, sizeof(cl_mem), &m_buffers[BufferAtoms]);
        cl::Kernel::set_arg(m_kernels[KernelAtomCopyVertex],
            2, sizeof(cl_mem), &m_buffers[BufferGLVertexAtom]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelAtomCopyVertex],
            cl::NDRange::Null,
            cl::NDRange::Make(Params::n_atoms, Params::work_group_size),
            cl::NDRange(Params::work_group_size),
            NULL,
            NULL);

        /* Wait for OpenCL to finish and release the gl objects. */
        cl::gl::enqueue_release_gl_objects(m_queue, 1, &m_buffers[BufferGLVertexAtom], NULL, NULL);
    }
}

/**
 * Engine::sample
 * @brief Sample fluid thermodynamic properties.
 */
std::string Engine::sample(void)
{
    cl::Queue::enqueue_copy_from(
        m_queue,
        m_buffers[BufferAtoms],
        m_atoms.size() * sizeof(m_atoms[0]),
        (void *) &m_atoms[0]);

    m_sampler.sample(m_atoms, m_domain);

    return m_sampler.log_string();
}

/** ---------------------------------------------------------------------------
 * Engine::generate
 * @brief Generate atom positions and momenta.
 */
void Engine::generate(void)
{
    /* Generate atom positions at the specified density. */
    {
        const cl_double epsilon = 0.9;
        const cl_double4 half = m_domain.length_half * epsilon;
        std::vector<cl_double4> positions = generate::points_fcc(
            Params::n_atoms,
            -half.s[0],
            -half.s[1],
            -half.s[2],
            half.s[0],
            half.s[1],
            half.s[2]);

        cl_ulong ix = 0;
        for (auto &pos : positions) {
            m_atoms[ix].pos = pos;
            m_atoms[ix].upos = pos;
            ix++;
        }
    }

    /*
     * Generate atom momenta from a Maxwell-Boltzmann distribution with zero
     * mean velocity and a standard devitation corresponding to the specified
     * temperature.
     */
    {
        math::rng::Kiss engine(true);           /* rng engine */
        math::rng::gauss<cl_double> rand;       /* rng sampler */

        for (auto &atom : m_atoms) {
            cl_double sdev = std::sqrt(Params::temperature * atom.mass);
            atom.mom = cl_double4{
                rand(engine, 0.0, sdev),
                rand(engine, 0.0, sdev),
                rand(engine, 0.0, sdev),
                0.0 /*unused*/};
        }
    }

    /*
     * Copy atom positions and momenta to the device.
     */
    cl::Queue::enqueue_copy_to(
        m_queue,
        m_buffers[BufferAtoms],
        m_atoms.size() * sizeof(m_atoms[0]),
        (void *) &m_atoms[0]);

}

/** ---------------------------------------------------------------------------
 * Engine::reset
 * @brief Reset the engine state.
 */
void Engine::reset(const cl_double radius)
{
    /* Reset field hard sphere cutoff radius */
    {
        m_field.r_hard = radius;
    }

    /*
     * Reset the fluid atom positions and momenta to the correct density
     * and temperature.
     */
    {
        /*
         * Copy the atoms from the device.
         */
        cl::Queue::enqueue_copy_from(
            m_queue,
            m_buffers[BufferAtoms],
            m_atoms.size() * sizeof(m_atoms[0]),
            (void *) &m_atoms[0]);

        /*
         * Reset the CoM positions and momenta.
         */
        cl_double4 pos = compute::com_pos(m_atoms);
        cl_double4 upos = compute::com_upos(m_atoms);
        cl_double4 vel = compute::com_vel(m_atoms);
        for (auto &atom : m_atoms) {
            atom.pos  -= pos;
            atom.upos -= upos;
            atom.mom  -= vel * atom.mass;
        }

        /*
         * Scale fluid positions and momenta.
         */
        cl_double density_cur = compute::density(m_atoms, m_domain);
        cl_double density_scale = std::pow(density_cur / Params::density, 1.0 / 3.0);

        cl_double grad_sq, laplace;
        cl_double temperature_cur = compute::temperature_kin(m_atoms, grad_sq, laplace);
        cl_double temperature_scale = std::sqrt(Params::temperature / temperature_cur);

        /*
         * Scale domain dimensions.
         */
        m_domain.length *= density_scale;
        m_domain.length_half *= density_scale;

        /*
         * Scale positions and momenta.
         */
        for (auto &atom : m_atoms) {
            atom.pos  *= density_scale;
            atom.upos *= density_scale;
            atom.mom  *= temperature_scale;
        }

        /*
         * Copy updated atoms and domain to the device.
         */
        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferAtoms],
            m_atoms.size() * sizeof(m_atoms[0]),
            (void *) &m_atoms[0]);

        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferDomain],
            sizeof(m_domain),
            (void *) &m_domain);
    }

    /*
     * Reset thermostat state.
     */
    {
        m_thermostat.eta = 0.0;     /* velocity */
        m_thermostat.deta_dt = 0.0; /* acceleration */

        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferThermostat],
            sizeof(m_thermostat),
            (void *) &m_thermostat);
    }

    /*
     * Reset sampler properties.
     */
    m_sampler.reset();
}
