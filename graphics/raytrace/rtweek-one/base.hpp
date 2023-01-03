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

#ifndef RAYTRACE_CPU_WEEK_1_BASE_H_
#define RAYTRACE_CPU_WEEK_1_BASE_H_

#include "atto/opencl/opencl.hpp"

namespace Params {
/* Engine parameters. */
static const uint32_t width = 1024;
static const uint32_t height = 576;
static const atto::math::vec3d eye{13.0, 2.0, 3.0};
static const atto::math::vec3d ctr{ 0.0, 0.0, 0.0};
static const atto::math::vec3d up{  0.0, 1.0, 0.0};
static const double fov = 20.0;
static const size_t max_depth = 64;
static const size_t num_samples = 128;

/* OpenGL parameters */
static const int window_width = 768;
static const int window_height = 400;
static const char window_title[] = "raytrace-cpu-week-1";
static const double poll_timeout = 0.01;

/* OpenCL parameters */
// static const cl_ulong n_items = 1024;
// static const cl_ulong device_index = 2;
// static const cl_ulong work_group_size = 256;
// static const cl_ulong num_work_items = atto::cl::NDRange::Roundup(
//     n_items, work_group_size);
// static const cl_ulong num_work_groups = num_work_items / work_group_size;
}

#endif /* RAYTRACE_CPU_WEEK_1_BASE_H_ */
