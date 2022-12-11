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
    for (size_t prop = 0; prop < NUM_PROPERTIES; ++prop) {
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
        double com_mass = compute::com_mass(atoms);
        math::vec3d com_pos = compute::com_pos(atoms);
        math::vec3d com_upos = compute::com_upos(atoms);
        math::vec3d com_vel = compute::com_vel(atoms);
        math::vec3d com_mom = compute::com_mom(atoms);
        math::vec3d com_force = compute::com_force(atoms);

        double density = compute::density(atoms, domain);

        double energy_kin = compute::energy_kin(atoms);
        double energy_pot = compute::energy_pot(atoms);

        double temp_grad_sq;
        double temp_laplace;
        double temperature = compute::temperature_kin(
                atoms, temp_grad_sq, temp_laplace);

        math::mat3d pressure_kin = compute::pressure_kin(atoms, domain);
        math::mat3d pressure_vir = compute::pressure_vir(atoms, domain);

        /* Fluid mass */
        m_item[COM_MASS] = com_mass;

        /* Fluid CoM position */
        m_item[COM_POS_X] = com_pos.x;
        m_item[COM_POS_Y] = com_pos.y;
        m_item[COM_POS_Z] = com_pos.z;

        /* Fluid CoM position */
        m_item[COM_UPOS_X] = com_upos.x;
        m_item[COM_UPOS_Y] = com_upos.y;
        m_item[COM_UPOS_Z] = com_upos.z;

        /* Fluid CoM velocity */
        m_item[COM_VEL_X] = com_vel.x;
        m_item[COM_VEL_Y] = com_vel.y;
        m_item[COM_VEL_Z] = com_vel.z;

        /* Fluid CoM momentum */
        m_item[COM_MOM_X] = com_mom.x;
        m_item[COM_MOM_Y] = com_mom.y;
        m_item[COM_MOM_Z] = com_mom.z;

        /* Fluid CoM force */
        m_item[COM_FORCE_X] = com_force.x;
        m_item[COM_FORCE_Y] = com_force.y;
        m_item[COM_FORCE_Z] = com_force.z;

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
        m_item[PRESSURE_KIN_XX] = pressure_kin.xx;
        m_item[PRESSURE_KIN_XY] = pressure_kin.xy;
        m_item[PRESSURE_KIN_XZ] = pressure_kin.xz;

        m_item[PRESSURE_KIN_YX] = pressure_kin.yx;
        m_item[PRESSURE_KIN_YY] = pressure_kin.yy;
        m_item[PRESSURE_KIN_YZ] = pressure_kin.yz;

        m_item[PRESSURE_KIN_ZX] = pressure_kin.zx;
        m_item[PRESSURE_KIN_ZY] = pressure_kin.zy;
        m_item[PRESSURE_KIN_ZZ] = pressure_kin.zz;

        /* Fluid virial pressure */
        m_item[PRESSURE_VIR_XX] = pressure_vir.xx;
        m_item[PRESSURE_VIR_XY] = pressure_vir.xy;
        m_item[PRESSURE_VIR_XZ] = pressure_vir.xz;

        m_item[PRESSURE_VIR_YX] = pressure_vir.yx;
        m_item[PRESSURE_VIR_YY] = pressure_vir.yy;
        m_item[PRESSURE_VIR_YZ] = pressure_vir.yz;

        m_item[PRESSURE_VIR_ZX] = pressure_vir.zx;
        m_item[PRESSURE_VIR_ZY] = pressure_vir.zy;
        m_item[PRESSURE_VIR_ZZ] = pressure_vir.zz;

        /* Fluid pressure */
        m_item[PRESSURE_XX] = pressure_kin.xx + pressure_vir.xx;
        m_item[PRESSURE_YY] = pressure_kin.yy + pressure_vir.yy;
        m_item[PRESSURE_ZZ] = pressure_kin.zz + pressure_vir.zz;
    }
    m_block_data.push_back(m_item);

    /*
     * If the number of items reaches the block segment size
     * store the block average for later processing and clear.
     */
    if (m_block_data.size() == m_block_size) {
        double block_data_size = static_cast<double>(m_block_data.size());

        /* Compute the block average. */
        Item avrg = m_sample_zero;
        for (auto &item : m_block_data) {
            for (size_t prop = 0; prop < NUM_PROPERTIES; ++prop) {
                avrg[prop] += item[prop];
            }
        }

        for (size_t prop = 0; prop < NUM_PROPERTIES; ++prop) {
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
    double m_block_avrg_size = static_cast<double>(m_block_average.size());

    /*
     * Compute sampler average.
     */
    m_sample_avrg = m_sample_zero;
    for (auto &item : m_block_average) {
        for (size_t prop = 0; prop < NUM_PROPERTIES; ++prop) {
            m_sample_avrg[prop] += item[prop];
        }
    }

    for (size_t prop = 0; prop < NUM_PROPERTIES; ++prop) {
        m_sample_avrg[prop] /= m_block_avrg_size;
    }

    /*
     * Compute sampler variance.
     */
    m_sample_sdev = m_sample_zero;
    for (auto &item : m_block_average) {
        for (size_t prop = 0; prop < NUM_PROPERTIES; ++prop) {
            double res = item[prop] - m_sample_avrg[prop];
            m_sample_sdev[prop] += res * res;
        }
    }

    for (size_t prop = 0; prop < NUM_PROPERTIES; ++prop) {
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
    for (size_t prop = 0; prop < NUM_PROPERTIES; ++prop) {
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
    for (size_t prop = 0; prop < NUM_PROPERTIES; ++prop) {
        ss << core::str_format(
            "%20s %lf\n", m_sample_name[prop].c_str(), m_item[prop]);
    }
    return ss.str();
}
