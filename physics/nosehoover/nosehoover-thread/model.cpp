/*
 * model.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "model.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * Model::Model
 * @brief Create OpenCL context and associated objects.
 */
Model::Model(void)
{
    /*
     * Setup Model data.
     */
    {
        m_threadpool.create(Params::n_threads);
        m_nosehoover = NoseHoover::generate();
    }

    /*
     * Setup OpenGL data.
     */
    {
        /* Create the shader program object. */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/nosehoover.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/nosehoover.frag")};
        m_gl.program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.program) << "\n";

        /* Create the 2d-texture data store. */
        m_gl.image = std::make_unique<gl::Image>(Params::n_points, Params::n_points, 32);
        m_gl.texture = gl::create_texture2d(
            GL_RGBA8,                       /* texture internal format */
            m_gl.image->width(),            /* texture width */
            m_gl.image->height(),           /* texture height */
            m_gl.image->pixelformat(),      /* pixel format */
            GL_UNSIGNED_BYTE,               /* pixel type */
            m_gl.image->bitmap());          /* pixel data */
        glBindTexture(GL_TEXTURE_2D, m_gl.texture);
        gl::set_texture_wrap(
            GL_TEXTURE_2D,
            GL_CLAMP_TO_EDGE,               /* wrap_s */
            GL_CLAMP_TO_EDGE);              /* wrap_t */
        gl::set_texture_filter(
            GL_TEXTURE_2D,
            GL_NEAREST,                     /* filter_min */
            GL_NEAREST);                    /* filter_mag */
        glBindTexture(GL_TEXTURE_2D, 0);

        /*
         * Create a mesh over a rectangle with screen size and set the mesh
         * vertex attributes in the program.
         */
        m_gl.mesh = gl::Mesh::Plane(
            m_gl.program,               /* shader program object */
            "canvas",                   /* vertex attributes prefix name */
            2,                          /* n1 vertices */
            2,                          /* n2 vertices */
            -1.0,                       /* xlo */
            1.0,                        /* xhi */
            -1.0,                       /* ylo */
            1.0);                       /* yhi */
    }

    /*
     * Setup OpenCL data.
     */
    {
#if 0
        // /* Setup OpenCL context with a command queue on the specified device. */
        // m_context = cl::Context::create(CL_DEVICE_TYPE_GPU);
        // m_device = cl::Context::get_device(m_context, Params::device_index);
        // m_queue = cl::Queue::create(m_context, m_device);
#else
        // /* Setup OpenCL context based on the OpenGL context in the device. */
        // m_device = cl::Context::get_device(m_context, Params::device_index);
        // m_context = cl::Context::create_cl_gl_shared(m_device);
        // m_queue = cl::Queue::create(m_context, m_device);
#endif

        /* Create OpenCL program. */
        // std::string source;
        // source.append(cl::Program::load_source_from_file("data/empty.cl"));
        // m_program = cl::Program::create_from_source(m_context, source);
        // cl::Program::build(m_program, m_device, "");

        // m_program = cl::Program::create_from_file(m_context, "data/empty.cl");
        // cl::Program::build(m_program, m_device, "");

        /* Create OpenCL kernel. */
        // m_kernels.resize(NumKernels, NULL);
        // m_kernels[KernelEmpty] = cl::Kernel::create(m_program, "empty");
    }
}

/**
 * Model::~Model
 * @brief Destroy the OpenCL context and associated objects.
 */
Model::~Model()
{
    /* Teardown model data. */
    {}

    /* Teardown OpenCL data. */
    {
    //     for (auto &it : m_images) {
    //         cl::Memory::release(it);
    //     }
    //     for (auto &it : m_buffers) {
    //         cl::Memory::release(it);
    //     }
    //     for (auto &it : m_kernels) {
    //         cl::Kernel::release(it);
    //     }
    //     cl::Program::release(m_program);
    }
}

/** ---------------------------------------------------------------------------
 * Model::handle
 * @brief Handle the event.
 */
void Model::handle(const gl::Event &event)
{}

/** ---------------------------------------------------------------------------
 * Model::draw
 * @brief Render the drawable.
 */
void Model::draw(void *data)
{
    GLFWwindow *window = gl::Renderer::window();
    if (window == nullptr) {
        return;
    }

    /* Update the vertex buffer data. */
    {
        for (uint32_t x = 0; x < Params::n_points; ++x) {
            for (uint32_t y = 0; y < Params::n_points; ++y) {
                uint8_t *px = m_gl.image->pixel(x, y);
                *px++ = 0;
                *px++ = 0;
                *px++ = 0;
            }
        }

        uint32_t therm_ix = 0;
        for (uint32_t x = 0; x < Params::n_points; ++x) {
            for (uint32_t y = 0; y < Params::n_points; ++y) {

                math::vec3d pt = m_nosehoover[therm_ix++].x_state;
                if (2.0 * std::fabs(pt.x) < Params::canvas_width &&
                    2.0 * std::fabs(pt.y) < Params::canvas_height) {
                    double u = 0.5 + pt.x / Params::canvas_width;
                    double v = 0.5 + pt.y / Params::canvas_height;

                    uint32_t u_ix = static_cast<uint32_t>(u * Params::n_points);
                    uint32_t v_ix = static_cast<uint32_t>(v * Params::n_points);
                    uint8_t *px = m_gl.image->pixel(u_ix, v_ix);

                    float r = 255.0 * static_cast<float>(x) / static_cast<float>(Params::n_points);
                    float g = 255.0 * static_cast<float>(y) / static_cast<float>(Params::n_points);
                    float b = 0.0f;

                    *px++ = uint8_t(r);
                    *px++ = uint8_t(g);
                    *px++ = uint8_t(b);
                }
            }
        }
        m_gl.image->copy(m_gl.texture);
    }

    /* Draw the mesh. */
    {
        /* Specify draw state modes. */
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        /* Bind the shader program object. */
        glUseProgram(m_gl.program);

#if 0
        /* Set window dimensions. */
        std::array<GLfloat,2> size = gl::Renderer::framebuffer_sizef();
        gl::set_uniform(m_gl.program, "u_width", GL_FLOAT, &size[0]);
        gl::set_uniform(m_gl.program, "u_height", GL_FLOAT, &size[1]);
#endif

        /* Set the sampler uniform with the texture unit and bind the texture */
        GLenum texunit = 0;
        gl::set_uniform(m_gl.program, "u_texsampler", GL_SAMPLER_2D, &texunit);
        gl::active_bind_texture(GL_TEXTURE_2D, GL_TEXTURE0 + texunit, m_gl.texture);

        /* Draw the quad mesh */
        m_gl.mesh->draw();

        /* Unbind the shader program object. */
        glUseProgram(0);
   }
}

/** ---------------------------------------------------------------------------
 * Model::execute
 * @brief Execute the model.
 */
void Model::execute(void)
{
    for (size_t therm_ix = 0; therm_ix < m_nosehoover.size(); ++therm_ix) {
        ThreadWork work{this, therm_ix};
        m_threadpool.enqueue((void *) &work);
    }
    m_threadpool.wait();
}

/**
 * Model::integrate
 * @brief Integrate a single thermostat
 */
void Model::integrate(size_t therm_ix)
{
    for (size_t n = 0; n < Params::n_steps; ++n) {
        m_nosehoover[therm_ix].step(Params::t_step, Params::max_err, Params::max_iter);
    }
}
