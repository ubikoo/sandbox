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
 * @brief Create a sampler object with a specified name.
 */
Sampler::Sampler(const std::string &name)
    : m_name(name)
{}

/**
 * Sampler::stats_avrg
 * @brief Return sampler average.
 */
double Sampler::stats_avrg(void) const
{
    if (!is_valid()) {
        return 0.0;
    }

    double norm = static_cast<double>(size());
    double avrg = 0.0;
    for (auto &item : m_items) {
        avrg += item;
    }
    return avrg / norm;
}

/**
 * Sampler::stats_var
 * @brief Return sampler variance.
 */
double Sampler::stats_var(void) const
{
    if (!is_valid()) {
        return 0.0;
    }

    double norm = static_cast<double>(size());
    double avrg = stats_avrg();
    double var = 0.0;
    for (auto &item : m_items) {
        var += (item - avrg) * (item - avrg);
    }
    var /= (norm * (norm - 1.0));
    return var;
}

/**
 * Sampler::stats_sdev
 * @brief Return sampler standard deviation.
 */
double Sampler::stats_sdev(void) const
{
    if (!is_valid()) {
        return 0.0;
    }
    return std::sqrt(stats_var());
}

/**
 * Sampler::to_string
 * @brief Serialize the sampler statistics.
 */
std::string Sampler::to_string(void) const
{
    return core::str_format(
        "%20s %lf %lf",
        m_name.c_str(),
        stats_avrg(),
        stats_sdev());
}
