/*
 * isect.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef RAYTRACE_CPU_WEEK_2_ISECT_H_
#define RAYTRACE_CPU_WEEK_2_ISECT_H_

#include "base.hpp"
#include "ray.hpp"
#include "material.hpp"

/**
 * Isect
 * @brief Isect handles the intersection between a ray and a primitive and
 * maintains the properties of at the intersection point - line parameter,
 * position, normal, etc.
 *
 *  bool hit(primitive, ray, t_min, t_max)
 *  bool scatter(wo, wi, bsdf, pdf)
 *
 * scatter() samples a scattering ray incident at the intersection point and
 * corresponding scattering properties - bsdf, pdf.
 */
struct Isect {
    atto::math::vec3d p;
    atto::math::vec3d n;
    atto::math::vec3d wo;
    Material material;
    double t;

    /* Return the unit direction vector to the specified destination point. */
    atto::math::vec3d dirto(const atto::math::vec3d &dst) const;

    /* Spawn a ray from the intersection point in the specified direction. */
    Ray spawn(const atto::math::vec3d &dir) const;

    Isect() = default;
    ~Isect() = default;
};

/**
 * Isect::dirto
 * @brief Return the unit direction vector to the specified target point.
 */
core_inline
atto::math::vec3d Isect::dirto(const atto::math::vec3d &target) const
{
    return atto::math::normalize(target - p);
}

/**
 * Isect::spawn
 * @brief Spawn a ray from the intersection point in the specified direction.
 */
core_inline
Ray Isect::spawn(const atto::math::vec3d &dir) const
{
    return Ray(p, atto::math::normalize(dir));
}

#endif /* RAYTRACE_CPU_WEEK_2_ISECT_H_ */
