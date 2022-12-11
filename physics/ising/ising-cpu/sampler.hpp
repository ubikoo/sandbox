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

#ifndef ISING_SAMPLER_H_
#define ISING_SAMPLER_H_

#include "atto/opencl/opencl.hpp"

/**
 * Sampler
 * @brief Sampler represents a sampling object storing a set of values of a
 * named property. Given a set of n values, compute the statistics of the
 * sampled set - average, variance, standard deviation, etc.
 */
struct Sampler {
    /* Sampler name and array of items. */
    std::string m_name;
    std::vector<double> m_items;

    /** Clear the sampler data. */
    void clear(void) { m_items.clear(); }

    /** Return the number of items in the sampler. */
    size_t size(void) const { return m_items.size(); }

    /** Do we have any items in the sampler? */
    bool is_empty(void) const { return m_items.empty(); }

    /** Do we have a valid (at least 2) set of items ? */
    bool is_valid(void) const { return m_items.size() > 1; }

    /** Return the value of the last item. */
    double peek(void) const { return is_empty() ? 0.0 : m_items.back(); }

    /** Add a new value to the sampler items. */
    void add(const double item) { m_items.push_back(item); }

    /** Return sampler average. */
    double stats_avrg(void) const;

    /** Return sampler variance. */
    double stats_var(void) const;

    /** Return sampler standard deviation. */
    double stats_sdev(void) const;

    /** Serialize the sampler statistics. */
    std::string to_string(void) const;

    /* Constructor/destructor. */
    Sampler() = default;
    explicit Sampler(const std::string &name);
    ~Sampler() = default;
};

#endif /* ISING_SAMPLER_H_ */
