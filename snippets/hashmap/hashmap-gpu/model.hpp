/*
 * model.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <vector>
#include "base.hpp"
#include "point.hpp"
#include "hashmap.hpp"

struct Model {
    /* ---- Model data ----------------------------------------------------- */
    struct Point {
        cl_float3 pos;
        cl_float3 col;
    };

    struct KeyValue {
        cl_uint key;
        cl_uint value;
    };

    struct Data {
        uint32_t n_points;
        uint32_t n_cells;
        atto::math::vec3f domain_lo;
        atto::math::vec3f domain_hi;
        std::vector<atto::math::vec3f> points;
        std::unique_ptr<Hashmap> hashmap;
        std::vector<std::pair<uint32_t, uint32_t>> keys;
    } m_data;

    struct CLData {
        cl_uint n_points;
        cl_uint n_cells;
        cl_uint capacity;
        cl_float3 domain_lo;
        cl_float3 domain_hi;
        std::vector<Point> points;
        std::vector<KeyValue> hashmap;
        std::vector<std::pair<uint32_t, uint32_t>> keys;
    } m_cldata;

    /* ---- Model OpenCL data ---------------------------------------------- */
    enum {
        BufferHashmap = 0,
        BufferPoints,
        NumBuffers
    };
    cl_context m_context = NULL;
    cl_device_id m_device = NULL;
    cl_command_queue m_queue = NULL;
    cl_program m_program = NULL;
    cl_kernel m_kernel_hashmap_insert = NULL;
    std::vector<cl_mem> m_buffers;
    std::vector<cl_mem> m_images;

    /* Model member functions. */
    void execute(void);
    void execute_cpu(void);
    void execute_gpu(void);
    void handle(const atto::gl::Event &event);

    Model();
    ~Model();
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
};

#endif /* MODEL_H_ */
