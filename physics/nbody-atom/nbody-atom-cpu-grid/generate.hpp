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
std::vector<atto::math::vec3d> points_random(
    const size_t n_points,
    const double xlo,
    const double ylo,
    const double zlo,
    const double xhi,
    const double yhi,
    const double zhi);

/** Create a collection of points inside a simple cubic lattice. */
std::vector<atto::math::vec3d> points_cubic(
    const size_t n_points,
    const double xlo,
    const double ylo,
    const double zlo,
    const double xhi,
    const double yhi,
    const double zhi);

/** Create a collection of points inside a face centred cubic lattice. */
std::vector<atto::math::vec3d> points_fcc(
    const size_t n_points,
    const double xlo,
    const double ylo,
    const double zlo,
    const double xhi,
    const double yhi,
    const double zhi);

} /* generate */

#endif /* MD_GENERATE_H_ */
