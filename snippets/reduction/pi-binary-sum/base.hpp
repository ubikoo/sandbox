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
static const cl_ulong num_steps = 1024*1024;
static const cl_ulong num_iters = 1024;
static const cl_ulong device_index = 2;
static const cl_ulong work_group_size = 256;
static const cl_ulong num_work_items = atto::cl::NDRange::Roundup(
    num_steps / num_iters,  work_group_size);
static const cl_ulong num_work_groups = num_work_items / work_group_size;
} /* Params */

#endif /* BASE_H_ */
