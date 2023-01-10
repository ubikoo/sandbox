/*
 * point.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef POINT_H_
#define POINT_H_

#include <vector>
#include "ito/math.hpp"

namespace common {

/**
 * @brief Create a collection of points with coordinates (x,y,z) uniformly
 * distributed inside a box:
 *  xlo < x < xhi
 *  ylo < y < yhi
 *  zlo < z < zhi
 */
std::vector<ito::math::vec3f> CreateBox(
    const size_t n_points,
    const float xlo,
    const float ylo,
    const float zlo,
    const float xhi,
    const float yhi,
    const float zhi);

/**
 * @brief Create a collection of points with coordinates (x,y,z) uniformly
 * distributed inside a sphere:
 *  rx = x - xctr
 *  ry = y - yctr
 *  rz = z - zctr
 *  sqrt(rx * rx + ry * ry + rz * rz) < radius
 */
std::vector<ito::math::vec3f> CreateSphere(
    const size_t n_points,
    const float radius,
    const float xctr,
    const float yctr,
    const float zctr);

/**
 * @brief Create a collection of points inside a simple cubic lattice.
 */
std::vector<ito::math::vec3f> CreateSccLattice(
    const size_t n_points,
    const float xlo,
    const float ylo,
    const float zlo,
    const float xhi,
    const float yhi,
    const float zhi);

/**
 * @brief Create a collection of points inside a face centred cubic lattice.
 */
std::vector<ito::math::vec3f> CreateFccLattice(
    const size_t n_points,
    const float xlo,
    const float ylo,
    const float zlo,
    const float xhi,
    const float yhi,
    const float zhi);

} /* common */

#endif /* POINT_H_ */
