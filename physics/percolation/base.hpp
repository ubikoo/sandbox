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
#include <unistd.h>
#include <pthread.h>

namespace Params {
/* Model parameters */
static const int32_t n_sites = 128;
static const uint64_t n_threads = 16;
static const uint64_t n_iters = 8192;

/* OpenGL parameters */
// static const int window_width = n_sites;
// static const int window_height = n_sites;
// static const char window_title[] = "Percolation";
// static const double poll_timeout = 0.01;

/* OpenCL parameters */
// static const cl_ulong n_points = 1048576;
// static const cl_ulong device_index = 2;
// static const cl_ulong work_group_size = 256;
// static const cl_ulong num_work_items = atto::cl::NDRange::Roundup(
//     n_points, work_group_size);
// static const cl_ulong num_work_groups = num_work_items / work_group_size;
}

#endif /* BASE_H_ */
