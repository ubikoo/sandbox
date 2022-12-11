/*
 * compute.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef MD_COMPUTE_H_
#define MD_COMPUTE_H_

#include "atto/opencl/opencl.hpp"
#include "base.hpp"

/**
 * @brief Collection of fluid compute functions.
 */
namespace compute {

/** Compute the total mass of the fluid. */
double com_mass(std::vector<Atom> &atoms);

/** Compute CoM position of the fluid. */
atto::math::vec3d com_pos(std::vector<Atom> &atoms);

/** Compute CoM unfolded position of the fluid. */
atto::math::vec3d com_upos(std::vector<Atom> &atoms);

/** Compute CoM velocity of the fluid. */
atto::math::vec3d com_vel(std::vector<Atom> &atoms);

/** Compute total momentum of the fluid. */
atto::math::vec3d com_mom(std::vector<Atom> &atoms);

/** Compute total force on the fluid. */
atto::math::vec3d com_force(std::vector<Atom> &atoms);

/** Compute the density of the fluid. */
double density(std::vector<Atom> &atoms, const Domain &domain);

/** Compute fluid kinetic energy. */
double energy_kin(std::vector<Atom> &atoms);

/** Compute fluid potential energy. */
double energy_pot(std::vector<Atom> &atoms);

/** Compute fluid kinetic temperature. */
double temperature_kin(
    std::vector<Atom> &atoms,
    double &grad_sq,
    double &laplace);

/** Compute fluid kinetic pressure. */
atto::math::mat3d pressure_kin(std::vector<Atom> &atoms, const Domain &domain);

/** Compute fluid virial pressure. */
atto::math::mat3d pressure_vir(std::vector<Atom> &atoms, const Domain &domain);

/** Return the periodic image in primary cell of the fluid domain. */
atto::math::vec3d pbc(const atto::math::vec3d &r, const Domain &domain);

/** Compute the force on the atom with the specified index. */
void force_atom(
    const size_t atom_1,
    const size_t n_atoms,
    const size_t n_neighbours,
    std::vector<Atom> &atoms,
    const Domain &domain,
    const Field &field);

/** Compute pair interaction force. */
Pair force_pair(
    const size_t atom_1,
    const size_t atom_2,
    const atto::math::vec3d &r_12,
    const Field &field);

} /* compute */

#endif /* MD_COMPUTE_H_ */
