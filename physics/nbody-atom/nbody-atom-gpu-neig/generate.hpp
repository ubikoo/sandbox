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

#ifndef MD_GENERATE_H_
#define MD_GENERATE_H_

#include "atto/opencl/opencl.hpp"
#include "base.hpp"

namespace generate {

/** Create a set of points uniformly distributed inside a box. */
std::vector<cl_double4> points_random(
    const cl_ulong n_points,
    const cl_double xlo,
    const cl_double ylo,
    const cl_double zlo,
    const cl_double xhi,
    const cl_double yhi,
    const cl_double zhi);

/** Create a collection of points inside a simple cubic lattice. */
std::vector<cl_double4> points_cubic(
    const cl_ulong n_points,
    const cl_double xlo,
    const cl_double ylo,
    const cl_double zlo,
    const cl_double xhi,
    const cl_double yhi,
    const cl_double zhi);

/** Create a collection of points inside a face centred cubic lattice. */
std::vector<cl_double4> points_fcc(
    const cl_ulong n_points,
    const cl_double xlo,
    const cl_double ylo,
    const cl_double zlo,
    const cl_double xhi,
    const cl_double yhi,
    const cl_double zhi);

} /* generate */

#endif /* MD_GENERATE_H_ */
