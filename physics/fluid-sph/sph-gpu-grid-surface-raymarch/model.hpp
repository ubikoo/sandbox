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
#include "iodepth.hpp"
#include "iobuffer.hpp"

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
        GLuint point_vbo;
        GLfloat point_scale;

        /* Sprite Data */
        std::vector<GLfloat> sprite_vertex;
        std::vector<GLuint> sprite_index;
        GLuint sprite_vbo;
        GLuint sprite_ebo;

        /* Render framebuffers */
        std::array<GLfloat,2> fbosize;
        std::unique_ptr<IODepth> iodepth;
        std::array<std::unique_ptr<IOBuffer>, 2> iobuffer;
        size_t read_ix;
        size_t write_ix;

        /* Draw sprite shader */
        GLuint replace_depth;
        GLuint draw_sprite_program;
        GLuint draw_sprite_vao;

        /* Draw compute linear depth shader */
        GLuint depth_compute_program;
        std::unique_ptr<atto::gl::Mesh> depth_compute_quad;

        /* Draw curvature flow smooth shader */
        GLuint compute_normal;
        GLuint depth_smooth_program;
        std::unique_ptr<atto::gl::Mesh> depth_smooth_quad;

        /* Draw surface shader */
        GLuint draw_surface_program;
        std::unique_ptr<atto::gl::Mesh> draw_surface_quad;
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