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

/**
 * @brief Global paramters
 */
namespace Params {

/* Model parameters */
static const cl_double t_step = 0.02;           /* Integration time step size */
static const cl_double max_err = 1.0E-7;        /* Midpoint max error */
static const cl_uint max_iter = 5;             /* Midpoing max iterations */

static const cl_uint canvas_width = 1024;       /* canvas horizontal intervals */
static const cl_uint canvas_height = 1024;      /* canvas vertical intervals */

static const cl_double canvas_x_range = 12.0;   /* canvas roi width */
static const cl_double canvas_y_range = 12.0;   /* convas roi height */

static const cl_double init_x_range = 4.0;      /* thermostat init horizontal range */
static const cl_double init_y_range = 4.0;      /* thermostat init vertical range */

static const cl_uint n_thermostats = canvas_width * canvas_height;

static const cl_double mass = 1.0;              /* Nose-Hoover mass */
static const cl_double kappa = 1.0;             /* Nose-Hoover force constant */
static const cl_double tau = 1.0;               /* Nose-Hoover thermostat constant */
static const cl_double temperature = 1.0;       /* Nose-Hoover thermostat temperature */

static const cl_uint empty_state = 0xffffffff;

/* OpenGL parameters */
static const int window_width = 1024;
static const int window_height = 1024;
static const char window_title[] = "NoseHoover";
static const double poll_timeout = 0.01;

/* OpenCL parameters */
static const cl_ulong device_index = 2;
static const cl_ulong work_group_size_1d = 256;
static const cl_ulong work_group_size_2d = 16;

}

/** ---------------------------------------------------------------------------
 * @brief NoseHoover spring-thermostat parameters.
 */
struct NoseHooverParam {
    cl_double mass;         /* mass */
    cl_double kappa;        /* force constant */
    cl_double tau;          /* thermostat constant */
    cl_double temperature;  /* thermostat temperature */
};

/**
 * @brief NoseHoover spring-thermostat data type.
 */
struct NoseHoover {
    cl_double pos;          /* spring position */
    cl_double mom;          /* spring momentum */
    cl_double eta;          /* thermostat state */
    cl_float4 color;        /* thermostat color attribute. */
};

#endif /* BASE_H_ */
