/*
 * model.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "model.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * Model::Model
 * @brief Create OpenCL context and associated objects.
 */
Model::Model()
{
    /*
     * Setup Model data.
     */
    {
        m_data.n_points = Params::n_points;
        m_data.n_cells = Params::n_cells;

        m_data.domain_lo = Params::domain_lo;
        m_data.domain_hi = Params::domain_hi;

        m_data.points = create_box(
            m_data.n_points,
            m_data.domain_lo.x,
            m_data.domain_lo.y,
            m_data.domain_lo.z,
            m_data.domain_hi.x,
            m_data.domain_hi.y,
            m_data.domain_hi.z);

        m_data.hashmap = std::make_unique<Hashmap>(
            Params::load_factor * Params::n_points);
    }

    // /* Setup OpenCL context on the first available platform. */
    // {
    //     /* Create a contex and get the specified device index. */
    //     m_context = cl::Context::create(CL_DEVICE_TYPE_GPU);
    //     m_device = cl::Context::get_device(m_context, Params::device_index);
    //     std::cout << cl::Device::get_info_string(m_device) << "\n";

    //     /* Create a command queue on the specified device. */
    //     m_queue = cl::Queue::create(m_context, m_device);

    //     /* Create OpenCL program. */
    //     m_program = cl::Program::create_from_file(m_context, "data/hashmap.cl");
    //     std::cout << cl::Program::get_source(m_program) << "\n";
    //     cl::Program::build(m_program, m_device, "");

    //     /* Create OpenCL kernels. */
    //     m_kernel = cl::Kernel::create(m_program, "hashmap_insert");
    // }
}

/**
 * Model::~Model
 * @brief Destroy the OpenCL context and associated objects.
 */
Model::~Model()
{
    /* Teardown OpenCL data. */
    // {
    //     for (auto &it : m_images) {
    //         cl::Memory::release(it);
    //     }
    //     for (auto &it : m_buffers) {
    //         cl::Memory::release(it);
    //     }
    //     cl::Kernel::release(m_kernel_hashmap_insert);
    //     cl::Program::release(m_program);
    //     cl::Queue::release(m_queue);
    //     cl::Device::release(m_device);
    //     cl::Context::release(m_context);
    // }
}

/** ---------------------------------------------------------------------------
 * Model::execute
 * @brief Execute the model.
 */
void Model::execute(void)
{
    /**
     * hash
     * Compute the hash value of a cell index.
     */
    auto hash = [&] (const math::vec3u32 &v) {
        const uint32_t c1 = static_cast<uint32_t>(73856093);
        const uint32_t c2 = static_cast<uint32_t>(19349663);
        const uint32_t c3 = static_cast<uint32_t>(83492791);

        uint32_t h1 = (v.x * c1);
        uint32_t h2 = (v.y * c2);
        uint32_t h3 = (v.z * c3);

        return (h1 ^ h2 ^ h3);
        // return (7*v1 + 503*v2 + 24847*v3);
    };

    /**
     * cell_ix
     * Compute the cell index of a given point.
     */
    auto cell_ix = [&] (const math::vec3f &point) {
        math::vec3f u_point = (point - m_data.domain_lo);
        u_point /= (m_data.domain_hi - m_data.domain_lo);
        u_point *= (float) m_data.n_cells;

        const uint32_t v1 = static_cast<uint32_t>(u_point.x);
        const uint32_t v2 = static_cast<uint32_t>(u_point.y);
        const uint32_t v3 = static_cast<uint32_t>(u_point.z);

        return math::vec3u32(v1, v2, v3);
    };

    /*
     * Create a set of points inside the domain.
     */
    m_data.points = create_box(
        m_data.n_points,
        m_data.domain_lo.x,
        m_data.domain_lo.y,
        m_data.domain_lo.z,
        m_data.domain_hi.x,
        m_data.domain_hi.y,
        m_data.domain_hi.z);

    /*
     * Create the hashmap from the array of particles.
     */
    m_data.hashmap->clear();
    for (size_t i = 0; i < m_data.points.size(); i++) {
         m_data.hashmap->insert(hash(cell_ix(m_data.points[i])), (uint32_t) i);
    }

    /* Query the hashmap data */
    {
        core_assert(m_data.hashmap->size() == m_data.n_points, "invalid size");

        uint32_t count = 0;
        for (auto &slot : m_data.hashmap->data()) {
            if (slot.key != m_data.hashmap->end()) {
                std::cout << core::str_format(
                    "count %12u, key %12u index %12u value %12u\n",
                    count++,
                    slot.key,
                    slot.key % m_data.hashmap->capacity(),
                    slot.value);
            }
        }
    }

    /* Query the particles in the hashmap. */
    {
        for (uint32_t i = 0; i < m_data.n_cells; ++i) {
            for (uint32_t j = 0; j < m_data.n_cells; ++j) {
                for (uint32_t k = 0; k < m_data.n_cells; ++k) {
                    uint32_t key = hash(math::vec3u32(i, j, k));
                    uint32_t slot = m_data.hashmap->begin(key);
                    while (slot != m_data.hashmap->end()) {
                        uint32_t value = m_data.hashmap->get(slot);
                        std::cout << core::str_format(
                            "%3u %3u %3u, key %12u, slot %12u, value %12u\n",
                            i, j, k, key, slot, value);
                        slot = m_data.hashmap->next(key, slot);
                    }
                }
            }
        }
    }
}

/** ---------------------------------------------------------------------------
 * Model::handle
 * Handle an event.
 */
void Model::handle(const gl::Event &event)
{}
