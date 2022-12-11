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

#ifndef SPH_MODEL_H_
#define SPH_MODEL_H_

#include <vector>
#include "atto/opencl/opencl.hpp"
#include "base.hpp"
#include "engine.hpp"
#include "camera.hpp"

struct Model : atto::gl::Drawable {
    /* ---- Model data ----------------------------------------------------- */
    Engine m_engine;

    /* ---- Model OpenCL data ---------------------------------------------- */
    cl_context m_context = NULL;
    cl_device_id m_device = NULL;
    cl_command_queue m_queue = NULL;
    cl_program m_program = NULL;

    /* Model kernels. */
    enum {
        NumKernels = 0
    };
    std::vector<cl_kernel> m_kernels;

    /* Model device buffer objects. */
    enum {
        NumBuffers = 0
    };
    std::vector<cl_mem> m_buffers;

    /* Model device image objects. */
    enum {
        NumImages = 0
    };
    std::vector<cl_mem> m_images;

    /* ---- Model OpenGL data ---------------------------------------------- */
    struct GLData {
        /* Camera */
        Camera camera;

        /* Point Data */
        GLsizei n_points;
        GLfloat point_scale = 0.5f;
        // std::vector<GLfloat> point_vertex;
        GLuint point_vbo;

        /* Sprite Data */
        std::vector<GLfloat> sprite_vertex;
        std::vector<GLuint> sprite_index;
        GLuint sprite_vbo;
        GLuint sprite_ebo;

        /* Shader program */
        GLuint program;
        GLuint vao;
    } m_gl;

    /* ---- Model member functions ----------------------------------------- */
    void handle(const atto::gl::Event &event) override;
    void draw(void *data = nullptr) override;
    bool execute(void);

    Model();
    ~Model();
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
};

#endif /* SPH_MODEL_H_ */
