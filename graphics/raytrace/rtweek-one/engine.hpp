/*
 * engine.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef RAYTRACE_CPU_WEEK_1_ENGINE_H_
#define RAYTRACE_CPU_WEEK_1_ENGINE_H_

#include <memory>
#include <vector>
#include <random>

#include "base.hpp"
#include "color.hpp"
#include "camera.hpp"
#include "film.hpp"
#include "sample.hpp"
#include "ray.hpp"
#include "isect.hpp"
#include "material.hpp"
#include "primitive.hpp"
#include "interaction.hpp"

/**
 * Engine
 * Raytracing render engine.
 */
struct Engine : atto::gl::Drawable {
    /* ---- Engine data ----------------------------------------------------- */
    size_t m_sample_count;
    std::unique_ptr<Sample> m_sample;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Film> m_film;
    std::vector<Primitive> m_world;

    /* ---- Engine OpenCL data ---------------------------------------------- */
    // cl_context m_context = NULL;
    // cl_device_id m_device = NULL;
    // cl_command_queue m_queue = NULL;
    // cl_program m_program = NULL;

    // /* Engine kernels. */
    // enum {
    //     NumKernels = 0
    // };
    // std::vector<cl_kernel> m_kernels;

    // /* Engine device buffer objects. */
    // enum {
    //     NumBuffers = 0
    // };
    // std::vector<cl_mem> m_buffers;

    // /* Engine device image objects. */
    // enum {
    //     NumImages = 0
    // };
    // std::vector<cl_mem> m_images;

    /* ---- Engine OpenGL data ---------------------------------------------- */
    struct GLData {
        GLuint program;                         /* shader program object */
        std::unique_ptr<atto::gl::Mesh> mesh;   /* quad mesh */
        GLuint texture;
        std::vector<uint8_t> bitmap;            /* film bitmap */
    } m_gl;

    /* ---- Engine member functions ----------------------------------------- */
    void handle(const atto::gl::Event &event) override;
    void draw(void *data = nullptr) override;
    void execute(void);

    /** @brief Return the radiance along the primary ray. */
    Color radiance(Ray &ray);

    /** @brief Generate a random scene. */
    std::vector<Primitive> generate(int32_t n_cells);

    /* Constructor/destructor. */
    Engine();
    ~Engine();
    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;
};

#endif /* RAYTRACE_CPU_WEEK_1_ENGINE_H_ */
