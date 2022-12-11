/*
 * primitive.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "primitive.hpp"
using namespace atto;

/**
 * Primitive::Create
 * @brief Primitive factory function with sphere geometry.
 */
Primitive Primitive::Create(
    const atto::math::vec3d &centre,
    const double radius,
    const Material &material)
{
    Primitive primitive{
        .centre = centre,
        .radius = radius,
        .material = material
    };
    return primitive;
}

/**
 * Primitive::Intersect
 * @brief Compute primitive-ray intersection.
 */
bool Primitive::Intersect(
    const Primitive &primitive,
    const Ray &ray,
    const double t_min,
    const double t_max,
    double &t,
    math::vec3d &n)
{
    /*
     * Solve the ray-sphere intersection problem:
     *  (p(t) - centre) \dot (p(t) - centre) = radius^2, where p(t) = o + t*d.
     */
    math::vec3d oc = ray.o - primitive.centre;
    double a = math::dot(ray.d, ray.d);
    double b = math::dot(ray.d, oc);
    double c = math::dot(oc, oc) - primitive.radius * primitive.radius;

    double discriminant = b*b - a*c;
    if (discriminant < 0.0) {
        return false;
    }
    discriminant = std::sqrt(discriminant);

    /* Compute the line parameter and normal at the intersection point. */
    t = -(b + discriminant) / a;
    if (t < t_min) {
        t = -(b - discriminant) / a;
    }
    if (t < t_min || t > t_max) {
        return false;   /* line parameter outside intersection range */
    }
    n = math::normalize(ray.at(t) - primitive.centre);
    return true;
}

/**
 * Primitive::Intersect
 * @brief Compute primitive-ray intersection and store geometric properties.
 */
bool Primitive::Intersect(
    const Primitive &primitive,
    const Ray &ray,
    const double t_min,
    const double t_max,
    Isect &isect)
{
    double t;
    math::vec3d n;
    if (Primitive::Intersect(primitive, ray, t_min, t_max, t, n)) {
        isect.p = ray.at(t);
        isect.n = n;
        isect.wo = -ray.d;
        isect.t = t;
        isect.material = primitive.material;
        return true;
    }
    return false;
}

/**
 * Primitive::Intersect
 * @brief Compute the closest primitive-ray intersection.
 */
bool Primitive::Intersect(
    const std::vector<Primitive> &primitives,
    const Ray &ray,
    const double t_min,
    const double t_max,
    Isect &isect)
{
    bool is_a_hit = false;
    double t_hit = t_max;
    for (const auto &primitive : primitives) {
        if (Primitive::Intersect(primitive, ray, t_min, t_hit, isect)) {
            is_a_hit = true;
            t_hit = isect.t;
        }
    }
    return is_a_hit;
}
