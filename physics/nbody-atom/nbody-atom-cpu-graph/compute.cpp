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
double com_mass(std::vector<Atom> &atoms)
{
    double mass = 0.0;
    for (auto &atom : atoms) {
        mass += atom.mass;
    }
    return mass;
}

/** ---------------------------------------------------------------------------
 * com_pos
 * @brief Compute CoM position of the fluid.
 */
math::vec3d com_pos(std::vector<Atom> &atoms)
{
    double mass = 0.0;
    math::vec3d pos = math::vec3d{};
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
math::vec3d com_upos(std::vector<Atom> &atoms)
{
    double mass = 0.0;
    math::vec3d upos = math::vec3d{};
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
math::vec3d com_vel(std::vector<Atom> &atoms)
{
    double mass = 0.0;
    math::vec3d mom = math::vec3d{};
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
math::vec3d com_mom(std::vector<Atom> &atoms)
{
    math::vec3d mom = math::vec3d{};
    for (auto &atom : atoms) {
        mom += atom.mom;
    }
    return mom;
}

/** ---------------------------------------------------------------------------
 * com_force
 * @brief Compute total force on the fluid.
 */
math::vec3d com_force(std::vector<Atom> &atoms)
{
    math::vec3d force = math::vec3d{};
    for (auto &atom : atoms) {
        force += atom.force;
    }
    return force;
}

/** ---------------------------------------------------------------------------
 * density
 * @brief Compute the density of the fluid.
 */
double density(std::vector<Atom> &atoms, const Domain &domain)
{
    double volume = domain.length.x *
                    domain.length.y *
                    domain.length.z;
    double density = com_mass(atoms) / volume;
    return density;
}

/** ---------------------------------------------------------------------------
 * energy_kin
 * @brief Compute fluid kinetic energy.
 */
double energy_kin(std::vector<Atom> &atoms)
{
    double energy = 0.0;
    for (auto &atom : atoms) {
        energy += math::dot(atom.mom, atom.mom) * atom.rmass;
    }
    return 0.5 * energy;
}

/** ---------------------------------------------------------------------------
 * energy_pot
 * @brief Compute fluid potential energy.
 */
double energy_pot(std::vector<Atom> &atoms)
{
    double energy = 0.0;
    for (auto &atom : atoms) {
        energy += atom.energy;
    }
    return energy;
}

/** ---------------------------------------------------------------------------
 * temperature_kin
 * @brief Compute fluid kinetic temperature.
 */
double temperature_kin(
    std::vector<Atom> &atoms,
    double &grad_sq,
    double &laplace)
{
    /* Compute kinetic temperature of the fluid. */
    grad_sq = 0.0;
    laplace = 0.0;
    for (auto &atom : atoms) {
        grad_sq += math::dot(atom.mom, atom.mom) * atom.rmass;
        laplace += 3.0;
    }

    /*
     * If fluid size is more than one atom, remove the CoM momentum
     * contribution to the total kinetic energy and account for the
     * correct number of degrees of freedom.
     */
    if (atoms.size() > 1) {
        double mass = compute::com_mass(atoms);
        math::vec3d mom = compute::com_mom(atoms);

        double com_grad_sq = math::dot(mom, mom) / mass;
        double com_laplace = 3.0;

        grad_sq -= com_grad_sq;
        laplace -= com_laplace;
    }

    return (math::isgreater(laplace, 0.0) ? (grad_sq / laplace): 0.0);
}

/** ---------------------------------------------------------------------------
 * pressure_kin
 * @brief Compute fluid kinetic pressure.
 */
math::mat3d pressure_kin(std::vector<Atom> &atoms, const Domain &domain)
{
    math::vec3d velocity = compute::com_vel(atoms);
    math::mat3d pressure = math::mat3d{};
    for (auto &atom : atoms) {
        /* Atom velocity relative to the fluid CoM. */
        math::vec3d vel = atom.mom * atom.rmass;
        vel -= velocity;

        pressure.xx += atom.mass * vel.x * vel.x;
        pressure.xy += atom.mass * vel.x * vel.y;
        pressure.xz += atom.mass * vel.x * vel.z;

        pressure.yx += atom.mass * vel.y * vel.x;
        pressure.yy += atom.mass * vel.y * vel.y;
        pressure.yz += atom.mass * vel.y * vel.z;

        pressure.zx += atom.mass * vel.z * vel.x;
        pressure.zx += atom.mass * vel.z * vel.y;
        pressure.zz += atom.mass * vel.z * vel.z;
    }

    double volume = domain.length.x *
                    domain.length.y *
                    domain.length.z;
    pressure /= volume;
    return pressure;
}

/** ---------------------------------------------------------------------------
 * pressure_vir
 * @brief Compute fluid virial pressure.
 */
math::mat3d pressure_vir(std::vector<Atom> &atoms, const Domain &domain)
{
    math::mat3d pressure = math::mat3d{};
    for (auto &atom : atoms) {
        pressure += atom.virial;
    }

    double volume = domain.length.x *
                    domain.length.y *
                    domain.length.z;
    pressure /= volume;
    return pressure;
}

/** ---------------------------------------------------------------------------
 * pbc
 * @brief Return the lengthic image in primary cell of the fluid domain.
 */
math::vec3d pbc(const math::vec3d &r, const Domain &domain)
{
    math::vec3d image(r);

    /* pbc along x-dimension */
    if (image.x < -domain.length_half.x) {
        image.x += domain.length.x;
    }
    if (image.x >  domain.length_half.x) {
        image.x -= domain.length.x;
    }

    /* pbc along y-dimension */
    if (image.y < -domain.length_half.y) {
        image.y += domain.length.y;
    }
    if (image.y >  domain.length_half.y) {
        image.y -= domain.length.y;
    }

    /* pbc along z-dimension */
    if (image.z < -domain.length_half.z) {
        image.z += domain.length.z;
    }
    if (image.z >  domain.length_half.z) {
        image.z -= domain.length.z;
    }

    return image;
}

/** ---------------------------------------------------------------------------
 * force_atom
 * @brief Compute the force on the atom with the specified index.
 */
void force_atom(
    const size_t atom_1,
    const size_t n_atoms,
    const size_t n_neighbours,
    std::vector<Atom> &atoms,
    const Domain &domain,
    const Field &field,
    const Graph &graph)
{
    const double r_cut_sq = field.r_cut * field.r_cut;

    atoms[atom_1].force = math::vec3d{};
    atoms[atom_1].energy = 0.0;
    atoms[atom_1].virial = math::mat3d{};

    const size_t begin = atom_1 * n_neighbours;
    const size_t end = begin + n_neighbours;

    size_t pair_ix = begin;
    for (auto &atom_2 : graph.neighbours(atom_1)) {
        if (atom_1 == atom_2) {
            continue;
        }

        math::vec3d r_12 = atoms[atom_1].pos - atoms[atom_2].pos;
        r_12 = compute::pbc(r_12, domain);

        if (pair_ix < end && dot(r_12, r_12) < r_cut_sq) {
            Pair pair = force_pair(atom_1, atom_2, r_12, field);
            atoms[atom_1].force  -= pair.gradient;
            atoms[atom_1].energy += pair.energy * 0.5;
            atoms[atom_1].virial += pair.virial * 0.5;
            pair_ix++;
        }
    }
}

/**
 * force_pair
 * @brief Compute pair interaction force.
 */
Pair force_pair(
    const size_t atom_1,
    const size_t atom_2,
    const math::vec3d &r_12,
    const Field &field)
{
    /* Compute pair attributes - energy, gradient, laplacian and virial. */
    Pair pair;

    pair.atom_1 = atom_1;
    pair.atom_2 = atom_2;
    pair.r_12 = r_12;

    pair.energy = 0.0;
    pair.gradient = math::vec3d{};
    pair.laplace = math::vec3d{};
    pair.virial = math::mat3d{};

    /* Interaction coefficients. */
    const double epsilon = field.epsilon;
    const double sigma = field.sigma;
    const double r_hard = field.r_hard;

    const double sigma_sq = sigma * sigma;
    const double r_hard_sq = r_hard * r_hard;

    /* Define pair energy and force coefficients */
    double energy_coeff = 4.0 * field.epsilon;
    double force_coeff = 24.0 * field.epsilon / sigma_sq;

    /* Ensure pair distance is larger than the hard sphere radius. */
    double energy_hard_sphere = 0.0;
    double r_12_sq = math::dot(r_12, r_12);
    if (r_12_sq < r_hard_sq) {
        /* Compute the hard sphere energy correction. */
        double r_12_len = std::sqrt(r_12_sq);

        double r_hard_2  = sigma_sq / r_hard_sq;
        double r_hard_4  = r_hard_2 * r_hard_2;
        double r_hard_6  = r_hard_4 * r_hard_2;
        double r_hard_12 = r_hard_6 * r_hard_6;

        energy_hard_sphere = -24.0 * epsilon * (2.0 * r_hard_12 - r_hard_6);
        energy_hard_sphere *= (r_12_len - r_hard) / r_12_len;

        /* Set the pair distance to the hard sphere radius. */
        r_12_sq = r_hard_sq;
    }

    double rr2  = sigma_sq / r_12_sq;
    double rr4  = rr2 * rr2;
    double rr6  = rr4 * rr2;
    double rr8  = rr4 * rr4;
    double rr12 = rr6 * rr6;
    double rr14 = rr8 * rr6;

    /* LJ energy */
    pair.energy = energy_coeff * (rr12 - rr6) + energy_hard_sphere;

    /* LJ gradient */
    pair.gradient = r_12;
    pair.gradient *= -force_coeff * (2.0 * rr14 - rr8);

    /* LJ laplacian */
    double laplace_c1 = force_coeff * (28.0 * rr14 - 8.0 * rr8) / r_12_sq;
    double laplace_c2 = force_coeff * (2.0 * rr14 - rr8);

    pair.laplace.x = laplace_c1 * r_12.x * r_12.x - laplace_c2;
    pair.laplace.y = laplace_c1 * r_12.y * r_12.y - laplace_c2;
    pair.laplace.z = laplace_c1 * r_12.z * r_12.z - laplace_c2;

    /* LJ virial */
    pair.virial.xx = -r_12.x * pair.gradient.x;
    pair.virial.xy = -r_12.x * pair.gradient.y;
    pair.virial.xz = -r_12.x * pair.gradient.z;

    pair.virial.yx = -r_12.y * pair.gradient.x;
    pair.virial.yy = -r_12.y * pair.gradient.y;
    pair.virial.yz = -r_12.y * pair.gradient.z;

    pair.virial.zx = -r_12.z * pair.gradient.x;
    pair.virial.zy = -r_12.z * pair.gradient.y;
    pair.virial.zz = -r_12.z * pair.gradient.z;

    return pair;
}

} /* compute */
