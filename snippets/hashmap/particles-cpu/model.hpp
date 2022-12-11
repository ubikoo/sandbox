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
#include "camera.hpp"

struct Model : atto::gl::Drawable {
    /* ---- Model data ---------------------------------------------- */
    struct Point {
        cl_float3 pos;
        cl_float3 col;
    };

    struct KeyValue {
        cl_uint key;
        cl_uint value;
    };

    struct Data {
        cl_uint n_points;
        cl_uint n_cells;
        cl_float3 domain_lo;
        cl_float3 domain_hi;
        std::vector<Point> points;
        std::unique_ptr<Hashmap> hashmap;
        atto::math::rng::Kiss random;
    } m_data;

    /* ---- Model OpenCL data ---------------------------------------------- */
    // cl_context m_context = NULL;
    // cl_device_id m_device = NULL;
    // cl_command_queue m_queue = NULL;
    // cl_program m_program = NULL;
    // enum {
    //     NumKernels = 0,
    // };
    // std::vector<cl_kernel> m_kernels;
    // enum {
    //     NumBuffers = 0,
    // };
    // std::vector<cl_mem> m_buffers;
    // enum {
    //     NumImages = 0
    // };
    // std::vector<cl_mem> m_images;

    /* ---- Model OpenGL data ---------------------------------------------- */
    struct GLData {
        /* point data */
        GLfloat point_scale = 0.02f;
        GLsizei n_points;
        std::vector<GLfloat> point_vertex;
        GLuint point_vbo;

        /* sprite data */
        std::vector<GLfloat> sprite_vertex;
        std::vector<GLuint> sprite_index;
        GLuint sprite_vbo;
        GLuint sprite_ebo;

        /* shader program */
        GLuint program;
        GLuint vao;
    } m_gl;
    Camera m_camera;

    /* ---- Model member functions ----------------------------------------- */
    void handle(const atto::gl::Event &event) override;
    void draw(void *data = nullptr) override;
    void execute(void);

    Model();
    ~Model();
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
};

#endif /* MODEL_H_ */
