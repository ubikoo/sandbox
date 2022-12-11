/*
 * material.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef RAYTRACE_CPU_WEEK_2_MATERIAL_H_
#define RAYTRACE_CPU_WEEK_2_MATERIAL_H_

#include "base.hpp"
#include "color.hpp"

/**
 * Material
 */
struct Material {
    /** Material properties. */
    enum : uint32_t {
        Diffuse = 0,
        Conductor,
        Dielectric,
        Emissive
    };
    uint32_t type;          /* material type */
    Color rho;              /* reflectance */
    double ior;             /* index of refraction */
    Color Le;               /* emitted radiance */

    /** Diffuse material factory function. */
    static Material CreateDiffuse(const Color &rho);

    /** Conductor material factory function. */
    static Material CreateConductor(const Color &rho);

    /** Dielectric material factory function. */
    static Material CreateDielectric(const double ior);

    /** Dielectric material factory function. */
    static Material CreateEmissive(const Color &rho, const Color &Le);

    /* Constructor/destructor. */
    Material() = default;
    ~Material() = default;
};

#endif /* RAYTRACE_CPU_WEEK_2_MATERIAL_H_ */
