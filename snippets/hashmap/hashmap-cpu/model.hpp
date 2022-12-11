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
    struct Data {
        uint32_t n_points;
        uint32_t n_cells;
        atto::math::vec3f domain_lo;
        atto::math::vec3f domain_hi;
        std::vector<atto::math::vec3f> points;
        std::unique_ptr<Hashmap> hashmap;
    } m_data;

    /* OpenCL/OpenGL data. */
    // cl_context m_context = NULL;
    // cl_device_id m_device = NULL;
    // cl_command_queue m_queue = NULL;
    // cl_program m_program = NULL;
    // cl_kernel m_kernel = NULL;
    // std::vector<cl_mem> m_buffers;
    // std::vector<cl_mem> m_images;

    /* Model member functions. */
    void execute(void);
    void handle(const atto::gl::Event &event);

    Model();
    ~Model();
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
};

#endif /* MODEL_H_ */
