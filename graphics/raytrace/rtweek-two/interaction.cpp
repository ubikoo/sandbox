/*
 * interaction.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include <random>
#include "interaction.hpp"
using namespace atto;

namespace Interaction {

/** ---------------------------------------------------------------------------
 * Reflect
 * @brief Given the specified outgoing vector wo at a point on a surface with
 * normal n, compute the incident vector wi that is reflected to wo.
 */
void Reflect(const math::vec3d &n, const math::vec3d &wo, math::vec3d &wi)
{
    wi = -wo + (2.0 * math::dot(n, wo)) * n;
}

/**
 * Refract
 * @brief Given the specified outgoing vector wo at a point on a surface with
 * normal n, compute the incident vector wi that is refracted to wo.
 * The eta parameter represents the ratio of the indices of refraction of the
 * outgoing to incident directions, eta = no / ni, where no is the ior of the
 * outgoing vector medium and ni is the ior of the incident vector medium.
 */
bool Refract(
    const double eta,
    const math::vec3d &n,
    const math::vec3d &wo,
    math::vec3d &wi)
{
    /*
     * Compute the trignometric components of wi using Snell's law and check
     * for the total internal reflection condition for transmission.
     */
    double cos_theta_o = math::dot(n, wo);
    double cos2_theta_o = cos_theta_o * cos_theta_o;
    double sin2_theta_o = std::max(0.0, 1.0 - cos2_theta_o);

    double sin2_theta_i = eta * eta * sin2_theta_o;
    if (sin2_theta_i > 1.0) {
        return false;
    }

    double cos2_theta_i = std::max(0.0, 1.0 - sin2_theta_i);
    double sign_theta_i = cos_theta_o < 0.0 ? 1.0 : -1.0;
    double cos_theta_i = sign_theta_i * std::sqrt(cos2_theta_i);

    /* Compute wi from wo and the normal n */
    wi = -eta * wo + (eta * cos_theta_o + cos_theta_i) * n;
    return true;
}

/** ---------------------------------------------------------------------------
 * SchlickConductor
 * @brief Return the reflectance of a conductor using Schlick approximation.
 * Argument rho is the reflectance when the incidient direction is parallel
 * to the surface normal (cos_theta = 0):
 *      R = R0 + (1 - R0) * (1 - |cos(theta)|)^5
 */
Color SchlickConductor(const Color &R0, const double cos_theta_i)
{
    double c = std::min(std::max(1.0 - cos_theta_i, 0.0), 1.0);
    return R0 + (Color::White - R0) * (c * c * c * c * c);
}


/**
 * SchlickDielectric
 * @brief Return the reflectance of a dielectric using Schlick approximation:
 *      R = R0 + (1 - R0) * (1 - |cos(theta)|)^5
 *      R0 = ((1 - eta) / (1 + eta))^2
 *
 * Schlick’s approximation of the Fresnel equation applies when the incident
 * light travels form a less dense medium to a more dense one:
 *      ni < no => eta = no / ni > 1
 *
 * If light travels in the opposite direction, leaving a more dense medion to
 * a lesser dense one:
 *      ni > no => eta = no / ni < 1
 * then we need to use the angle of the outgoing direction with the normal and
 * handle the case of total internal reflection.
 */
double SchlickDielectric(const double eta, const double cos_theta_i)
{
    double c = std::min(std::max(1.0 - cos_theta_i, 0.0), 1.0);

    if (eta < 1.0) {
        double cos2_theta_i = cos_theta_i * cos_theta_i;
        double sin2_theta_i = std::max(0.0, 1.0 - cos2_theta_i);
        double sin2_theta_o = sin2_theta_i / (eta * eta);

        if (sin2_theta_o > 1.0) {
            return 1.0;    /* Total Internal Reflection */
        }

        double cos2_theta_o = std::max(0.0, 1.0 -  sin2_theta_o);
        double cos_theta_o = std::sqrt(cos2_theta_o);
        c = std::min(std::max(1.0 - cos_theta_o, 0.0), 1.0);
    }

    double R0 = (1.0 - eta) / (1.0 + eta);
    R0 *= R0;
    return R0 + (1.0 - R0) * (c * c * c * c * c);
}

/** ---------------------------------------------------------------------------
 * Scatter
 * @brief Return the intersection indicident direction and scattering functions.
 */
bool Scatter(
    const Isect &isect,
    const math::vec2d &u,
    const math::vec3d &wo,
    math::vec3d &wi,
    Color &bsdf,
    double &pdf)
{
    /*
     * Diffuse material. Sample a random direction on the same hemisphere as
     * the outgoing direction using a cosine distribution to favour variates
     * close to the normal direction.
     * The bsdf of a diffuse material is constant for every pair wi and wo,
     * and equal to (reflectance / pi).
     */
    if (isect.material.type == Material::Diffuse) {
        math::Orthod uvw = {};
        uvw.create_from_w(isect.n);
        wi = uvw.local_to_world(Sample::CosineHemisphere(u));
        if (!SameHemisphere(isect.n, wo, wi)) {
            wi = -wi;
        }
        double cos_theta_i = AbsDot(isect.n, wi);

        bsdf = isect.material.rho * M_1_PI;
        pdf = Sample::CosineHemispherePdf(cos_theta_i);
        return true;
    }

    /*
     * Conductor material. Compute the incident direction that is reflected
     * into the outgoing direction. The associated pdf is a directional Dirac
     * delta function, delta(wi - reflect(wo)).
     * The bsdf of a conductor material contains the same directional Dirac
     * delta function, delta(wi - reflect(wo)), thereby cancelling each other
     * in the estimator for the outgoing radiance.
     * The pdf is thus set to one, and the bsdf is set to be:
     *  reflectance / |cos(theta_i)|
     */
    if (isect.material.type == Material::Conductor) {
        Reflect(isect.n, wo, wi);
        double cos_theta_i = AbsDot(isect.n, wi);
        Color R = SchlickConductor(isect.material.rho, cos_theta_i);

        bsdf = R / cos_theta_i;
        pdf = 1.0;
        return true;
    }

    /*
     * Dielectric material
     * The eta parameter is the ratio of the indices of refraction of the
     * outgoing to incident directions, eta = n_o / n_i, where n_o is the
     * ior of the outgoing medium and n_i is the ior of the incident medium.
     */
    if (isect.material.type == Material::Dielectric) {
        double cos_theta_o = math::dot(isect.n, wo);
        bool entering = cos_theta_o < 0.0;

        double eta_i = entering ? 1.0 : isect.material.ior;
        double eta_o = entering ? isect.material.ior : 1.0;
        double eta = eta_o / eta_i;

        /*
         * Compute the Fresnel reflectance using Schlick approximation.
         * Here, the incident direction is on the same hemisphere as the
         * outgoing direction, with cos_theta_i = cos_theta_o, and with
         * eta inverted because the ior of the incident medium is on the
         * same side as the outgoing medium.
         */
        double F = SchlickDielectric(1.0 / eta, std::abs(cos_theta_o));

        if (u.x < F) {
            Reflect(isect.n, wo, wi);
            double cos_theta_i = AbsDot(isect.n, wi);
            double R = F / cos_theta_i;

            bsdf = Color::White * R;
            pdf = F;
        } else {
            if (!Refract(eta, isect.n, wo, wi)) {
                return false;
            }
            double cos_theta_i = AbsDot(isect.n, wi);
            double T = (1.0 - F) * eta * eta / cos_theta_i;

            bsdf = Color::White * T;
            pdf = 1.0 - F;
        }
        return true;
    }

    /*
     * Emissive material. Sample a random direction on the same hemisphere as
     * the outgoing direction using a cosine distribution to favour variates
     * close to the normal direction.
     * The bsdf of a diffuse material is constant for every pair wi and wo,
     * and equal to (reflectance / pi).
     */
    if (isect.material.type == Material::Emissive) {
        math::Orthod uvw = {};
        uvw.create_from_w(isect.n);
        wi = uvw.local_to_world(Sample::CosineHemisphere(u));
        if (!SameHemisphere(isect.n, wo, wi)) {
            wi = -wi;
        }
        double cos_theta_i = AbsDot(isect.n, wi);

        bsdf = isect.material.rho * M_1_PI;
        pdf = Sample::CosineHemispherePdf(cos_theta_i);
        return true;
    }

    return false;
}

} /* Interaction */
