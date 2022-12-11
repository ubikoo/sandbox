/*
 * interaction.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef RAYTRACE_CPU_WEEK_1_INTERACTION_H_
#define RAYTRACE_CPU_WEEK_1_INTERACTION_H_

#include "base.hpp"
#include "color.hpp"
#include "ray.hpp"
#include "isect.hpp"
#include "material.hpp"
#include "primitive.hpp"
#include "sample.hpp"

namespace Interaction {

/** ---------------------------------------------------------------------------
 * AbsDot
 * @brief Return the absolute dot product |w.v|.
 */
core_inline
double AbsDot(const atto::math::vec3d &v, const atto::math::vec3d &w)
{
    return std::abs(atto::math::dot(v, w));
}

/**
 * SameHemisphere
 * @brief Are vectors wo and wi on the same hemisphere specified by normal n?
 */
core_inline
bool SameHemisphere(
    const atto::math::vec3d &n,
    const atto::math::vec3d &wo,
    atto::math::vec3d &wi)
{
    double cos_theta_o = atto::math::dot(n, wo);
    double cos_theta_i = atto::math::dot(n, wi);
    return (cos_theta_o * cos_theta_i > 0.0);
}

/**
 * FaceForward
 * @brief Orient w to lie on the same hemisphere specified by n.
 */
core_inline
atto::math::vec3d FaceForward(
    const atto::math::vec3d &n,
    const atto::math::vec3d &w)
{
    if (atto::math::dot(n, w) < 0.0) {
        return -w;
    }
    return w;
}

/** ---------------------------------------------------------------------------
 * @brief Given the specified outgoing vector wo at a point on a surface with
 * normal n, compute the incident vector wi that is reflected to wo.
 */
void Reflect(
    const atto::math::vec3d &n,
    const atto::math::vec3d &wo,
    atto::math::vec3d &wi);

/**
 * @brief Given the specified outgoing vector wo at a point on a surface with
 * normal n, compute the incident vector wi that is refracted to wo.
 */
bool Refract(
    const double eta,
    const atto::math::vec3d &n,
    const atto::math::vec3d &wo,
    atto::math::vec3d &wi);

/** Return the reflectance of a conductor using Schlick approximation. */
Color SchlickConductor(const Color &R0, const double cos_theta_i);

/** Return the reflectance of a dielectric using Schlick approximation. */
double SchlickDielectric(const double eta, const double cos_theta_i);

/** Return the bsdf of a dielectric material. */
Color BsdfDielectricRefract(const double eta, const double cos_theta_i);

/** ---------------------------------------------------------------------------
 * @brief Return the intersection indicident direction and scattering functions.
 */
bool Scatter(
    const Isect &isect,
    const atto::math::vec2d &u,
    const atto::math::vec3d &wo,
    atto::math::vec3d &wi,
    Color &bsdf,
    double &pdf);

} /* Interaction */

#endif /* RAYTRACE_CPU_WEEK_1_INTERACTION_H_ */
