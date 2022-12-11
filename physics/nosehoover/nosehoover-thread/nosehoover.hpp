/*
 * nosehoover.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef NOSEHOOVER_H_
#define NOSEHOOVER_H_

#include <vector>
#include "base.hpp"

struct NoseHoover {
    atto::math::vec3d x_state;  /* thermostat state */
    double mass;                /* mass */
    double kappa;               /* force constant */
    double tau;                 /* thermostat constant */
    double temp;                /* thermostat temperature */

    /** Compute Nose-Hoover integration step. */
    void step(double t_step, double max_err, uint64_t max_iter);

    /** Compute Nose-Hoover time derivative. */
    atto::math::vec3d deriv(atto::math::vec3d &x);

    /** @brief Generate a collection of Nose-Hoover thermostats. */
    static std::vector<NoseHoover> generate(void);
};

#endif /* NOSEHOOVER_H_ */
