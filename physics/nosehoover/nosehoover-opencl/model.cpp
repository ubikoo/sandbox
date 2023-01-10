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
 * @brief Create OpenCL context and associated objects.
 */
Model::Model(void)
{
    /*
     * Setup Model data.
     */
    {
        /* Specify thermostat parameters */
        m_nosehoover_param = NoseHooverParam{
            Params::mass,
            Params::kappa,
            Params::tau,
            Params::temperature};

        /* Generate a collection of Nose-Hoover thermostats. */
        cl_double x_delta = Params::init_x_range / static_cast<cl_double>(Params::canvas_width);
        cl_double y_delta = Params::init_y_range / static_cast<cl_double>(Params::canvas_height);

        cl_double x_offset = -0.5 * Params::init_x_range;
        cl_double y_offset = -0.5 * Params::init_y_range;

        for (cl_uint y = 0; y < Params::canvas_height; ++y) {
            for (cl_uint x = 0; x < Params::canvas_width; ++x) {
                cl_double pos = x_offset + x * x_delta;  /* position */
                cl_double mom = y_offset + y * y_delta;  /* momentum */
                cl_float4 color = (cl_float4) {
                    static_cast<float>(x) / static_cast<float>(Params::canvas_width),
                    static_cast<float>(y) / static_cast<float>(Params::canvas_height),
                    0.0f,
                    1.0f};
                NoseHoover item{pos, mom, 0.0, color};
                m_nosehoover.push_back(item);
            }
        }
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
        m_gl.texture = gl::create_texture2d(
            GL_RGBA8,                       /* texture internal format */
            Params::canvas_width,           /* texture width */
            Params::canvas_height,          /* texture height */
            GL_RGBA,                        /* pixel format */
            GL_UNSIGNED_BYTE,               /* pixel type */
            nullptr);                       /* pixel data */
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
            m_gl.program,                   /* shader program object */
            "canvas",                       /* vertex attributes prefix name */
            2,                              /* n1 vertices */
            2,                              /* n2 vertices */
            -1.0,                           /* xlo */
             1.0,                           /* xhi */
            -1.0,                           /* ylo */
             1.0);                          /* yhi */
    }

    /*
     * Setup OpenCL data.
     */
    {
        /* Setup OpenCL context based on the OpenGL context in the device. */
        std::vector<cl_device_id> devices = cl::Device::get_device_ids(CL_DEVICE_TYPE_GPU);
        core_assert(Params::device_index < devices.size(), "device index overflow");
        m_device = devices[Params::device_index];
        m_context = cl::Context::create_cl_gl_shared(m_device);
        m_queue = cl::Queue::create(m_context, m_device);

        /* Create OpenCL program. */
        std::string source;
        source.append(cl::Program::load_source_from_file("data/base.cl"));
        source.append(cl::Program::load_source_from_file("data/nosehoover.cl"));

        m_program = cl::Program::create_from_source(m_context, source);
        cl::Program::build(m_program, m_device, "");
        std::cout << cl::Program::get_source(m_program) << "\n";

        /* Create OpenCL kernel. */
        m_kernels.resize(NumKernels, NULL);
        m_kernels[KernelIntegrate] = cl::Kernel::create(m_program, "integrate");
        m_kernels[KernelResetCanvas] = cl::Kernel::create(m_program, "reset_canvas");
        m_kernels[KernelDepthCanvas] = cl::Kernel::create(m_program, "depth_canvas");
        m_kernels[KernelDrawCanvas] = cl::Kernel::create(m_program, "draw_canvas");

        /* Create OpenCL buffers. */
        m_buffers.resize(NumBuffers, NULL);

        m_buffers[BufferNoseHooverParam] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_ONLY,
            sizeof(NoseHooverParam),
            (void *) NULL);

        m_buffers[BufferNoseHoover] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            Params::n_thermostats * sizeof(NoseHoover),
            (void *) NULL);

        m_buffers[BufferCanvas] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            Params::n_thermostats * sizeof(cl_uint),
            (void *) NULL);

        /* Create OpenCL image store from OpenGL texture object. */
        m_images.resize(NumImages, NULL);

        glBindTexture(GL_TEXTURE_2D, m_gl.texture);
        m_images[ImageCanvas] = cl::gl::create_from_gl_texture(
            m_context,
            CL_MEM_WRITE_ONLY,
            GL_TEXTURE_2D,
            0,
            m_gl.texture);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    /*
     * Copy OpenCL data.
     */
    {
        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferNoseHooverParam],
            sizeof(NoseHooverParam),
            (void *) &m_nosehoover_param);

        cl::Queue::enqueue_copy_to(
            m_queue,
            m_buffers[BufferNoseHoover],
            Params::n_thermostats * sizeof(NoseHoover),
            (void *) &m_nosehoover[0]);
    }
}

/**
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
    }
}

/** ---------------------------------------------------------------------------
 * @brief Handle the event.
 */
void Model::handle(const gl::Event &event)
{}

/** ---------------------------------------------------------------------------
 * @brief Render the drawable.
 */
void Model::draw(void *data)
{
    GLFWwindow *window = gl::Renderer::window();
    if (window == nullptr) {
        return;
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
 * @brief Execute the model.
 */
void Model::execute(void)
{
    /* Integrate the Nose-Hoover thermostats. */
    {
        cl::Kernel::set_arg(m_kernels[KernelIntegrate], 0, sizeof(cl_double), (void *) &Params::t_step);
        cl::Kernel::set_arg(m_kernels[KernelIntegrate], 1, sizeof(cl_double), (void *) &Params::max_err);
        cl::Kernel::set_arg(m_kernels[KernelIntegrate], 2, sizeof(cl_uint), (void *) &Params::max_iter);
        cl::Kernel::set_arg(m_kernels[KernelIntegrate], 3, sizeof(cl_uint), (void *) &Params::n_thermostats);
        cl::Kernel::set_arg(m_kernels[KernelIntegrate], 4, sizeof(cl_mem), &m_buffers[BufferNoseHooverParam]);
        cl::Kernel::set_arg(m_kernels[KernelIntegrate], 5, sizeof(cl_mem), &m_buffers[BufferNoseHoover]);

        static cl::NDRange global_ws(
            cl::NDRange::Roundup(Params::n_thermostats, Params::work_group_size_1d));
        static cl::NDRange local_ws(Params::work_group_size_1d);

        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelIntegrate],
            cl::NDRange::Null,
            global_ws,
            local_ws);

        cl::Queue::enqueue_copy_from(
            m_queue,
            m_buffers[BufferNoseHoover],
            Params::n_thermostats * sizeof(NoseHoover),
            (void *) &m_nosehoover[0]);
    }

    /* Reset the canvas buffer. */
    {
        cl::Kernel::set_arg(m_kernels[KernelResetCanvas], 0, sizeof(cl_uint), (void *) &Params::canvas_width);
        cl::Kernel::set_arg(m_kernels[KernelResetCanvas], 1, sizeof(cl_uint), (void *) &Params::canvas_height);
        cl::Kernel::set_arg(m_kernels[KernelResetCanvas], 2, sizeof(cl_mem), &m_buffers[BufferCanvas]);

        static cl::NDRange global_ws(
            cl::NDRange::Roundup(Params::canvas_width,  Params::work_group_size_2d),
            cl::NDRange::Roundup(Params::canvas_height, Params::work_group_size_2d));
        static cl::NDRange local_ws(
            Params::work_group_size_2d,
            Params::work_group_size_2d);

        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelResetCanvas],
            cl::NDRange::Null,
            global_ws,
            local_ws);
    }

    /* Compute the thermostats depth test. */
    {
        cl::Kernel::set_arg(m_kernels[KernelDepthCanvas], 0, sizeof(cl_uint), (void *) &Params::n_thermostats);
        cl::Kernel::set_arg(m_kernels[KernelDepthCanvas], 1, sizeof(cl_uint), (void *) &Params::canvas_width);
        cl::Kernel::set_arg(m_kernels[KernelDepthCanvas], 2, sizeof(cl_uint), (void *) &Params::canvas_height);
        cl::Kernel::set_arg(m_kernels[KernelDepthCanvas], 3, sizeof(cl_double), (void *) &Params::canvas_x_range);
        cl::Kernel::set_arg(m_kernels[KernelDepthCanvas], 4, sizeof(cl_double), (void *) &Params::canvas_y_range);
        cl::Kernel::set_arg(m_kernels[KernelDepthCanvas], 5, sizeof(cl_mem), &m_buffers[BufferNoseHoover]);
        cl::Kernel::set_arg(m_kernels[KernelDepthCanvas], 6, sizeof(cl_mem), &m_buffers[BufferCanvas]);

        static cl::NDRange global_ws(
            cl::NDRange::Roundup(Params::n_thermostats, Params::work_group_size_1d));
        static cl::NDRange local_ws(Params::work_group_size_1d);

        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelDepthCanvas],
            cl::NDRange::Null,
            global_ws,
            local_ws);
    }

    /* Use the canvas indices to draw. */
    {
        cl::Kernel::set_arg(m_kernels[KernelDrawCanvas], 0, sizeof(cl_uint), (void *) &Params::canvas_width);
        cl::Kernel::set_arg(m_kernels[KernelDrawCanvas], 1, sizeof(cl_uint), (void *) &Params::canvas_height);
        cl::Kernel::set_arg(m_kernels[KernelDrawCanvas], 2, sizeof(cl_mem), &m_images[ImageCanvas]);
        cl::Kernel::set_arg(m_kernels[KernelDrawCanvas], 3, sizeof(cl_mem), &m_buffers[BufferNoseHoover]);
        cl::Kernel::set_arg(m_kernels[KernelDrawCanvas], 4, sizeof(cl_mem), &m_buffers[BufferCanvas]);

        static cl::NDRange global_ws(
            cl::NDRange::Roundup(Params::canvas_width,  Params::work_group_size_2d),
            cl::NDRange::Roundup(Params::canvas_height, Params::work_group_size_2d));
        static cl::NDRange local_ws(
            Params::work_group_size_2d,
            Params::work_group_size_2d);

        /* Wait for OpenGL to finish and acquire the gl objects. */
        cl::gl::enqueue_acquire_gl_objects(m_queue, 1, &m_images[ImageCanvas]);

        /* Enqueue the OpenCL kernel for execution. */
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelDrawCanvas],
            cl::NDRange::Null,
            global_ws,
            local_ws);

        /* Wait for OpenCL to finish and release the gl objects. */
        cl::gl::enqueue_release_gl_objects(m_queue, 1, &m_images[ImageCanvas]);
    }
}
