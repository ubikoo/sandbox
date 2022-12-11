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
#include "base.hpp"
#include "generate.hpp"

struct Model : atto::gl::Drawable {
    /* ---- Model data ----------------------------------------------------- */
    Sphere m_sphere;

    /* ---- Model OpenCL data ---------------------------------------------- */
    cl_context m_context = NULL;
    cl_device_id m_device = NULL;
    cl_command_queue m_queue = NULL;
    cl_program m_program = NULL;

    /* Model kernels. */
    enum {
        KernelRaymarch = 0,
        NumKernels,
    };
    std::vector<cl_kernel> m_kernels;

    /* Model device buffer objects. */
    enum {
        BufferSphere = 0,
        NumBuffers,
    };
    std::vector<cl_mem> m_buffers;

    /* Model device image objects. */
    enum {
        ImageRaymarch = 0,
        NumImages
    };
    std::vector<cl_mem> m_images;

    /* ---- Model OpenGL data ---------------------------------------------- */
    struct GLData {
        GLuint program;                             /* shader program object */
        std::unique_ptr<atto::gl::Mesh> mesh;       /* texture mesh */
        GLuint texture;                             /* texture data */
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

#endif /* SPH_MODEL_H_ */
