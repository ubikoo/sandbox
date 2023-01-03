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

#ifndef RAYMARCH_BASE_H_
#define RAYMARCH_BASE_H_

#include "atto/opencl/opencl.hpp"

/** ---------------------------------------------------------------------------
 * @brief Global parameters.
 */
namespace Params {
/* Model parameters */
static const cl_uint width = 1024;              /* camera film width */
static const cl_uint height = 512;             /* camera film height */
static const cl_float depth = 1.0;              /* camera film depth */

static const cl_float4 sphere_centre = cl_float4{0.0f, 1.0f, -6.0f, 0.0f};
static const cl_float sphere_radius = 1.0f;

static const cl_float t_min = 0.01;             /* raymarch min distance */
static const cl_float t_max = 100.0;             /* raymarch max distance */
static const cl_uint maxsteps = 1000;            /* raymarch max steps */

/* OpenGL parameters */
static const int window_width = 1024;
static const int window_height = 512;
static const char window_title[] = "sphere";
static const double poll_timeout = 0.01;

/* OpenCL parameters */
static const cl_ulong device_index = 2;         /* gpu device index. */
static const cl_ulong work_group_size = 16;     /* local workgroup size */
}

/** ---------------------------------------------------------------------------
 * @brief Sphere data type.
 */
struct Sphere {
    cl_float4 centre;
    cl_float radius;
};

/**
 * @brief Camera
 */
struct Camera {
    cl_float4 u;            /* local basis set */
    cl_float4 v;
    cl_float4 w;
    cl_float4 eye;          /* eye position */
    cl_float width;         /* -1 <= width <= 1 */
    cl_float height;        /* -1 <= height <= 1 */
    cl_float depth;         /* 0 <= depth */
};

/**
 * @brief Ray starting at origin o and with direction d.
 */
typedef struct {
    cl_float4 o;
    cl_float4 d;
} Ray_t;

#endif /* RAYMARCH_BASE_H_ */
