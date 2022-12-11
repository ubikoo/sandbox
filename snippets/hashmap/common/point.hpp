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

/**
 * create_box
 * Create a collection of n_points with coordinates (x,y,z) and
 * uniformly distributed inside the box:
 *  xlo < x < xhi
 *  xlo < x < yhi
 *  xlo < x < xhi
 */
std::vector<atto::math::vec3f> create_box(
    const size_t n_points,
    const float xlo,
    const float ylo,
    const float zlo,
    const float xhi,
    const float yhi,
    const float zhi);

/**
 * create_sphere
 * Create a collection of n_points with coordinates (x,y,z) uniformly
 * distributed inside the a sphere:
 *  rx = x - xctr
 *  ry = y - yctr
 *  rz = z - zctr
 *  sqrt(rx*rx + ry*ry + rz*rz) < radius
 */
std::vector<atto::math::vec3f> create_sphere(
    const size_t n_points,
    const float radius,
    const float xctr,
    const float yctr,
    const float zctr);

/**
 * create_lattice
 * Create a collection of n_points inside an lattice with simple cubic
 * cell with specified dimensions
 */
std::vector<atto::math::vec3f> create_lattice(
    const size_t n_points,
    const float xlo,
    const float ylo,
    const float zlo,
    const float xhi,
    const float yhi,
    const float zhi);

/**
 * create_lattice_fcc
 * Create a collection of n_points inside an lattice with face
 * centred cubic cells with specified dimensions.
 */
std::vector<atto::math::vec3f> create_lattice_fcc(
    const size_t n_points,
    const float xlo,
    const float ylo,
    const float zlo,
    const float xhi,
    const float yhi,
    const float zhi);

#endif /* POINT_H_ */
