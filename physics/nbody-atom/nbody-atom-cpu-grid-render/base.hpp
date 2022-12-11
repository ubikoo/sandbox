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
static const double t_step = 0.005;             /* timestep size */
static const size_t n_min_steps = 1000;         /* minimization steps */
static const size_t n_run_steps = 1000;         /* number of run steps */
static const size_t sample_frequency = 10;      /* sample frequency */
static const size_t sample_block_size = 10;     /* sampler block size */

/* Engine parameters. */
static const double density = 0.8;              /* fluid density */
static const double temperature = 2.0;          /* fluid temperature */
static const size_t n_atoms = 4096;             /* number of atoms */
static const size_t n_neighbours = 256;         /* neighbours per atom */
static const double atom_mass = 1.0;            /* atom mass */
static const double pair_epsilon = 1.0;         /* energy coefficient */
static const double pair_sigma = 1.0;           /* sigma coefficient */
static const double pair_r_cut = 2.0;           /* cutoff radius */
static const double pair_r_skin = 1.0;          /* skin radius */
static const double pair_r_hard = 0.01;         /* hard sphere radius */
static const double thermostat_mass = 10.0;     /* thermostat inertial mass */

/* OpenGL parameters */
static const int window_width = 1024;
static const int window_height = 1024;
static const char window_title[] = "md-cpu-grid-render";
static const double poll_timeout = 0.01;
} /* Params */

/**
 * @brief Fluid atoms.
 */
struct Atom {
    double mass;                    /* atom mass */
    double rmass;                   /* inverse mass */
    atto::math::vec3d pos;          /* periodic image in primary cell */
    atto::math::vec3d upos;         /* unfolded position */
    atto::math::vec3d mom;          /* momentum */
    atto::math::vec3d force;        /* force */
    double energy;                  /* energy */
    atto::math::mat3d virial;       /* virial */
};

/**
 * @brief Fluid pairs.
 */
struct Pair {
    size_t atom_1;                  /* first atom index */
    size_t atom_2;                  /* second atom index */
    atto::math::vec3d r_12;         /* pairwise vector */
    double energy;                  /* energy */
    atto::math::vec3d gradient;     /* gradient */
    atto::math::vec3d laplace;      /* laplace */
    atto::math::mat3d virial;       /* virial */
};

/**
 * @brief Fluid domain.
 */
struct Domain {
    atto::math::vec3d length;       /* domain period length*/
    atto::math::vec3d length_half;  /* domain half length */
};

/**
 * @brief Field represents the Lennard-Jones force field.
 */
struct Field {
    double epsilon;                 /* LJ pair energy */
    double sigma;                   /* LJ pair size */
    double r_cut;                   /* LJ cutoff radius */
    double r_skin;                  /* Neighbour list skin radius */
    double r_hard;                  /* Hard sphere truncation radius */
};

/**
 * @brief Nose-Hoover thermostat.
 */
struct Thermostat {
    double mass;                    /* mass */
    double eta;                     /* velocity */
    double deta_dt;                 /* acceleration */
    double temperature;             /* temperature */
};

/**
 * @brief Thermo holds a collection of thermodynamic properties.
 */
struct Thermo {
    double com_mass;                /* Total mass of the fluid */
    atto::math::vec3d com_pos;      /* CoM position of the fluid */
    atto::math::vec3d com_upos;     /* CoM unfolded position of the fluid */
    atto::math::vec3d com_vel;      /* CoM velocity of the fluid */
    atto::math::vec3d com_mom;      /* Total momentum of the fluid */
    atto::math::vec3d com_force;    /* Total force  of the fluid */
    double density;                 /* Mass density */
    double energy_kin;              /* Kinetic energy */
    double energy_pot;              /* Potential energy */
    double temp_grad_sq;            /* Kinetic energy square gradient */
    double temp_laplace;            /* Kinetic energy laplacian */
    double temp_kinetic;            /* Kinetic temperature */
    atto::math::mat3d pres_kinetic; /* Kinetic pressure */
    atto::math::mat3d pres_virial;  /* Virial pressure */
};

#endif /* MD_BASE_H_ */
