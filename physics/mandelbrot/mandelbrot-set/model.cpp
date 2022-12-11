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
Model::Model()
{
    /*
     * Setup Model data.
     */
    {}

    /*
     * Setup OpenGL data.
     */
    {
        /* Create the shader program object. */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/mandelbrot.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/mandelbrot.frag")};
        m_gl.program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.program) << "\n";

        /* Create the 2d-texture data store. */
        m_gl.texture = gl::create_texture2d(
            GL_RGBA8,                   /* texture internal format */
            Params::width,             /* texture width */
            Params::height,            /* texture height */
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
            m_gl.program,           /* shader program object */
            "quad",                 /* vertex attributes prefix name */
            2,                      /* n1 vertices */
            2,                      /* n2 vertices */
            -1.0,                   /* xlo */
            1.0,                    /* xhi */
            -1.0,                   /* ylo */
            1.0);                   /* yhi */
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
        // std::cout << cl::Device::get_info_string(m_device) << "\n";

        /*
         * Create OpenCL program.
         */
        m_program = cl::Program::create_from_file(m_context, "data/mandelbrot.cl");
        std::cout << cl::Program::get_source(m_program) << "\n";
        cl::Program::build(m_program, m_device, "");

        /*
         * Create OpenCL kernel.
         */
        m_kernels.resize(NumKernels, NULL);
        m_kernels[KernelMandelbrot] = cl::Kernel::create(m_program, "mandelbrot");

        /*
         * Create OpenCL image store from the OpenGL texture object.
         */
        m_images.resize(NumImages, NULL);
        glBindTexture(GL_TEXTURE_2D, m_gl.texture);
        m_images[ImageMandelbrot] = cl::gl::create_from_gl_texture(
            m_context,
            CL_MEM_WRITE_ONLY,
            GL_TEXTURE_2D,
            0,
            m_gl.texture);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

/**
 * Model::~Model
 * @brief Destroy the OpenCL context and associated objects.
 */
Model::~Model()
{
    /* Teardown model data. */
    {
        /* empty */
    }

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
 * Handle an event.
 */
void Model::handle(const gl::Event &event)
{
    /* Set shift key zoom control. */
    if (event.type == gl::Event::Key &&
        event.key.code == GLFW_KEY_LEFT_SHIFT) {
        m_gl.shift_key_pressed = event.key.action == GLFW_PRESS ? true : false;
    }

    /* Set the left button state */
    if (event.type == gl::Event::MouseButton) {
        m_gl.centre_end = m_gl.centre_beg;

        if (event.mousebutton.button == GLFW_MOUSE_BUTTON_LEFT &&
            event.mousebutton.action == GLFW_PRESS) {
            m_gl.left_button_pressed = true;
        } else {
            m_gl.left_button_pressed = false;
        }

        if (event.mousebutton.button == GLFW_MOUSE_BUTTON_RIGHT &&
            event.mousebutton.action == GLFW_PRESS) {
            m_gl.right_button_pressed = true;
        } else {
            m_gl.right_button_pressed = false;
        }
    }

    /* Store the current cursor position */
    if (event.type == gl::Event::CursorPos) {
        if (m_gl.left_button_pressed || m_gl.right_button_pressed) {
            auto window_size = gl::Renderer::framebuffer_sizef();
            m_gl.centre_end = {
                (cl_float) event.cursorpos.xpos / window_size[0],
                1.0f - (cl_float) event.cursorpos.ypos / window_size[1]
            };
            // m_gl.centre_end.s[0] = std::min(std::max(m_gl.centre_end.s[0], 0.0f), 1.0f);
            // m_gl.centre_end.s[1] = std::min(std::max(m_gl.centre_end.s[1], 0.0f), 1.0f);
        }
    }

    /* Set the t step factor for the current cursor position */
    if (event.type == gl::Event::MouseScroll) {
        m_gl.domain_step *= event.mousescroll.yoffset > 0.0 ? 2.0 : 0.5;
        std::cout << "m_gl.domain_step " << m_gl.domain_step << "\n";
    }
}

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
    /* Compute Mandelbrot range */
    cl_float2 domain_size = {
        Params::xrange.s[1] - Params::xrange.s[0],
        Params::yrange.s[1] - Params::yrange.s[0]
    };

    cl_float2 domain_centre = {
        Params::xrange.s[0] + m_gl.centre_beg.s[0] * domain_size.s[0],
        Params::yrange.s[0] + m_gl.centre_beg.s[1] * domain_size.s[1]
    };

    cl_float2 domain_xrange = {
        domain_centre.s[0] - 0.5f * m_gl.domain_scale * domain_size.s[0],
        domain_centre.s[0] + 0.5f * m_gl.domain_scale * domain_size.s[0]
    };

    cl_float2 domain_yrange = {
        domain_centre.s[1] - 0.5f * m_gl.domain_scale * domain_size.s[1],
        domain_centre.s[1] + 0.5f * m_gl.domain_scale * domain_size.s[1]
    };

    /* Update Mandelbrot centre position */
    {
        cl_float2 dr = {
            m_gl.domain_scale * domain_size.s[0],
            m_gl.domain_scale * domain_size.s[1]
        };
        m_gl.domain_step = m_gl.domain_step_factor *
            std::sqrt(dr.s[0]*dr.s[0] + dr.s[1]*dr.s[1]);
    }

    if (m_gl.left_button_pressed || m_gl.right_button_pressed) {
        cl_float2 dr = {
            m_gl.centre_end.s[0] - m_gl.centre_beg.s[0],
            m_gl.centre_end.s[1] - m_gl.centre_beg.s[1]
        };

        m_gl.centre_beg.s[0] += m_gl.domain_step * dr.s[0];
        m_gl.centre_beg.s[1] += m_gl.domain_step * dr.s[1];

        if (m_gl.shift_key_pressed) {
            m_gl.domain_scale *= m_gl.left_button_pressed
                ? m_gl.domain_scale_factor
                : 1.0 / m_gl.domain_scale_factor;
        }
    }

    /* Set kernel arguments. */
    cl::Kernel::set_arg(m_kernels[KernelMandelbrot], 0, sizeof(cl_mem),    &m_images[ImageMandelbrot]);
    cl::Kernel::set_arg(m_kernels[KernelMandelbrot], 1, sizeof(cl_float2), &domain_xrange);
    cl::Kernel::set_arg(m_kernels[KernelMandelbrot], 2, sizeof(cl_float2), &domain_yrange);
    cl::Kernel::set_arg(m_kernels[KernelMandelbrot], 3, sizeof(cl_long),   &Params::width);
    cl::Kernel::set_arg(m_kernels[KernelMandelbrot], 4, sizeof(cl_long),   &Params::height);
    cl::Kernel::set_arg(m_kernels[KernelMandelbrot], 5, sizeof(cl_long),   &Params::maxiters);

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
        m_kernels[KernelMandelbrot],
        cl::NDRange::Null,
        global_ws,
        local_ws,
        NULL,
        NULL);

    /* Wait for OpenCL to finish and release the gl objects. */
    cl::gl::enqueue_release_gl_objects(m_queue, &m_images, NULL, NULL);
}
