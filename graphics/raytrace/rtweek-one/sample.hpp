/*
 * sample.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef RAYTRACE_CPU_WEEK_1_SAMPLE_H_
#define RAYTRACE_CPU_WEEK_1_SAMPLE_H_

#include "atto/opencl/opencl.hpp"

/**
 * Sample
 * @brief Sample uniform random numbers and associated distributions.
 */
struct Sample {
    /** Sample number generator. */
    atto::math::rng::Kiss m_engine;             /* rng engine */
    atto::math::rng::uniform<double> m_urand;   /* rng sampler */

    /* Sample 1d and 2d uniform variates. */
    double rand1d(void);
    atto::math::vec2d rand2d(void);

    /** Sample a unit sphere using a uniform distribution. */
    static atto::math::vec3d UniformSphere(const atto::math::vec2d &u);
    static double UniformSpherePdf(void);

    /** Sample a unit hemisphere using a uniform distribution. */
    static atto::math::vec3d UniformHemisphere(const atto::math::vec2d &u);
    static double UniformHemispherePdf(void);

    /** Sample a unit hemisphere using a cosine distribution. */
    static atto::math::vec3d CosineHemisphere(const atto::math::vec2d &u);
    static double CosineHemispherePdf(const double cos_theta);

    /** Sample a cone using a uniform distribution. */
    static atto::math::vec3d UniformCone(
        const atto::math::vec2d &u,
        const double cos_theta_max);
    static double UniformConePdf(const double cos_theta_max);

    /** Sample unit disk using a uniform distribution. */
    static atto::math::vec2d UniformDisk(const atto::math::vec2d &u);
    static double UniformDiskPdf(void);

    /** Sample unit triangle using a uniform distribution. */
    static atto::math::vec2d UniformTriangle(const atto::math::vec2d &u);
    static double UniformTrianglePdf(void);

    /* Constructor/destructor. */
    Sample() : m_engine(true) {}
    ~Sample() = default;
};

/**
 * Sample::rand1d
 * @brief rand1d 1-dimensional uniform variate.
 */
core_inline
double Sample::rand1d(void)
{
    return m_urand(m_engine, 0.0, 1.0);
}

/**
 * Sample::rand2d
 * @brief Sample 2-dimensional uniform variate.
 */
core_inline
atto::math::vec2d Sample::rand2d(void)
{
    return atto::math::vec2d{
        m_urand(m_engine, 0.0, 1.0),
        m_urand(m_engine, 0.0, 1.0)};
}

#endif /* RAYTRACE_CPU_WEEK_1_SAMPLE_H_ */
