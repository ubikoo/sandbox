/*
 * primitive.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef RAYTRACE_CPU_WEEK_2_PRIMITIVE_H_
#define RAYTRACE_CPU_WEEK_2_PRIMITIVE_H_

#include "base.hpp"
#include "ray.hpp"
#include "isect.hpp"
#include "material.hpp"

/**
 * Primitive
 * @brief Primitive represents a geometric object with a specified material.
 * For simplicity, only sphere geometry is represented here.
 *
 * @todo Disks, planes, meshes, etc.
 */
struct Primitive {
    /** Primitive geometry. */
    atto::math::vec3d centre;
    double radius;

    /** Primitive material. */
    Material material;

    /** Primitive factory function with sphere geometry. */
    static Primitive Create(
        const atto::math::vec3d &centre,
        const double radius,
        const Material &material);

    /** Compute primitive-ray intersection. */
    static bool Intersect(
        const Primitive &primitive,
        const Ray &ray,
        const double t_min,
        const double t_max,
        double &t,
        atto::math::vec3d &n);

    /** Compute primitive-ray intersection and store geometric properties. */
    static bool Intersect(
        const Primitive &primitive,
        const Ray &ray,
        const double t_min,
        const double t_max,
        Isect &isect);

    /** Compute the closest primitive-ray intersection. */
    static bool Intersect(
        const std::vector<Primitive> &primitives,
        const Ray &ray,
        const double t_min,
        const double t_max,
        Isect &isect);

    Primitive() = default;
    ~Primitive() = default;
};

#endif /* RAYTRACE_CPU_WEEK_2_PRIMITIVE_H_ */
