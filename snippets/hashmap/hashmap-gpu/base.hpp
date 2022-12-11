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
/* OpenGL/OpenCL parameters */
static const cl_ulong device_index = 2;
static const cl_ulong work_group_size = 256;

/* Model data */
static const uint32_t load_factor = 4;
static const uint32_t n_steps = 10;
static const uint32_t n_cells = (1 << 4);
static const uint32_t n_points = (1 << 8);
static const atto::math::vec3f domain_lo{-1.0f, -1.0f, -1.0f};
static const atto::math::vec3f domain_hi{1.0f,  1.0f,  1.0f};
} /* Params */

#endif /* BASE_H_ */
