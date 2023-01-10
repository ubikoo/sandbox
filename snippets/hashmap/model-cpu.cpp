/*
 * model-cpu.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include <vector>
#include <utility>
#include <algorithm>
#include "ito/opengl.hpp"
#include "ito/opencl.hpp"
#include "point.hpp"
#include "params.hpp"
#include "hashmap.hpp"
#include "model-cpu.hpp"

using namespace ito;

/**
 * @brief Create a Hashmap model executing on the CPU.
 */
ModelCPU ModelCPU::Create()
{
    ModelCPU model;
    model.m_hashmap = Hashmap::Create(Params::kLoadFactor * Params::kNumPoints);
    model.m_keys.resize(Params::kNumPoints, {0, 0});
    return model;
}

/**
 * @brief Destroy the Hashmap model.
 */
void ModelCPU::Destroy(ModelCPU &model)
{}

/**
 * @brief Execute the model.
 */
void ModelCPU::Execute(const std::vector<Point> &points)
{
    /**
     * @brief Compute the hash value of a cell index.
     */
    auto hash = [&] (const cl_uint3 &v) -> uint32_t {
        const uint32_t c1 = static_cast<uint32_t>(73856093);
        const uint32_t c2 = static_cast<uint32_t>(19349663);
        const uint32_t c3 = static_cast<uint32_t>(83492791);

        uint32_t h1 = c1 * v.s[0];
        uint32_t h2 = c2 * v.s[1];
        uint32_t h3 = c3 * v.s[2];
        return (h1 ^ h2 ^ h3);  // return (7*h1 + 503*h2 + 24847*h3);
    };


    /**
     * @brief Compute the cell index of a given point.
     */
    auto cell_ix = [&] (const cl_float3 &p) -> cl_uint3 {
        cl_float3 u = (p - Params::kDomainLo);
        u /= (Params::kDomainHi - Params::kDomainLo);
        u *= (cl_float) Params::kNumCells;

        const uint32_t v1 = static_cast<uint32_t>(u.s[0]);
        const uint32_t v2 = static_cast<uint32_t>(u.s[1]);
        const uint32_t v3 = static_cast<uint32_t>(u.s[2]);
        return (cl_uint3) {v1, v2, v3};
    };

    /*
     * Create the hashmap from the array of particles.
     */
    m_hashmap.clear();
    for (size_t i = 0; i < Params::kNumPoints; i++) {
         m_hashmap.insert(hash(cell_ix(points[i].pos)), (uint32_t) i);
    }

    /*
     * Query the hashmap for all valid key-value pairs.
     */
    {
        std::fill(m_keys.begin(), m_keys.end(), std::make_pair(0, 0));
        for (auto &slot : m_hashmap.data()) {
            if (slot.key != m_hashmap.end()) {
                m_keys[slot.value] = std::make_pair(
                    slot.key, slot.key % Params::kCapacity);
            }
        }
    }
}
