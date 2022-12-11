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

    {
        m_cldata.n_points = m_data.n_points;
        m_cldata.n_cells = m_data.n_cells;
        m_cldata.capacity = m_data.hashmap->capacity();

        m_cldata.domain_lo = (cl_float3) {
            Params::domain_lo.x, Params::domain_lo.y, Params::domain_lo.z};
        m_cldata.domain_hi = (cl_float3) {
            Params::domain_hi.x, Params::domain_hi.y, Params::domain_hi.z};

        m_cldata.points.resize(m_data.n_points);
        m_cldata.hashmap.resize(m_data.hashmap->capacity());
    }

    /*
     * Setup OpenCL data.
     */
    {
        /* Create a context with a command queue on the specified device */
        m_context = cl::Context::create(CL_DEVICE_TYPE_GPU);
        m_device = cl::Context::get_device(m_context, Params::device_index);
        m_queue = cl::Queue::create(m_context, m_device);
        std::cout << cl::Device::get_info_string(m_device) << "\n";

        /* Create the program object. */
        m_program = cl::Program::create_from_file(m_context, "data/hashmap.cl");
        cl::Program::build(m_program, m_device, "");
        std::cout << cl::Program::get_source(m_program) << "\n";

        /* Create the hashmap insert kernel in the program object. */
        m_kernel_hashmap_insert = cl::Kernel::create(m_program, "hashmap_insert");

        /* Create memory buffers. */
        m_buffers.resize(NumBuffers, NULL);

        m_buffers[BufferHashmap] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            m_cldata.capacity * sizeof(KeyValue),
            (void *) NULL);

        m_buffers[BufferPoints] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_ONLY,
            m_cldata.n_points * sizeof(Point),
            (void *) NULL);
    }
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
    execute_cpu();
    execute_gpu();
    for (size_t i = 0; i < m_data.n_points; i++) {
        // std::cout << core::str_format(
        //                 "%lu: cpu %12u %12u, gpu %12u %12u\n",
        //                 i,
        //                 m_data.keys[i].first,
        //                 m_data.keys[i].second,
        //                 m_cldata.keys[i].first,
        //                 m_cldata.keys[i].second);
        core_assert(m_data.keys[i].first == m_cldata.keys[i].first, "FAIL");
        core_assert(m_data.keys[i].second == m_cldata.keys[i].second, "FAIL");
    }
}

/**
 * Model::execute_cpu
 */
void Model::execute_cpu(void)
{
    /**
     * hash
     * Compute the hash value of a cell index.
     */
    auto hash = [&] (const math::vec3u32 &v) {
        const uint32_t c1 = static_cast<uint32_t>(73856093);
        const uint32_t c2 = static_cast<uint32_t>(19349663);
        const uint32_t c3 = static_cast<uint32_t>(83492791);

        uint32_t h1 = c1 * v.x;
        uint32_t h2 = c2 * v.y;
        uint32_t h3 = c3 * v.z;

        return (h1 ^ h2 ^ h3);
        // return (7*h1 + 503*h2 + 24847*h3);
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
        m_data.keys.resize(m_data.n_points);
        for (auto &slot : m_data.hashmap->data()) {
            if (slot.key != m_data.hashmap->end()) {
                // std::cout << core::str_format(
                //     "cpu: count %12u, key %12u index %12u value %12u, "
                //     "%10.6f %10.6f %10.6f\n",
                //     count++,
                //     slot.key,
                //     slot.key % m_data.hashmap->capacity(),
                //     slot.value,
                //     m_data.points[slot.value].x,
                //     m_data.points[slot.value].y,
                //     m_data.points[slot.value].z);

                m_data.keys[slot.value] = std::make_pair(
                    slot.key, (slot.key % m_cldata.capacity));
            }
        }
    }
}

/**
 * Model::execute_gpu
 */
void Model::execute_gpu(void)
{
    /* Setup hashmap and point data. */
    std::fill(
        m_cldata.hashmap.begin(),
        m_cldata.hashmap.end(),
        KeyValue{0xffffffff, 0xffffffff});

    for (size_t i = 0; i < m_cldata.n_points; ++i) {
        m_cldata.points[i] = {
            (cl_float) m_data.points[i].x,
            (cl_float) m_data.points[i].y,
            (cl_float) m_data.points[i].z,
            0.0f,
            0.0f,
            0.0f};
    }

    /* Update the gpu buffer store. */
    cl::Queue::enqueue_write_buffer(
        m_queue,
        m_buffers[BufferHashmap],
        CL_TRUE,
        0,
        m_cldata.capacity * sizeof(KeyValue),
        (void *) &m_cldata.hashmap[0],
        NULL,
        NULL);

    cl::Queue::enqueue_write_buffer(
        m_queue,
        m_buffers[BufferPoints],
        CL_TRUE,
        0,
        m_cldata.n_points * sizeof(Point),
        (void *) &m_cldata.points[0],
        NULL,
        NULL);

    /* Enqueue hashmap insert kernel. */
    {
        /* Set kernel arguments. */
        cl::Kernel::set_arg(m_kernel_hashmap_insert, 0, sizeof(cl_mem), &m_buffers[BufferHashmap]);
        cl::Kernel::set_arg(m_kernel_hashmap_insert, 1, sizeof(cl_mem), &m_buffers[BufferPoints]);
        cl::Kernel::set_arg(m_kernel_hashmap_insert, 2, sizeof(cl_uint), &m_cldata.capacity);
        cl::Kernel::set_arg(m_kernel_hashmap_insert, 3, sizeof(cl_uint), &m_cldata.n_points);
        cl::Kernel::set_arg(m_kernel_hashmap_insert, 4, sizeof(cl_uint), &m_cldata.n_cells);
        cl::Kernel::set_arg(m_kernel_hashmap_insert, 5, sizeof(cl_float3), &m_cldata.domain_lo);
        cl::Kernel::set_arg(m_kernel_hashmap_insert, 6, sizeof(cl_float3), &m_cldata.domain_hi);

        /* Set the size of the NDRange workgroups */
        cl::NDRange local_ws(Params::work_group_size);
        cl::NDRange global_ws(cl::NDRange::Roundup(
            m_cldata.n_points, Params::work_group_size));

        /* Run the kernel */
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernel_hashmap_insert,
            cl::NDRange::Null,
            global_ws,
            local_ws,
            NULL,
            NULL);

        /* Wait for kernel to compute */
        cl::Queue::finish(m_queue);
    }

    /* Read the hashmap buffer back to the host. */
    cl::Queue::enqueue_read_buffer(
        m_queue,
        m_buffers[BufferHashmap],
        CL_TRUE,
        0,
        m_cldata.capacity * sizeof(KeyValue),
        (void *) &m_cldata.hashmap[0],
        NULL,
        NULL);

    /* Query the hashmap data */
    {
        cl_uint count = 0;
        m_cldata.keys.resize(m_cldata.n_points);
        for (auto &slot : m_cldata.hashmap) {
            if (slot.key != 0xffffffff) {
                std::cout << core::str_format(
                    "gpu: count %12u, key %12u index %12u value %12u, "
                    "%10.6f %10.6f %10.6f\n",
                    count++,
                    slot.key,
                    slot.key % m_cldata.capacity,
                    slot.value,
                    m_cldata.points[slot.value].pos.s[0],
                    m_cldata.points[slot.value].pos.s[1],
                    m_cldata.points[slot.value].pos.s[2]);

                m_cldata.keys[slot.value] = std::make_pair(
                    slot.key, slot.key % m_cldata.capacity);
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
