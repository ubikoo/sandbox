/*
 * sampler.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef MD_SAMPLER_H_
#define MD_SAMPLER_H_

#include "atto/opencl/opencl.hpp"
#include "base.hpp"
#include "compute.hpp"

/**
 * Sampler
 * @brief Thermodynamic sampler.
 */
struct Sampler {
    /* Item enumerated type */
    enum {
        /* Fluid mass */
        COM_MASS = 0,
        /* Fluid CoM position */
        COM_POS_X,
        COM_POS_Y,
        COM_POS_Z,
        /* Fluid CoM unfolded position */
        COM_UPOS_X,
        COM_UPOS_Y,
        COM_UPOS_Z,
        /* Fluid CoM velocity */
        COM_VEL_X,
        COM_VEL_Y,
        COM_VEL_Z,
        /* Fluid CoM momentum */
        COM_MOM_X,
        COM_MOM_Y,
        COM_MOM_Z,
        /* Fluid CoM force */
        COM_FORCE_X,
        COM_FORCE_Y,
        COM_FORCE_Z,
        /* Fluid density */
        DENSITY,
        /* Fluid energy */
        ENERGY_KIN,
        ENERGY_POT,
        /* Fluid temperature */
        TEMP_GRAD_SQ,
        TEMP_LAPLACE,
        TEMPERATURE,
        /* Fluid kinetic pressure */
        PRESSURE_KIN_XX,
        PRESSURE_KIN_XY,
        PRESSURE_KIN_XZ,
        PRESSURE_KIN_YX,
        PRESSURE_KIN_YY,
        PRESSURE_KIN_YZ,
        PRESSURE_KIN_ZX,
        PRESSURE_KIN_ZY,
        PRESSURE_KIN_ZZ,
        /* Fluid virial pressure */
        PRESSURE_VIR_XX,
        PRESSURE_VIR_XY,
        PRESSURE_VIR_XZ,
        PRESSURE_VIR_YX,
        PRESSURE_VIR_YY,
        PRESSURE_VIR_YZ,
        PRESSURE_VIR_ZX,
        PRESSURE_VIR_ZY,
        PRESSURE_VIR_ZZ,
        /* Fluid pressure */
        PRESSURE_XX,
        PRESSURE_YY,
        PRESSURE_ZZ,
        NUM_PROPERTIES
    };
    typedef std::array<std::string, NUM_PROPERTIES> ItemName;
    typedef std::array<double, NUM_PROPERTIES> Item;

    /* Sampler sample item. */
    const size_t m_block_size;
    std::vector<Item> m_block_data;
    std::vector<Item> m_block_average;

    ItemName m_sample_name;
    Item m_sample_zero;
    Item m_sample_avrg;
    Item m_sample_sdev;
    Item m_item;

    /* Reset sampler properties. */
    void reset(void);

    /* Do we have any sample items? */
    bool empty(void) const { return m_block_data.empty(); }

    /* Return a reference to the last sample item. */
    const Item &back(void) const { return m_block_data.back(); }

    /* Sample sampler properties. */
    void sample(std::vector<Atom> &atoms, const Domain &domain);

    /* Compute sampler statistics. */
    void statistics(void);

    /* Serialize the sampler statistics. */
    std::string to_string(void) const;

    /* Return a serialized version of the latest sample item. */
    std::string log_string(void) const;

    /* Constructor/destructor. */
    Sampler();
    ~Sampler() = default;
}; /* Sampler */

#endif /* MD_SAMPLER_H_ */
