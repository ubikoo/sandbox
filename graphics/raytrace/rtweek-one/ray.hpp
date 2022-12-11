/*
 * ray.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef RAYTRACE_CPU_WEEK_1_RAY_H_
#define RAYTRACE_CPU_WEEK_1_RAY_H_

#include "base.hpp"

/**
 * Ray
 * @brief Represent a ray as p(t) = o + t*d;
 */
struct Ray {
    atto::math::vec3d o;
    atto::math::vec3d d;

    /* Return the point at line parameter t. */
    atto::math::vec3d at(const double t) const { return (o + d*t); }

    Ray() = default;
    Ray(const atto::math::vec3d &o, const atto::math::vec3d &d) : o(o), d(d) {}
    ~Ray() = default;
};

#endif /* RAYTRACE_CPU_WEEK_1_RAY_H_ */
