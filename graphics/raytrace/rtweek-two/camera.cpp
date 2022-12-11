/*
 * camera.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "camera.hpp"
using namespace atto;

/**
 * Camera::Camera
 * Create a camera with a specified eye position, looking into the ctr position
 * with a vertical up direction. The camera field of view is given by fov, with
 * aspect specified the aspect ratio.
 * The camera focus plane is located at a the distance given by depth. This
 * value is fixed, so both the width and height are scaled to maintain the
 * fov angle invariant in the scaling.
 */
Camera::Camera(
    const math::vec3d &eye,
    const math::vec3d &ctr,
    const math::vec3d &up,
    double fov,
    double aspect,
    double focus,
    double aperture)
{
    /* Camera orhonormal basis set */
    m_ortho.create_from_wv(eye - ctr, up);

    /* Camera eye position */
    m_eye = eye;

    /* Camera screen dimensions */
    double half_theta = 0.5 * fov * M_PI / 180.0;
    double viewport_height = 2.0 * std::tan(half_theta);
    double viewport_width = aspect * viewport_height;

    m_width = focus * viewport_width;
    m_height = focus * viewport_height;
    m_depth = focus;

    /* Camera lens radius */
    m_radius = 0.5 * aperture;
}

/**
 * Camera::generate_ray
 * Generate a new camera ray with direction towards a point on the screen
 * specified by the normalized coordinates (0,0) <= (u,v) <= (1,1).
 * The argument urand represents two uniform random numbers.
 */
Ray Camera::generate_ray(const math::vec2d &uv, const math::vec2d &urand) const
{
    /* Clamp the normalized coordinates to the range [0,1]. */
    double u = std::min(std::max(uv.x, 0.0), 1.0);
    double v = std::min(std::max(uv.y, 0.0), 1.0);

    /*
     * Sample a random point in the unit disk and project the corresponding
     * camera offset to world space.
     */
    math::vec2d disk = Sample::UniformDisk(urand);
    math::vec3d offset_local{
        m_radius * disk.x * std::cos(disk.y),
        m_radius * disk.x * std::sin(disk.y),
        0.0};
    math::vec3d offset = m_ortho.local_to_world(offset_local);

    /* Generate a point in camera space and project it to world space. */
    math::vec3d point_camera{
        (u - 0.5) * m_width,
        (v - 0.5) * m_height,
        -m_depth};
    math::vec3d point_world = m_ortho.local_to_world(point_camera);

    /* Return a ray with origin at the camera and with d. */
    return Ray(m_eye + offset, math::normalize(point_world - offset));
}
