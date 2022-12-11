/*
 * sample.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "sample.hpp"
using namespace atto;

/**
 * Sample::UniformSphere
 * @brief Sample a unit sphere using a uniform distribution.
 */
math::vec3d Sample::UniformSphere(const math::vec2d &u)
{
    double cos_theta = 1.0 - 2.0 * u.x;
    double sin_theta = std::sqrt(std::max(0.0, 1.0 - cos_theta*cos_theta));

    double phi = 2.0 * M_PI * u.y;
    double sin_phi = std::sin(phi);
    double cos_phi = std::cos(phi);

    return math::vec3d(sin_theta*cos_phi, sin_theta*sin_phi, cos_theta);
}

double Sample::UniformSpherePdf(void)
{
    constexpr double pdf = 1.0 / (4.0 * M_PI);
    return pdf;
}

/**
 * Sample::UniformHemisphere
 * @brief Sample a unit hemisphere using a uniform distribution.
 */
math::vec3d Sample::UniformHemisphere(const math::vec2d &u)
{
    double cos_theta = u.x;
    double sin_theta = std::sqrt(std::max(0.0, 1.0 - cos_theta*cos_theta));

    double phi = 2.0 * M_PI * u.y;
    double sin_phi = std::sin(phi);
    double cos_phi = std::cos(phi);

    return math::vec3d(sin_theta*cos_phi, sin_theta*sin_phi, cos_theta);
}

double Sample::UniformHemispherePdf(void)
{
    constexpr double pdf = 0.5 * M_1_PI;    /* 2.0/(4.0*M_PI) */
    return pdf;
}

/**
 * Sample::CosineHemisphere
 * @brief Sample a unit hemisphere using a cosine distribution.
 */
math::vec3d Sample::CosineHemisphere(const math::vec2d &u)
{
    double cos_theta = std::sqrt(u.x);
    double sin_theta = std::sqrt(1.0 - u.x);

    double phi = 2.0 * M_PI * u.y;
    double sin_phi = std::sin(phi);
    double cos_phi = std::cos(phi);

    return math::vec3d(sin_theta*cos_phi, sin_theta*sin_phi, cos_theta);
}

double Sample::CosineHemispherePdf(const double cos_theta)
{
    return cos_theta > 0.0 ? cos_theta * M_1_PI : 0.0;
}

/**
 * Sample::UniformCone
 * @brief Sample a cone using a uniform distribution.
 */
math::vec3d Sample::UniformCone(
    const math::vec2d &u,
    const double cos_theta_max)
{
    double cos_theta = 1.0 - u.x * (1.0 - cos_theta_max);
    double sin_theta = std::sqrt(std::max(0.0, 1.0 - cos_theta*cos_theta));

    double phi = 2.0 * M_PI * u.y;
    double sin_phi = std::sin(phi);
    double cos_phi = std::cos(phi);

    return math::vec3d(sin_theta*cos_phi, sin_theta*sin_phi, cos_theta);
}

double Sample::UniformConePdf(const double cos_theta_max)
{
    constexpr double pdf = 1.0 / (2.0 * M_PI);
    return pdf / (1.0 - cos_theta_max);
}

/**
 * Sample::UniformDisk
 * @brief Sample unit disk using a uniform distribution:
 *  r = sqrt(u),
 *  theta = 2*pi*u
 *  pdf = 1 / pi;
 *
 */
math::vec2d Sample::UniformDisk(const math::vec2d &u)
{
    return math::vec2d{std::sqrt(u.x), 2.0 * M_PI * u.y};
}

double Sample::UniformDiskPdf(void)
{
    constexpr double pdf = M_1_PI;
    return pdf;
}

/**
 * Sample::UniformTriangle
 * @brief Sample unit triangle using a uniform distribution.
 */
math::vec2d Sample::UniformTriangle(const math::vec2d &u)
{
    double r = std::sqrt(u.x);
    return math::vec2d{1.0 - r, r*u.y};
}

double Sample::UniformTrianglePdf(void)
{
    constexpr double pdf = 2.0;
    return pdf;
}
