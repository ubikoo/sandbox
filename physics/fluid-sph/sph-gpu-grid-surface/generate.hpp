/*
 * generate.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef SPH_GENERATE_H_
#define SPH_GENERATE_H_

#include "atto/opencl/opencl.hpp"
#include "base.hpp"

namespace generate {

/** Create a set of points uniformly distributed inside a box. */
std::vector<cl_float4> points_random(
    const cl_ulong n_points,
    const cl_float xlo,
    const cl_float ylo,
    const cl_float zlo,
    const cl_float xhi,
    const cl_float yhi,
    const cl_float zhi);

/** Create a collection of points inside a simple cubic lattice. */
std::vector<cl_float4> points_cubic(
    const cl_ulong n_points,
    const cl_float xlo,
    const cl_float ylo,
    const cl_float zlo,
    const cl_float xhi,
    const cl_float yhi,
    const cl_float zhi);

/** Create a collection of points inside a face centred cubic lattice. */
std::vector<cl_float4> points_fcc(
    const cl_ulong n_points,
    const cl_float xlo,
    const cl_float ylo,
    const cl_float zlo,
    const cl_float xhi,
    const cl_float yhi,
    const cl_float zhi);

} /* generate */

#endif /* SPH_GENERATE_H_ */
