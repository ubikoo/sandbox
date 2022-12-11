/*
 * compute.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "atto/opencl/opencl.hpp"
#include "compute.hpp"
using namespace atto;

namespace compute {

/** ---------------------------------------------------------------------------
 * com_mass
 * @brief Compute the total mass of the fluid.
 */
cl_double com_mass(std::vector<Atom> &atoms)
{
    cl_double mass = 0.0;
    for (auto &atom : atoms) {
        mass += atom.mass;
    }
    return mass;
}

/** ---------------------------------------------------------------------------
 * com_pos
 * @brief Compute CoM position of the fluid.
 */
cl_double4 com_pos(std::vector<Atom> &atoms)
{
    cl_double mass = 0.0;
    cl_double4 pos = cl_double4{};
    for (auto &atom : atoms) {
        mass += atom.mass;
        pos += atom.pos * atom.mass;
    }
    return pos / mass;
}

/** ---------------------------------------------------------------------------
 * com_upos
 * @brief Compute CoM unfolded position of the fluid.
 */
cl_double4 com_upos(std::vector<Atom> &atoms)
{
    cl_double mass = 0.0;
    cl_double4 upos = cl_double4{};
    for (auto &atom : atoms) {
        mass += atom.mass;
        upos += atom.upos * atom.mass;
    }
    return upos / mass;
}

/** ---------------------------------------------------------------------------
 * com_vel
 * @brief Compute CoM velocity of the fluid.
 */
cl_double4 com_vel(std::vector<Atom> &atoms)
{
    cl_double mass = 0.0;
    cl_double4 mom = cl_double4{};
    for (auto &atom : atoms) {
        mass += atom.mass;
        mom += atom.mom;
    }
    return mom /= mass;
}

/** ---------------------------------------------------------------------------
 * com_mom
 * @brief Compute total momentum of the fluid.
 */
cl_double4 com_mom(std::vector<Atom> &atoms)
{
    cl_double4 mom = cl_double4{};
    for (auto &atom : atoms) {
        mom += atom.mom;
    }
    return mom;
}

/** ---------------------------------------------------------------------------
 * com_force
 * @brief Compute total force on the fluid.
 */
cl_double4 com_force(std::vector<Atom> &atoms)
{
    cl_double4 force = cl_double4{};
    for (auto &atom : atoms) {
        force += atom.force;
    }
    return force;
}

/** ---------------------------------------------------------------------------
 * density
 * @brief Compute the density of the fluid.
 */
cl_double density(std::vector<Atom> &atoms, const Domain &domain)
{
    cl_double volume = domain.length.s[0] *
                       domain.length.s[1] *
                       domain.length.s[2];
    cl_double density = com_mass(atoms) / volume;
    return density;
}

/** ---------------------------------------------------------------------------
 * energy_kin
 * @brief Compute fluid kinetic energy.
 */
cl_double energy_kin(std::vector<Atom> &atoms)
{
    cl_double energy = 0.0;
    for (auto &atom : atoms) {
        energy += cl::dot(atom.mom, atom.mom) * atom.rmass;
    }
    return 0.5 * energy;
}

/** ---------------------------------------------------------------------------
 * energy_pot
 * @brief Compute fluid potential energy.
 */
cl_double energy_pot(std::vector<Atom> &atoms)
{
    cl_double energy = 0.0;
    for (auto &atom : atoms) {
        energy += atom.energy;
    }
    return energy;
}

/** ---------------------------------------------------------------------------
 * temperature_kin
 * @brief Compute fluid kinetic temperature.
 */
cl_double temperature_kin(
    std::vector<Atom> &atoms,
    cl_double &grad_sq,
    cl_double &laplace)
{
    /* Compute kinetic temperature of the fluid. */
    grad_sq = 0.0;
    laplace = 0.0;
    for (auto &atom : atoms) {
        grad_sq += cl::dot(atom.mom, atom.mom) * atom.rmass;
        laplace += 3.0;
    }

    /*
     * If fluid size is more than one atom, remove the CoM momentum
     * contribution to the total kinetic energy and account for the
     * correct number of degrees of freedom.
     */
    if (atoms.size() > 1) {
        cl_double mass = compute::com_mass(atoms);
        cl_double4 mom = compute::com_mom(atoms);

        cl_double com_grad_sq = cl::dot(mom, mom) / mass;
        cl_double com_laplace = 3.0;

        grad_sq -= com_grad_sq;
        laplace -= com_laplace;
    }

    return (math::isgreater(laplace, 0.0) ? (grad_sq / laplace): 0.0);
}

/** ---------------------------------------------------------------------------
 * pressure_kin
 * @brief Compute fluid kinetic pressure.
 */
cl_double16 pressure_kin(std::vector<Atom> &atoms, const Domain &domain)
{
    cl_double4 velocity = compute::com_vel(atoms);
    cl_double16 pressure = cl_double16{};
    for (auto &atom : atoms) {
        /* Atom velocity relative to the fluid CoM. */
        cl_double4 vel = atom.mom * atom.rmass;
        vel -= velocity;

        pressure.s[0] += atom.mass * vel.s[0] * vel.s[0];
        pressure.s[1] += atom.mass * vel.s[0] * vel.s[1];
        pressure.s[2] += atom.mass * vel.s[0] * vel.s[2];

        pressure.s[3] += atom.mass * vel.s[1] * vel.s[0];
        pressure.s[4] += atom.mass * vel.s[1] * vel.s[1];
        pressure.s[5] += atom.mass * vel.s[1] * vel.s[2];

        pressure.s[6] += atom.mass * vel.s[2] * vel.s[0];
        pressure.s[7] += atom.mass * vel.s[2] * vel.s[1];
        pressure.s[8] += atom.mass * vel.s[2] * vel.s[2];
    }

    cl_double volume = domain.length.s[0] *
                       domain.length.s[1] *
                       domain.length.s[2];
    pressure /= volume;
    return pressure;
}

/** ---------------------------------------------------------------------------
 * pressure_vir
 * @brief Compute fluid virial pressure.
 */
cl_double16 pressure_vir(std::vector<Atom> &atoms, const Domain &domain)
{
    cl_double16 pressure = cl_double16{};
    for (auto &atom : atoms) {
        pressure += atom.virial;
    }

    cl_double volume = domain.length.s[0] *
                       domain.length.s[1] *
                       domain.length.s[2];
    pressure /= volume;
    return pressure;
}

/** ---------------------------------------------------------------------------
 * pbc
 * @brief Return the lengthic image in primary cell of the fluid domain.
 */
cl_double4 pbc(const cl_double4 &r, const Domain &domain)
{
    cl_double4 image(r);

    /* pbc along x-dimension */
    if (image.s[0] < -domain.length_half.s[0]) {
        image.s[0] += domain.length.s[0];
    }
    if (image.s[0] >  domain.length_half.s[0]) {
        image.s[0] -= domain.length.s[0];
    }

    /* pbc along y-dimension */
    if (image.s[1] < -domain.length_half.s[1]) {
        image.s[1] += domain.length.s[1];
    }
    if (image.s[1] >  domain.length_half.s[1]) {
        image.s[1] -= domain.length.s[1];
    }

    /* pbc along z-dimension */
    if (image.s[2] < -domain.length_half.s[2]) {
        image.s[2] += domain.length.s[2];
    }
    if (image.s[2] >  domain.length_half.s[2]) {
        image.s[2] -= domain.length.s[2];
    }

    return image;
}

} /* compute */
