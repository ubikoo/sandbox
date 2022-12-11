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

#include "atto/opencl/opencl.hpp"
#include "model.hpp"
#include <cfloat>
using namespace atto;

/** ---------------------------------------------------------------------------
 * Model::Model
 * @brief Create OpenCL context and associated objects.
 */
Model::Model()
{
    /*
     * Setup OpenGL data.
     */
    {
        /* Create the shader program object. */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/raymarch.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/raymarch.frag")};
        m_gl.program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.program) << "\n";

        /* Create the 2d-texture data store. */
        m_gl.texture = gl::create_texture2d(
            GL_RGBA8,                   /* texture internal format */
            Params::width,              /* texture width */
            Params::height,             /* texture height */
            GL_RGBA,                    /* pixel format */
            GL_UNSIGNED_BYTE,           /* pixel type */
            NULL);                      /* pixel data */
        glBindTexture(GL_TEXTURE_2D, m_gl.texture);
        gl::set_texture_mipmap(
            GL_TEXTURE_2D,
            GL_TRUE);                   /* generate mipmap */
        gl::set_texture_wrap(
            GL_TEXTURE_2D,
            GL_CLAMP_TO_EDGE,           /* wrap_s */
            GL_CLAMP_TO_EDGE);          /* wrap_t */
        gl::set_texture_filter(
            GL_TEXTURE_2D,
            GL_LINEAR,                  /* filter_min */
            GL_LINEAR);                 /* filter_mag */
        glBindTexture(GL_TEXTURE_2D, 0);

        /*
         * Create a mesh over a rectangle with screen size and set the mesh
         * vertex attributes in the program.
         */
        m_gl.mesh = gl::Mesh::Plane(
            m_gl.program,               /* shader program object */
            "quad",                     /* vertex attributes prefix name */
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
        /*
         * Setup OpenCL context with a command queue on the specified device.
         */
        // m_context = cl::Context::create(CL_DEVICE_TYPE_GPU);
        // m_device = cl::Context::get_device(m_context, Params::device_index);
        // m_queue = cl::Queue::create(m_context, m_device);

        /*
         * Setup OpenCL context based on the OpenGL context in the device.
         */
        std::vector<cl_device_id> devices = cl::Device::get_device_ids(CL_DEVICE_TYPE_GPU);
        core_assert(Params::device_index < devices.size(), "device index overflow");
        m_device = devices[Params::device_index];
        m_context = cl::Context::create_cl_gl_shared(m_device);
        m_queue = cl::Queue::create(m_context, m_device);
        std::cout << cl::Device::get_info_string(m_device) << "\n";

        /*
         * Create program object.
         */
        std::string source;
        source.append(cl::Program::load_source_from_file("data/base.cl"));
        source.append(cl::Program::load_source_from_file("data/raymarch.cl"));

        m_program = cl::Program::create_from_source(m_context, source);
        cl::Program::build(m_program, m_device, "");
        std::cout << cl::Program::get_source(m_program) << "\n";

        /*
         * Create engine kernels.
         */
        m_kernels.resize(NumKernels, NULL);
        m_kernels[KernelRaymarch] = cl::Kernel::create(m_program, "raymarch");

        /*
         * Create engine device buffer objects.
         */
        m_buffers.resize(NumBuffers, NULL);
        m_buffers[BufferSphere] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            sizeof(Sphere),
            (void *) NULL);

        /*
         * Create engine device image store from OpenGL texture object.
         */
        m_images.resize(NumImages, NULL);
        glBindTexture(GL_TEXTURE_2D, m_gl.texture);
        m_images[ImageRaymarch] = cl::gl::create_from_gl_texture(
            m_context,
            CL_MEM_WRITE_ONLY,
            GL_TEXTURE_2D,
            0,
            m_gl.texture);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    /*
     * Setup engine data.
     */
    {
        /*
         * Setup a sphere centred at the specified position and copy data.
         */
        m_sphere.centre = Params::sphere_centre;
        m_sphere.radius = Params::sphere_radius;

        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferSphere],
            sizeof(Sphere),
            (void *) &m_sphere);
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
        for (auto &it : m_images) {
            cl::Memory::release(it);
        }
        for (auto &it : m_buffers) {
            cl::Memory::release(it);
        }
        for (auto &it : m_kernels) {
            cl::Kernel::release(it);
        }
        cl::Program::release(m_program);
        cl::Queue::release(m_queue);
        cl::Device::release(m_device);
        cl::Context::release(m_context);
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

    /* Specify draw state modes. */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* Bind the shader program object. */
    glUseProgram(m_gl.program);

    /* Set the sampler uniform with the texture unit and bind the texture */
#if 0
    std::array<GLfloat,2> sizef = gl::Renderer::framebuffer_sizef();
    gl::set_uniform(m_gl.program, "u_width", GL_FLOAT, &sizef[0]);
    gl::set_uniform(m_gl.program, "u_height", GL_FLOAT, &sizef[1]);
#endif
    GLenum texunit = 0;
    gl::set_uniform(m_gl.program, "u_texsampler", GL_SAMPLER_2D, &texunit);
    gl::active_bind_texture(GL_TEXTURE_2D, GL_TEXTURE0 + texunit, m_gl.texture);

    /* Draw the mesh */
    m_gl.mesh->draw();

    /* Unbind the shader program object. */
    glUseProgram(0);
}

/** ---------------------------------------------------------------------------
 * Model::execute
 * @brief Execute the model.
 */
void Model::execute(void)
{
    /*
     * Execute KernelRaymarch
     */
    {
        /* Set kernel arguments. */
        const float current_time = glfwGetTime();
        cl::Kernel::set_arg(m_kernels[KernelRaymarch], 0, sizeof(cl_uint),  (void *) &Params::width);
        cl::Kernel::set_arg(m_kernels[KernelRaymarch], 1, sizeof(cl_uint),  (void *) &Params::height);
        cl::Kernel::set_arg(m_kernels[KernelRaymarch], 2, sizeof(cl_float), (void *) &Params::depth);
        cl::Kernel::set_arg(m_kernels[KernelRaymarch], 3, sizeof(cl_float), (void *) &current_time);
        cl::Kernel::set_arg(m_kernels[KernelRaymarch], 4, sizeof(cl_float), (void *) &Params::t_min);
        cl::Kernel::set_arg(m_kernels[KernelRaymarch], 5, sizeof(cl_float), (void *) &Params::t_max);
        cl::Kernel::set_arg(m_kernels[KernelRaymarch], 6, sizeof(cl_float), (void *) &Params::maxsteps);
        cl::Kernel::set_arg(m_kernels[KernelRaymarch], 7, sizeof(cl_mem),   (void *) &m_buffers[BufferSphere]);
        cl::Kernel::set_arg(m_kernels[KernelRaymarch], 8, sizeof(cl_mem),   (void *) &m_images[ImageRaymarch]);

        /* Set the size of the NDRange workgroups and run the kernel. */
        cl::NDRange local_ws(Params::work_group_size, Params::work_group_size);
        cl::NDRange global_ws(
            cl::NDRange::Roundup(Params::width,  local_ws(0)),
            cl::NDRange::Roundup(Params::height, local_ws(1)));

        /* Wait for OpenGL to finish and acquire the gl objects. */
        cl::gl::enqueue_acquire_gl_objects(m_queue, &m_images, NULL, NULL);

        /* Enqueue the OpenCL kernel for execution. */
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelRaymarch],
            cl::NDRange::Null,
            global_ws,
            local_ws,
            NULL,
            NULL);

        /* Wait for OpenCL to finish and release the gl objects. */
        cl::gl::enqueue_release_gl_objects(m_queue, &m_images, NULL, NULL);
    }
}
