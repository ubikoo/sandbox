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
static const uint64_t n_threads = 16;   /* Threadpool number threads */

static const uint64_t n_steps = 256;     /* Integration time step size */
static const double t_step = 0.01;      /* Integration time step size */
static const double max_err = 1.0E-5;   /* Midpoint max error */
static const uint64_t max_iter = 3;     /* Midpoing max iterations */

static const double mass = 1.0;          /* Nose-Hoover mass */
static const double kappa = 1.0;         /* Nose-Hoover force constant */
static const double tau = 10.0;          /* Nose-Hoover thermostat constant */
static const double temperature = 1.0;   /* Nose-Hoover thermostat temperature */

static const uint64_t n_points = 1024;         /* thermostat init points */
static const uint64_t n_thermostats = n_points * n_points;
static const double q_init_width = 4.0;     /* thermostat init position range */
static const double p_init_height = 4.0;    /* thermostat init momentum range */

static const double canvas_width = 12.0;     /* canvas roi width */
static const double canvas_height = 12.0;    /* convas roi height */

/* OpenGL parameters */
static const int window_width = 1024;
static const int window_height = 1024;
static const char window_title[] = "NoseHoover";
static const double poll_timeout = 0.01;

/* OpenCL parameters */
//static const cl_ulong device_index = 2;
//static const cl_ulong work_group_size = 256;
//static const cl_ulong num_work_items = atto::cl::NDRange::Roundup(n_thermostats, work_group_size);
//static const cl_ulong num_work_groups = num_work_items / work_group_size;

}

#endif /* BASE_H_ */
