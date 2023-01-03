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
static const cl_long n_sites = 1024;        /* number of lattice sites along each dimension */
static const cl_long n_steps = 256;         /* number of Monte Carlo steps */
static const cl_double ising_J = 1.0;       /* Ising energy coefficient */
static const cl_double ising_h = 0.0;       /* Ising external field coefficient */
static const cl_double ising_beta = 1.0;    /* Inverse reduced temperature */
static const cl_long image_width = 1024;    /* bitmap width in pixels */
static const cl_long image_height = 1024;   /* bitmap height in pixels */
static const cl_long image_bpp = 32;        /* pixel bit depth */

/* OpenGL parameters */
static const int window_width = 1024;
static const int window_height = 1024;
static const char window_title[] = "ising2d";
static const double poll_timeout = 0.01;

/* OpenCL parameters */
static const cl_ulong device_index = 2;     /* gpu device index. */
static const cl_ulong work_group_size = 16; /* local workgroup size */
}

#endif /* BASE_H_ */
