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

#include "material.hpp"
using namespace atto;

/**
 * Material::CreateDiffuse
 * Diffuse material factory function.
 */
Material Material::CreateDiffuse(const Color &rho)
{
    Material material{
        .type = Material::Diffuse,
        .rho  = rho,
        .ior  = 0.0,
        .Le   = Color::Black,
    };
    return material;
}

/**
 * Material::CreateConductor
 * Conductor material factory function.
 */
Material Material::CreateConductor(const Color &rho)
{
    Material material{
        .type = Material::Conductor,
        .rho  = rho,
        .ior  = 0.0,
        .Le   = Color::Black,
    };
    return material;
}

/**
 * Material::CreateDielectric
 * Dielectric material factory function.
 */
Material Material::CreateDielectric(const double ior)
{
    Material material{
        .type = Material::Dielectric,
        .rho  = Color::Black,
        .ior  = ior,
        .Le   = Color::Black,
    };
    return material;
}
