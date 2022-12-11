/*
 * camera.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef RAYTRACE_CPU_WEEK_1_CAMERA_H_
#define RAYTRACE_CPU_WEEK_1_CAMERA_H_

#include "base.hpp"
#include "ray.hpp"

/**
 * Camera
 */
struct Camera {
    /* Camera member data. */
    atto::math::Orthod m_ortho;     /* camera orthonormal basis set */
    atto::math::vec3d m_eye;        /* camera eye position */
    double m_width;                 /* -1 <= width <= 1 */
    double m_height;                /* -1 <= height <= 1 */
    double m_depth;                 /* 0 <= depth */

    /* Generate a new camera ray. */
    Ray generate_ray(const atto::math::vec2d &uv) const;

    /* Camera ctor/dtor. */
    Camera(
        const atto::math::vec3d &eye,
        const atto::math::vec3d &ctr,
        const atto::math::vec3d &up,
        double fov,
        double aspect);
    ~Camera() = default;
};

#endif /* RAYTRACE_CPU_WEEK_1_CAMERA_H_ */
