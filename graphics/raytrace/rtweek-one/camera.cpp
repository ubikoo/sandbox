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
 * Create a camera with a specified eye position, looking into the ctr
 * position with a vertical up direction, with a field of view fov and a
 * specified aspect ratio.
 */
Camera::Camera(
    const math::vec3d &eye,
    const math::vec3d &ctr,
    const math::vec3d &up,
    double fov,
    double aspect)
{
    /* Camera orhonormal basis set */
    m_ortho.create_from_wv(eye - ctr, up);

    /* Camera eye position */
    m_eye = eye;

    /* Camera screen dimensions */
    double half_width = 1.0;
    double half_height = 1.0;

    if (aspect < 1.0) {
        half_width *= aspect;
    } else {
        half_height /= aspect;
    }

    m_width = 2.0 * half_width;
    m_height = 2.0 * half_height;
    double half_theta = 0.5 * fov * M_PI / 180.0;
    m_depth = half_height / std::tan(half_theta);
}

/**
 * Camera::generate_ray
 * Generate a new camera ray with direction towards a point on the screen
 * specified by the normalized coordinates (0,0) <= (u,v) <= (1,1).
 */
Ray Camera::generate_ray(const math::vec2d &uv) const
{
    /* Clamp the normalized coordinates to the range [0,1] */
    double u = std::min(std::max(uv.x, 0.0), 1.0);
    double v = std::min(std::max(uv.y, 0.0), 1.0);

    /* Generate a point in camera space and project it to world space */
    math::vec3d point_camera{
        (u - 0.5) * m_width,
        (v - 0.5) * m_height,
        -m_depth};
    math::vec3d point_world = m_ortho.local_to_world(point_camera);

    /* Return a ray with origin at the camera and with d. */
    return Ray(m_eye, math::normalize(point_world));
}
