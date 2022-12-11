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
void Engine::setup(void)
{
    /* Create fluid atoms. */
    m_atoms.resize(Params::n_atoms, Atom{
        .mass = Params::atom_mass,          /* atom mass */
        .rmass = 1.0 / Params::atom_mass,   /* inverse mass */
        .pos = math::vec3d{},               /* periodic image in primary cell */
        .upos = math::vec3d{},              /* unfolded position */
        .mom = math::vec3d{},               /* momentum */
        .force = math::vec3d{}});           /* force */

    /* Create fluid domain. */
    double volume = (double) Params::n_atoms / Params::density;
    double length = std::pow(volume, 1.0 / 3.0);
    m_domain = Domain{
        math::vec3d{length, length, length},
        math::vec3d{0.5*length, 0.5*length, 0.5*length}};

    /* Create fluid force field. */
    m_field = Field{
        Params::pair_epsilon,
        Params::pair_sigma,
        Params::pair_r_cut * Params::pair_sigma,
        Params::pair_r_skin * Params::pair_sigma,
        Params::pair_r_hard * Params::pair_sigma};

    /* Setup thermostat */
    m_thermostat = Thermostat{
        .mass = Params::thermostat_mass,     /* mass */
        .eta = 0.0,                         /* velocity */
        .deta_dt = 0.0,                     /* acceleration */
        .temperature = Params::temperature}; /* temperature */

    /* Setup thermo data */
    m_thermo = Thermo{
        .com_mass = 0.0,                    /* Total mass */
        .com_pos = math::vec3d{},           /* CoM position */
        .com_upos = math::vec3d{},          /* CoM unfolded position */
        .com_vel = math::vec3d{},           /* CoM velocity */
        .com_mom = math::vec3d{},           /* Total momentum */
        .com_force = math::vec3d{},         /* Total force  */
        .density = 0.0,                     /* Mass density */
        .energy_kin = 0.0,                  /* Kinetic energy */
        .energy_pot = 0.0,                  /* Potential energy */
        .temp_grad_sq = 0.0,                /* Kinetic energy square gradient */
        .temp_laplace = 0.0,                /* Kinetic energy laplacian */
        .temp_kinetic = 0.0,                /* Kinetic temperature */
        .pres_kinetic = math::mat3d{},      /* Kinetic pressure */
        .pres_virial = math::mat3d{}};      /* Virial pressure */
}

/**
 * Engine::teardown
 * @brief Teardown OpenCL data and cleanup object state.
 */
void Engine::teardown(void)
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

/** ---------------------------------------------------------------------------
 * Engine::execute
 * @brief Engine integration step.
 */
void Engine::execute(void)
{
    const double half_t_step = 0.5 * Params::t_step;

    /*
     * Update fluid state and associated data structures.
     */
    {
        /* Apply pbc to the fluid particle positions and reset atom forces. */
        for (auto &atom : m_atoms) {
            atom.pos = compute::pbc(atom.pos, m_domain);
            atom.force = math::vec3d{};
        }
    }

    /*
     * Begin integration - first half of the integration step.
     */
    {
        /* Integrate the atoms at half time step. */
        double exp_eta = exp(-m_thermostat.eta * half_t_step);
        for (auto &atom : m_atoms) {
            atom.mom += atom.force * half_t_step;
            atom.mom *= exp_eta;

            atom.pos += atom.mom * atom.rmass * Params::t_step;
            atom.upos += atom.mom * atom.rmass * Params::t_step;
        }

        /* Integrate thermostat half time step. */
        double grad_sq;
        double laplace;
        compute::temperature_kin(m_atoms, grad_sq, laplace);
        double force = grad_sq - m_thermostat.temperature * laplace;
        m_thermostat.deta_dt = force / m_thermostat.mass;
        m_thermostat.eta += half_t_step * m_thermostat.deta_dt;
    }

    /*
     * Compute fluid forces.
     */
    {
        core_pragma_omp(parallel for default(none) \
            shared(m_atoms, m_domain, m_field) schedule(dynamic))
        for (size_t atom_ix = 0; atom_ix < Params::n_atoms; ++atom_ix) {
            compute::force_atom(
                atom_ix,
                Params::n_atoms,
                Params::n_neighbours,
                m_atoms,
                m_domain,
                m_field);
        }
    }

    /*
     * End integration - second half of the integration step.
     */
    {
        /* Integrate thermostat half time step. */
        double grad_sq;
        double laplace;
        compute::temperature_kin(m_atoms, grad_sq, laplace);
        double force = grad_sq - m_thermostat.temperature * laplace;
        m_thermostat.deta_dt = force / m_thermostat.mass;
        m_thermostat.eta += half_t_step * m_thermostat.deta_dt;

        /* Integrate momenta half time step. */
        double exp_eta = exp(-m_thermostat.eta * half_t_step);
        for (auto &atom : m_atoms) {
            atom.mom *= exp_eta;
            atom.mom += atom.force * half_t_step;
        }
    }
}

/**
 * Engine::sample
 * @brief Sample fluid thermodynamic properties.
 */
std::string Engine::sample(void)
{
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
        const double epsilon = 0.9;
        const math::vec3d half = m_domain.length_half * epsilon;
        std::vector<math::vec3d> positions = generate::points_fcc(
            Params::n_atoms, -half.x, -half.y, -half.z, half.x, half.y, half.z);

        size_t ix = 0;
        for (auto &pos : positions) {
            m_atoms[ix].pos = pos;
            m_atoms[ix].upos = pos;
            ix++;
        }
    }

    /*
     * Generate atom momenta from a Maxwell-Boltzmann distribution with zero
     * mean velocity and a standard devitation corresponding to the specied
     * temperature.
     */
    {
        math::rng::Kiss engine(true);       /* rng engine */
        math::rng::gauss<double> rand;      /* rng sampler */

        for (auto &atom : m_atoms) {
            double sdev = std::sqrt(Params::temperature * atom.mass);
            atom.mom = math::vec3d{
                rand(engine, 0.0, sdev),
                rand(engine, 0.0, sdev),
                rand(engine, 0.0, sdev)};
        }
    }
}

/** ---------------------------------------------------------------------------
 * Engine::reset
 * @brief Reset the engine state.
 */
void Engine::reset(const double radius)
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
         * Reset the CoM positions and momenta.
         */
        math::vec3d pos = compute::com_pos(m_atoms);
        math::vec3d upos = compute::com_upos(m_atoms);
        math::vec3d vel = compute::com_vel(m_atoms);
        for (auto &atom : m_atoms) {
            atom.pos  -= pos;
            atom.upos -= upos;
            atom.mom  -= vel * atom.mass;
        }

        /*
         * Scale fluid positions and momenta.
         */
        double density_cur = compute::density(m_atoms, m_domain);
        double density_scale = std::pow(density_cur / Params::density, 1.0 / 3.0);

        double grad_sq, laplace;
        double temperature_cur = compute::temperature_kin(m_atoms, grad_sq, laplace);
        double temperature_scale = std::sqrt(Params::temperature / temperature_cur);

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
    }

    /*
     * Reset thermostat state.
     */
    {
        m_thermostat.eta = 0.0;     /* velocity */
        m_thermostat.deta_dt = 0.0; /* acceleration */
    }

    /*
     * Reset sampler properties.
     */
    m_sampler.reset();
}
