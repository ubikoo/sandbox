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
static const  cl_long width = 12288;            /* texture width */
static const  cl_long height = 12288;           /* texture height */
static const  cl_long maxiters = 512;           /* mandelbrot max iterations */
static const  cl_float2 xrange = {-2.0f, 2.0f}; /* mandelbrot domain xrange */
static const  cl_float2 yrange = {-2.0f, 2.0f}; /* mandelbrot domain yrange */

/* OpenGL parameters */
static const int window_width = 1024;
static const int window_height = 1024;
static const char window_title[] = "mandlebrot-zoom";
static const double poll_timeout = 0.01;

/* OpenCL parameters */
static const cl_ulong device_index = 2;         /* gpu device index. */
static const cl_ulong work_group_size = 16;     /* local workgroup size */
}

#endif /* BASE_H_ */
