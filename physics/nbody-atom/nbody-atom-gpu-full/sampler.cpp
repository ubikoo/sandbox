/*
 * sampler.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "atto/opencl/opencl.hpp"
#include "sampler.hpp"
using namespace atto;

/**
 * Sampler::Sampler
 * @brief Create a thermodynamic sampler object with a specified block size.
 */
Sampler::Sampler()
    : m_block_size(Params::sample_block_size)
{
    m_sample_name = {
        /* Fluid mass */
        "com_mass",
        /* Fluid CoM position */
        "com_pos_x",
        "com_pos_y",
        "com_pos_z",
        /* Fluid CoM unfolded position */
        "com_upos_x",
        "com_upos_y",
        "com_upos_z",
        /* Fluid CoM velocity */
        "com_vel_x",
        "com_vel_y",
        "com_vel_z",
        /* Fluid CoM momentum */
        "com_mom_x",
        "com_mom_y",
        "com_mom_z",
        /* Fluid CoM force */
        "com_force_x",
        "com_force_y",
        "com_force_z",
        /* Fluid density */
        "density",
        /* Fluid energy */
        "energy_kin",
        "energy_pot",
        /* Fluid temperature */
        "temp_grad_sq",
        "temp_laplace",
        "temperature",
        /* Fluid kinetic pressure */
        "pressure_kin_xx",
        "pressure_kin_xy",
        "pressure_kin_xz",
        "pressure_kin_yx",
        "pressure_kin_yy",
        "pressure_kin_yz",
        "pressure_kin_zx",
        "pressure_kin_zy",
        "pressure_kin_zz",
        /* Fluid virial pressure */
        "pressure_vir_xx",
        "pressure_vir_xy",
        "pressure_vir_xz",
        "pressure_vir_yx",
        "pressure_vir_yy",
        "pressure_vir_yz",
        "pressure_vir_zx",
        "pressure_vir_zy",
        "pressure_vir_zz",
        /* Fluid pressure */
        "pressure_xx",
        "pressure_yy",
        "pressure_zz",
    };

    /* Reset sampler properties */
    reset();
}

/**
 * Sampler::reset
 * @brief Reset sampler properties.
 */
void Sampler::reset(void)
{
    m_block_data.clear();
    m_block_average.clear();
    for (cl_ulong prop = 0; prop < NUM_PROPERTIES; ++prop) {
        m_sample_zero[prop] = 0.0;
        m_sample_avrg[prop] = 0.0;
        m_sample_sdev[prop] = 0.0;
    }
    m_item = Item{};
}

/**
 * Sampler::sample
 * @brief Sample sampler properties.
 */
void Sampler::sample(std::vector<Atom> &atoms, const Domain &domain)
{
    /* Sample a new sampler block item */
    {
        cl_double com_mass = compute::com_mass(atoms);
        cl_double4 com_pos = compute::com_pos(atoms);
        cl_double4 com_upos = compute::com_upos(atoms);
        cl_double4 com_vel = compute::com_vel(atoms);
        cl_double4 com_mom = compute::com_mom(atoms);
        cl_double4 com_force = compute::com_force(atoms);

        cl_double density = compute::density(atoms, domain);

        cl_double energy_kin = compute::energy_kin(atoms);
        cl_double energy_pot = compute::energy_pot(atoms);

        cl_double temp_grad_sq;
        cl_double temp_laplace;
        cl_double temperature = compute::temperature_kin(
                atoms, temp_grad_sq, temp_laplace);

        cl_double16 pressure_kin = compute::pressure_kin(atoms, domain);
        cl_double16 pressure_vir = compute::pressure_vir(atoms, domain);

        /* Fluid mass */
        m_item[COM_MASS] = com_mass;

        /* Fluid CoM position */
        m_item[COM_POS_X] = com_pos.s[0];
        m_item[COM_POS_Y] = com_pos.s[1];
        m_item[COM_POS_Z] = com_pos.s[2];

        /* Fluid CoM position */
        m_item[COM_UPOS_X] = com_upos.s[0];
        m_item[COM_UPOS_Y] = com_upos.s[1];
        m_item[COM_UPOS_Z] = com_upos.s[2];

        /* Fluid CoM velocity */
        m_item[COM_VEL_X] = com_vel.s[0];
        m_item[COM_VEL_Y] = com_vel.s[1];
        m_item[COM_VEL_Z] = com_vel.s[2];

        /* Fluid CoM momentum */
        m_item[COM_MOM_X] = com_mom.s[0];
        m_item[COM_MOM_Y] = com_mom.s[1];
        m_item[COM_MOM_Z] = com_mom.s[2];

        /* Fluid CoM force */
        m_item[COM_FORCE_X] = com_force.s[0];
        m_item[COM_FORCE_Y] = com_force.s[1];
        m_item[COM_FORCE_Z] = com_force.s[2];

        /* Fluid energy */
        m_item[ENERGY_KIN] = energy_kin;
        m_item[ENERGY_POT] = energy_pot;

        /* Fluid density */
        m_item[DENSITY] = density;

        /* Fluid temperature */
        m_item[TEMP_GRAD_SQ] = temp_grad_sq;
        m_item[TEMP_LAPLACE] = temp_laplace;
        m_item[TEMPERATURE] = temperature;

        /* Fluid kinetic pressure */
        m_item[PRESSURE_KIN_XX] = pressure_kin.s[0];
        m_item[PRESSURE_KIN_XY] = pressure_kin.s[1];
        m_item[PRESSURE_KIN_XZ] = pressure_kin.s[2];

        m_item[PRESSURE_KIN_YX] = pressure_kin.s[3];
        m_item[PRESSURE_KIN_YY] = pressure_kin.s[4];
        m_item[PRESSURE_KIN_YZ] = pressure_kin.s[5];

        m_item[PRESSURE_KIN_ZX] = pressure_kin.s[6];
        m_item[PRESSURE_KIN_ZY] = pressure_kin.s[7];
        m_item[PRESSURE_KIN_ZZ] = pressure_kin.s[8];

        /* Fluid virial pressure */
        m_item[PRESSURE_VIR_XX] = pressure_vir.s[0];
        m_item[PRESSURE_VIR_XY] = pressure_vir.s[1];
        m_item[PRESSURE_VIR_XZ] = pressure_vir.s[2];

        m_item[PRESSURE_VIR_YX] = pressure_vir.s[3];
        m_item[PRESSURE_VIR_YY] = pressure_vir.s[4];
        m_item[PRESSURE_VIR_YZ] = pressure_vir.s[5];

        m_item[PRESSURE_VIR_ZX] = pressure_vir.s[6];
        m_item[PRESSURE_VIR_ZY] = pressure_vir.s[7];
        m_item[PRESSURE_VIR_ZZ] = pressure_vir.s[8];

        /* Fluid pressure */
        m_item[PRESSURE_XX] = pressure_kin.s[0] + pressure_vir.s[0];
        m_item[PRESSURE_YY] = pressure_kin.s[4] + pressure_vir.s[4];
        m_item[PRESSURE_ZZ] = pressure_kin.s[8] + pressure_vir.s[8];
    }
    m_block_data.push_back(m_item);

    /*
     * If the number of items reaches the block segment size
     * store the block average for later processing and clear.
     */
    if (m_block_data.size() == m_block_size) {
        cl_double block_data_size = static_cast<cl_double>(m_block_data.size());

        /* Compute the block average. */
        Item avrg = m_sample_zero;
        for (auto &item : m_block_data) {
            for (cl_ulong prop = 0; prop < NUM_PROPERTIES; ++prop) {
                avrg[prop] += item[prop];
            }
        }

        for (cl_ulong prop = 0; prop < NUM_PROPERTIES; ++prop) {
            avrg[prop] /= block_data_size;
        }

        m_block_average.push_back(avrg);

        /* Clear the block data for the next segment. */
        m_block_data.clear();
    }
}

/**
 * Sampler::statistics
 * @brief Compute sampler statistics.
 */
void Sampler::statistics(void)
{
    /* Statistics are undefined if we have less than two block segments. */
    if (m_block_average.size() < 2) {
        return;
    }
    cl_double m_block_avrg_size = static_cast<cl_double>(m_block_average.size());

    /*
     * Compute sampler average.
     */
    m_sample_avrg = m_sample_zero;
    for (auto &item : m_block_average) {
        for (cl_ulong prop = 0; prop < NUM_PROPERTIES; ++prop) {
            m_sample_avrg[prop] += item[prop];
        }
    }

    for (cl_ulong prop = 0; prop < NUM_PROPERTIES; ++prop) {
        m_sample_avrg[prop] /= m_block_avrg_size;
    }

    /*
     * Compute sampler variance.
     */
    m_sample_sdev = m_sample_zero;
    for (auto &item : m_block_average) {
        for (cl_ulong prop = 0; prop < NUM_PROPERTIES; ++prop) {
            cl_double res = item[prop] - m_sample_avrg[prop];
            m_sample_sdev[prop] += res * res;
        }
    }

    for (cl_ulong prop = 0; prop < NUM_PROPERTIES; ++prop) {
        m_sample_sdev[prop] /= m_block_avrg_size * (m_block_avrg_size - 1.0);
        m_sample_sdev[prop]  = std::sqrt(m_sample_sdev[prop]);
    }
}

/**
 * Sampler::to_string
 * @brief Serialize the sampler statistics.
 */
std::string Sampler::to_string(void) const
{
    std::ostringstream ss;
    for (cl_ulong prop = 0; prop < NUM_PROPERTIES; ++prop) {
        ss << core::str_format(
            "%20s %lf %lf\n",
            m_sample_name[prop].c_str(),
            m_sample_avrg[prop],
            m_sample_sdev[prop]);
    }
    return ss.str();
}

/**
 * Sampler::log_string
 * @brief Return a serialized version of the latest sample item.
 */
std::string Sampler::log_string(void) const
{
    /* Return the a string of the last sample item. */
    std::ostringstream ss;
    for (cl_ulong prop = 0; prop < NUM_PROPERTIES; ++prop) {
        ss << core::str_format(
            "%20s %lf\n", m_sample_name[prop].c_str(), m_item[prop]);
    }
    return ss.str();
}
