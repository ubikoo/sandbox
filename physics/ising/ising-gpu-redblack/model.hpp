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

#ifndef ISING_MODEL_H_
#define ISING_MODEL_H_

#include <vector>
#include <random>
#include <algorithm>
#include "base.hpp"

struct Model : atto::gl::Drawable {
    /* ---- Model data ----------------------------------------------------- */
    enum {
        RED = 0,                /* Red enumerated color. */
        BLACK,                  /* Black enumerated color. */
    };
    cl_long m_step;             /* iteration counter */
    cl_double m_ising_J;        /* Ising energy coefficient */
    cl_double m_ising_h;        /* Ising external field coefficient */
    cl_double m_ising_beta;     /* Inverse reduced temperature */

    /* ---- Model OpenCL data ---------------------------------------------- */
    cl_context m_context = NULL;
    cl_device_id m_device = NULL;
    cl_command_queue m_queue = NULL;
    cl_program m_program = NULL;

    /* Model kernels. */
    enum {
        KernelRandomLattice = 0,
        KernelInitLattice,
        KernelFlipLattice,
        KernelImageLattice,
        NumKernels,
    };
    std::vector<cl_kernel> m_kernels;

    /* Model device buffer objects. */
    enum {
        BufferRandom = 0,
        BufferLattice,
        NumBuffers
    };
    std::vector<cl_mem> m_buffers;

    /* Model device image objects. */
    enum {
        ImageLattice = 0,
        NumImages,
    };
    std::vector<cl_mem> m_images;

    /* ---- Model OpenGL data ---------------------------------------------- */
    struct GLData {
        GLuint program;                         /* shader program object */
        std::unique_ptr<atto::gl::Mesh> mesh;   /* lattice quad mesh */
        std::unique_ptr<atto::gl::Image> image; /* lattice image */
        GLuint texture;                         /* lattice texture */
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

#endif /* ISING_MODEL_H_ */
