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

#include <memory>
#include <vector>
#include "base.hpp"

/**
 * Model
 * Mandelbrot model.
 */
struct Model : atto::gl::Drawable {
    /* ---- Model data ----------------------------------------------------- */

    /* ---- Model OpenCL data ---------------------------------------------- */
    cl_context m_context = NULL;
    cl_device_id m_device = NULL;
    cl_command_queue m_queue = NULL;
    cl_program m_program = NULL;

    /* Model kernels. */
    enum {
        KernelMandelbrot = 0,
        NumKernels
    };
    std::vector<cl_kernel> m_kernels;

    /* Model device buffer objects. */
    enum {
        NumBuffers = 0
    };
    std::vector<cl_mem> m_buffers;

    /* Model device image objects. */
    enum {
        ImageMandelbrot = 0,
        NumImages
    };
    std::vector<cl_mem> m_images;

    /* ---- Model OpenGL data ---------------------------------------------- */
    struct GLData {
        GLuint program;                             /* shader program object */
        std::unique_ptr<atto::gl::Mesh> mesh;       /* texture mesh */
        GLuint texture;                             /* texture data */
        bool shift_key_pressed = false;
        bool left_button_pressed = false;
        bool right_button_pressed = false;
        cl_float2 centre_beg = {0.5f, 0.5f};
        cl_float2 centre_end = {0.5f, 0.5f};
        const cl_float domain_step_factor = 0.02f;
        cl_float domain_step = 0.0f;
        const cl_float domain_scale_factor = 0.95f;
        cl_float domain_scale = 1.0f;
    } m_gl;

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
