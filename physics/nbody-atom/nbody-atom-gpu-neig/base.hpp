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
/* Integration parameters */
static const cl_ulong n_steps = 1000;          /* number of run steps */
static const cl_double t_step = 0.005;          /* timestep size */

/* Fluid parameters */
static const cl_double density = 0.8;           /* fluid density */
static const cl_double temperature = 2.0;       /* fluid temperature */
static const cl_uint n_atoms = 16384;           /* number of atoms */
static const cl_double atom_mass = 1.0;         /* atom mass */

/* Force field parameters */
static const cl_double pair_epsilon = 1.0;      /* energy coefficient */
static const cl_double pair_sigma = 1.0;        /* sigma coefficient */
static const cl_double pair_r_cut = 2.0;        /* cutoff radius */
static const cl_double pair_r_hard = 0.01;      /* hard sphere radius */

/* Thermostat parameters */
static const cl_double thermostat_mass = 10.0;  /* thermostat inertial mass */

/* Neighbour list parameters */
static const cl_uint list_empty = 0xffffffff;
static const cl_uint list_freq = 10;            /* list update frequency */
static const cl_uint list_scale = 2;            /* scale factor >= 1 */
static const cl_double list_radius = 3.0;       /* neighbour range radius */

/* OpenGL parameters */
static const int window_width = 1024;
static const int window_height = 1024;
static const char window_title[] = "md-gpu-neig";
static const cl_double poll_timeout = 0.01;

/* OpenCL parameters */
static const cl_ulong device_index = 2;         /* gpu device index. */
static const cl_ulong work_group_size = 256;    /* local workgroup size */
static const cl_ulong num_work_items = atto::cl::NDRange::Roundup(
    n_atoms, work_group_size);
static const cl_ulong num_work_groups = num_work_items / work_group_size;
} /* Params */

/**
 * @brief Fluid domain.
 */
struct Domain {
    cl_double4 length;          /* domain period length*/
    cl_double4 length_half;     /* domain half length */
};

/**
 * @brief Field represents the Lennard-Jones force field.
 */
struct Field {
    cl_double epsilon;          /* LJ pair energy */
    cl_double sigma;            /* LJ pair size */
    cl_double r_cut;            /* LJ cutoff radius */
    cl_double r_hard;           /* LJ hard sphere radius */
};

/**
 * @brief Fluid atoms.
 */
struct Atom {
    cl_double mass;             /* atom mass */
    cl_double rmass;            /* inverse mass */
    cl_double4 pos;             /* periodic image in primary cell */
    cl_double4 upos;            /* unfolded position */
    cl_double4 mom;             /* momentum */
    cl_double4 force;           /* force */
    cl_double energy;           /* energy */
    cl_double16 virial;         /* virial */
};

/**
 * @brief Nose-Hoover thermostat.
 */
struct Thermostat {
    cl_double mass;             /* mass */
    cl_double eta;              /* velocity */
    cl_double deta_dt;          /* acceleration */
    cl_double temperature;      /* temperature */
};

/**
 * @brief Neighbour list parameters.
 */
struct List {
    cl_double radius;           /* list neighbour radius */
    cl_double skin;             /* list neighbour skin length */
    cl_uint n_neighbours;       /* neighbours per atom */
    cl_uint capacity;           /* total number of neighbour edges */
    cl_uint is_stale;
};

/**
 * @brief Uniform grid parameters. Each grid cell is an array of
 * key-value pairs representing the index of an atom whose key is a hash
 * function of the atom coordinates.
 */
struct Grid {
    struct Node {
        cl_uint key;            /* atom hash key */
        cl_uint atom;           /* atom index value */
    };
    cl_double4 length;          /* grid length along each dimension */
    cl_uint4 n_cells;           /* number of cells along each dimension  */
    cl_uint n_nodes;            /* number of grid nodes per cell */
    cl_uint capacity;           /* total number of nodes in the grid */
};

#endif /* MD_BASE_H_ */
