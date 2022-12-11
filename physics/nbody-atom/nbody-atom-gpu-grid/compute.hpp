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
cl_double com_mass(std::vector<Atom> &atoms);

/** Compute CoM position of the fluid. */
cl_double4 com_pos(std::vector<Atom> &atoms);

/** Compute CoM unfolded position of the fluid. */
cl_double4 com_upos(std::vector<Atom> &atoms);

/** Compute CoM velocity of the fluid. */
cl_double4 com_vel(std::vector<Atom> &atoms);

/** Compute total momentum of the fluid. */
cl_double4 com_mom(std::vector<Atom> &atoms);

/** Compute total force on the fluid. */
cl_double4 com_force(std::vector<Atom> &atoms);

/** Compute the density of the fluid. */
cl_double density(std::vector<Atom> &atoms, const Domain &domain);

/** Compute fluid kinetic energy. */
cl_double energy_kin(std::vector<Atom> &atoms);

/** Compute fluid potential energy. */
cl_double energy_pot(std::vector<Atom> &atoms);

/** Compute fluid kinetic temperature. */
cl_double temperature_kin(
    std::vector<Atom> &atoms,
    cl_double &grad_sq,
    cl_double &laplace);

/** Compute fluid kinetic pressure. */
cl_double16 pressure_kin(std::vector<Atom> &atoms, const Domain &domain);

/** Compute fluid virial pressure. */
cl_double16 pressure_vir(std::vector<Atom> &atoms, const Domain &domain);

/** Return the periodic image in primary cell of the fluid domain. */
cl_double4 pbc(const cl_double4 &r, const Domain &domain);

} /* compute */

#endif /* MD_COMPUTE_H_ */
