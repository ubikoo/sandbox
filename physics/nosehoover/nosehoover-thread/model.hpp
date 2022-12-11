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
#include "nosehoover.hpp"
#include "threadpool.hpp"

struct Model : atto::gl::Drawable {
    /* ---- Model data ----------------------------------------------------- */
    ThreadPool m_threadpool;
    std::vector<NoseHoover> m_nosehoover;

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
        GLuint program;                             /* shader program object */
        GLuint texture;                             /* texture data */
        std::unique_ptr<atto::gl::Image> image;     /* texture image */
        std::unique_ptr<atto::gl::Mesh> mesh;       /* texture mesh */
    } m_gl;

    /* ---- Model member functions ----------------------------------------- */
    void handle(const atto::gl::Event &event) override;
    void draw(void *data = nullptr) override;
    void execute(void);

    void integrate(size_t therm_ix);

    Model();
    ~Model();
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
};

#endif /* MODEL_H_ */
