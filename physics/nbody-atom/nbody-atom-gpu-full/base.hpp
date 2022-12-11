/*
 * base.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef MD_BASE_H_
#define MD_BASE_H_

#include "atto/opencl/opencl.hpp"

/**
 * @brief Model parameters.
 */
namespace Params {
/* Simulation parameters. */
static const cl_double t_step = 0.005;          /* timestep size */
static const cl_ulong n_min_steps = 1000;       /* minimization steps */
static const cl_ulong n_run_steps = 1000;       /* number of run steps */
static const cl_ulong sample_frequency = 10;    /* sample frequency */
static const cl_ulong sample_block_size = 10;   /* sampler block size */

/* Engine parameters. */
static const cl_double density = 0.8;           /* fluid density */
static const cl_double temperature = 2.0;       /* fluid temperature */
static const cl_ulong n_atoms = 16384;          /* number of atoms */
static const cl_ulong n_neighbours = 256;       /* neighbours per atom */
static const cl_double atom_mass = 1.0;         /* atom mass */
static const cl_double pair_epsilon = 1.0;      /* energy coefficient */
static const cl_double pair_sigma = 1.0;        /* sigma coefficient */
static const cl_double pair_r_cut = 2.0;        /* cutoff radius */
static const cl_double pair_r_skin = 1.0;       /* skin radius */
static const cl_double pair_r_hard = 0.01;      /* hard sphere radius */
static const cl_double thermostat_mass = 10.0;  /* thermostat inertial mass */

/* OpenGL parameters */
static const int window_width = 1024;
static const int window_height = 1024;
static const char window_title[] = "md-gpu-full";
static const cl_double poll_timeout = 0.01;

/* OpenCL parameters */
static const cl_ulong device_index = 2;         /* gpu device index. */
static const cl_ulong work_group_size = 256;    /* local workgroup size */
static const cl_ulong num_work_items = atto::cl::NDRange::Roundup(
    n_atoms, work_group_size);
static const cl_ulong num_work_groups = num_work_items / work_group_size;
} /* Params */

/**
 * @brief Fluid atoms.
 */
struct Atom {
    cl_double mass;                 /* atom mass */
    cl_double rmass;                /* inverse mass */
    cl_double4 pos;                 /* periodic image in primary cell */
    cl_double4 upos;                /* unfolded position */
    cl_double4 mom;                 /* momentum */
    cl_double4 force;               /* force */
    cl_double energy;               /* energy */
    cl_double16 virial;             /* virial */
};

/**
 * @brief Fluid pairs.
 */
struct Pair {
    cl_ulong atom_1;                /* first atom index */
    cl_ulong atom_2;                /* second atom index */
    cl_double4 r_12;                /* pairwise vector */
    cl_double energy;               /* energy */
    cl_double4 gradient;            /* gradient */
    cl_double4 laplace;             /* laplace */
    cl_double16 virial;             /* virial */
};

/**
 * @brief Fluid domain.
 */
struct Domain {
    cl_double4 length;              /* domain period length*/
    cl_double4 length_half;         /* domain half length */
};

/**
 * @brief Field represents the Lennard-Jones force field.
 */
struct Field {
    cl_double epsilon;              /* LJ pair energy */
    cl_double sigma;                /* LJ pair size */
    cl_double r_cut;                /* LJ cutoff radius */
    cl_double r_skin;               /* Neighbour list skin radius */
    cl_double r_hard;               /* Hard sphere truncation radius */
};

/**
 * @brief Nose-Hoover thermostat.
 */
struct Thermostat {
    cl_double mass;                 /* mass */
    cl_double eta;                  /* velocity */
    cl_double deta_dt;              /* acceleration */
    cl_double temperature;          /* temperature */
};

/**
 * @brief Thermo holds a collection of thermodynamic properties.
 */
struct Thermo {
    cl_double com_mass;             /* Total mass of the fluid */
    cl_double4 com_pos;             /* CoM position of the fluid */
    cl_double4 com_upos;            /* CoM unfolded position of the fluid */
    cl_double4 com_vel;             /* CoM velocity of the fluid */
    cl_double4 com_mom;             /* Total momentum of the fluid */
    cl_double4 com_force;           /* Total force  of the fluid */
    cl_double density;              /* Mass density */
    cl_double energy_kin;           /* Kinetic energy */
    cl_double energy_pot;           /* Potential energy */
    cl_double temp_grad_sq;         /* Kinetic energy square gradient */
    cl_double temp_laplace;         /* Kinetic energy laplacian */
    cl_double temp_kinetic;         /* Kinetic temperature */
    cl_double16 pres_kinetic;       /* Kinetic pressure */
    cl_double16 pres_virial;        /* Virial pressure */
};

#endif /* MD_BASE_H_ */
