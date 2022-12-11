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

#ifndef BASE_H_
#define BASE_H_

#include "atto/opencl/opencl.hpp"

namespace Params {
/* Model parameters */
static const size_t n_sites = 512;      /* number of lattice sites along each dimension */
static const size_t n_iters = 1024;     /* number of Monte Carlo iterations */
static const size_t n_equil = 8;        /* number of Monte Carlo equilibration steps */
static const size_t n_steps = 8;        /* number of Monte Carlo steps */
static const double ising_J = 1.0;      /* Ising energy coefficient */
static const double ising_h = 0.0;      /* Ising external field coefficient */
static const double ising_beta = 1.0;   /* Inverse reduced temperature */

/* OpenGL parameters */
static const int window_width = 1024;
static const int window_height = 1024;
static const char window_title[] = "ising2d";
static const double poll_timeout = 0.01;

/* OpenCL parameters */
// static const cl_ulong device_index = 2;
// static const cl_ulong work_group_size = 256;
} /* Params */

#endif /* BASE_H_ */
