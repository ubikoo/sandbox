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

#ifndef SPH_BASE_H_
#define SPH_BASE_H_

#include "atto/opencl/opencl.hpp"

/**
 * @brief Global parameters.
 */
namespace Params {
/* Model parameters */
static const cl_ulong n_steps = 1000000;        /* number of run steps */
static const cl_float t_step = 0.005;           /* timestep size */
static const cl_uint n_particles = 32768;       /* number of particles */
static const cl_float particle_mass = 1.0;      /* particle mass */

static const cl_float eos_kappa = 20.0;         /* eos compressibility coeff */
static const cl_float eos_density = 1.0;        /* eos reference density */
static const cl_float viscosity = 1.0;          /* viscosity coefficient */
static const cl_float gravity_coeff = -10.0;    /* gravity constant */
static const cl_float friction_coeff = 1.0f;    /* collision friction coefficient */
static const cl_float elastic_coeff = 1.0f;     /* collision elastic coefficient */

static const cl_float kernel_radius = 1.0;      /* kernel interpolation radius */
static const cl_float max_density = 2.5;        /* tone mapping max density */

static const cl_uint k_empty = 0xffffffff;      /* empty cell mask */

/* OpenGL parameters */
static const int window_width = 1024;
static const int window_height = 1024;
static const char window_title[] = "sph-gpu-full-pbc";
static const double poll_timeout = 0.01;

/* OpenCL parameters */
static const cl_ulong device_index = 2;         /* gpu device index. */
static const cl_ulong work_group_size = 256;    /* local workgroup size */
static const cl_ulong num_work_items = atto::cl::NDRange::Roundup(
    n_particles, work_group_size);
static const cl_ulong num_work_groups = num_work_items / work_group_size;
} /* Params */

/**
 * @brief Plane boundary.
 */
struct Domain{
    cl_float4 bound_lo;     /* domain lower bound */
    cl_float4 bound_hi;     /* domain upper bound */
};

/**
 * @brief Fluid particles.
 */
struct Particle {
    cl_float4 prev;         /* previous position */
    cl_float4 pos;          /* position */
    cl_float4 vel;          /* velocity */
    cl_float4 force;        /* force */
    cl_float mass;          /* mass */
    cl_float dens;          /* density */
    cl_float pres;          /* pressure from EoS, p = p(rho) */
};

#endif /* SPH_BASE_H_ */
