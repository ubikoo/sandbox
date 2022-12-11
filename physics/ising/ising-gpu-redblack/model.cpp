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
    {
        /* Model parameters */
        m_step = 0;
        m_ising_J = Params::ising_J;
        m_ising_h = Params::ising_h;
        m_ising_beta = Params::ising_beta;
    }

    /*
     * Setup OpenGL data.
     */
    {
        /*
         * Create the shader program object.
         */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/ising.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/ising.frag")};
        m_gl.program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.program) << "\n";

        /*
         * Set the 2d-image bitmap and corresponding texture object.
         */
        m_gl.image = std::make_unique<gl::Image>(
            Params::image_width, Params::image_height, Params::image_bpp);
        std::cout << m_gl.image->infolog("Image attributes:") << "\n";

        m_gl.texture = gl::create_texture2d(
            GL_RGBA8,                       /* texture internal format */
            m_gl.image->width(),            /* texture width */
            m_gl.image->height(),           /* texture height */
            m_gl.image->pixelformat(),      /* pixel format */
            GL_UNSIGNED_BYTE,               /* pixel type */
            m_gl.image->bitmap());          /* pixel data */
        glBindTexture(GL_TEXTURE_2D, m_gl.texture);
        gl::set_texture_filter(
            GL_TEXTURE_2D,
            GL_LINEAR,                      /* filter_min */
            GL_LINEAR);                     /* filter_mag */
        glBindTexture(GL_TEXTURE_2D, 0);

        /*
         * Create a mesh over a rectangle with screen size and set the mesh
         * vertex attributes in the program.
         */
        GLfloat aspect =
            (GLfloat) m_gl.image->width() / m_gl.image->height();
        GLfloat xrange = aspect > 1.0 ? 1.0 : aspect;
        GLfloat yrange = aspect > 1.0 ? 1.0 / aspect : 1.0;
        m_gl.mesh = gl::Mesh::Plane(
            m_gl.program,                   /* shader program object */
            "lattice",                         /* vertex attributes prefix name */
            2,                              /* n1 vertices */
            2,                              /* n2 vertices */
            -xrange,                        /* xlo */
            xrange,                         /* xhi */
            -yrange,                        /* ylo */
            yrange);                        /* yhi */
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
        std::string source;
        source.append(cl::Program::load_source_from_file("data/common.cl"));
        source.append(cl::Program::load_source_from_file("data/ising.cl"));
        m_program = cl::Program::create_from_source(m_context, source);
        std::cout << cl::Program::get_source(m_program) << "\n";
        cl::Program::build(m_program, m_device, "");

        /*
         * Create OpenCL kernels.
         */
        m_kernels.resize(NumKernels, NULL);
        m_kernels[KernelRandomLattice] = cl::Kernel::create(m_program, "random_lattice");
        m_kernels[KernelInitLattice] = cl::Kernel::create(m_program, "init_lattice");
        m_kernels[KernelFlipLattice] = cl::Kernel::create(m_program, "flip_lattice");
        m_kernels[KernelImageLattice] = cl::Kernel::create(m_program, "image_lattice");

        /*
         * Create OpenCL device buffer objects.
         */
        std::random_device seed;    /* rng device */
        std::mt19937 rng(seed());   /* rng engine */
        std::uniform_int_distribution<uint32_t> dist;
        std::vector<cl_uint> random_data(Params::n_sites * Params::n_sites);
        std::generate(
            random_data.begin(),
            random_data.end(),
            [&] () { return dist(rng); });

        m_buffers.resize(NumBuffers, NULL);
        m_buffers[BufferRandom] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            Params::n_sites * Params::n_sites * sizeof(cl_uint),
            (void *) random_data.data());
        m_buffers[BufferLattice] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            Params::n_sites * Params::n_sites * sizeof(cl_int),
            (void *) NULL);

        /*
         * Create OpenCL image store from OpenGL texture object.
         */
        m_images.resize(NumImages, NULL);
        glBindTexture(GL_TEXTURE_2D, m_gl.texture);
        m_images[ImageLattice] = cl::gl::create_from_gl_texture(
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
 * @brief Handle an event.
 */
void Model::handle(const gl::Event &event)
{
    bool dirty = false;

    /* Reset the lattice state */
    if (event.type == gl::Event::Key &&
        event.key.code == GLFW_KEY_R &&
        event.key.action == GLFW_RELEASE) {
        m_step = 0;
        dirty = true;
    }

    /* Change the external field */
    if (event.type == gl::Event::Key &&
        event.key.code == GLFW_KEY_LEFT &&
        event.key.action == GLFW_RELEASE) {
        m_ising_h -= 0.001;
        dirty = true;
    }
    if (event.type == gl::Event::Key &&
        event.key.code == GLFW_KEY_RIGHT &&
        event.key.action == GLFW_RELEASE) {
        m_ising_h += 0.001;
        dirty = true;
    }

    /* Change the temperature */
    if (event.type == gl::Event::Key &&
        event.key.code == GLFW_KEY_DOWN &&
        event.key.action == GLFW_RELEASE) {
        m_ising_beta *= 1.01;
        dirty = true;
    }
    if (event.type == gl::Event::Key &&
        event.key.code == GLFW_KEY_UP &&
        event.key.action == GLFW_RELEASE) {
        m_ising_beta /= 1.01;
        dirty = true;
    }

    if (dirty) {
        std::cout << " ising_h " << m_ising_h
                  << " ising_beta " << m_ising_beta
                  << "\n";
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
bool Model::execute(void)
{
    /*
     * Sample a random state for each point in the lattice
     */
    {
        cl::Kernel::set_arg(m_kernels[KernelRandomLattice], 0, sizeof(cl_long), &Params::n_sites);
        cl::Kernel::set_arg(m_kernels[KernelRandomLattice], 1, sizeof(cl_mem),  &m_buffers[BufferRandom]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelRandomLattice],
            cl::NDRange::Null,
            cl::NDRange(
                cl::NDRange::Roundup(Params::n_sites, Params::work_group_size),
                cl::NDRange::Roundup(Params::n_sites, Params::work_group_size)),
            cl::NDRange(Params::work_group_size, Params::work_group_size),
            NULL,
            NULL);
    }

    /*
     * Setup the lattice.
     */
    if (m_step == 0)
    {
        cl::Kernel::set_arg(m_kernels[KernelInitLattice], 0, sizeof(cl_long),   &Params::n_sites);
        cl::Kernel::set_arg(m_kernels[KernelInitLattice], 1, sizeof(cl_mem),    &m_buffers[BufferLattice]);
        cl::Kernel::set_arg(m_kernels[KernelInitLattice], 2, sizeof(cl_mem),    &m_buffers[BufferRandom]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelInitLattice],
            cl::NDRange::Null,
            cl::NDRange(
                cl::NDRange::Roundup(Params::n_sites, Params::work_group_size),
                cl::NDRange::Roundup(Params::n_sites, Params::work_group_size)),
            cl::NDRange(Params::work_group_size, Params::work_group_size),
            NULL,
            NULL);
    }
    if (++m_step == Params::n_steps) {
        m_step = 0;
    }

    /*
     * Flip the lattice for a specified number of steps.
     */
    for (cl_long redblack = RED; redblack <= BLACK; redblack++)
    {
        cl::Kernel::set_arg(m_kernels[KernelFlipLattice], 0, sizeof(cl_double), &m_ising_J);
        cl::Kernel::set_arg(m_kernels[KernelFlipLattice], 1, sizeof(cl_double), &m_ising_h);
        cl::Kernel::set_arg(m_kernels[KernelFlipLattice], 2, sizeof(cl_double), &m_ising_beta);
        cl::Kernel::set_arg(m_kernels[KernelFlipLattice], 3, sizeof(cl_long),   &redblack);
        cl::Kernel::set_arg(m_kernels[KernelFlipLattice], 4, sizeof(cl_long),   &Params::n_sites);
        cl::Kernel::set_arg(m_kernels[KernelFlipLattice], 5, sizeof(cl_mem),    &m_buffers[BufferLattice]);
        cl::Kernel::set_arg(m_kernels[KernelFlipLattice], 6, sizeof(cl_mem),    &m_buffers[BufferRandom]);
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelFlipLattice],
            cl::NDRange::Null,
            cl::NDRange(
                cl::NDRange::Roundup(Params::n_sites, Params::work_group_size),
                cl::NDRange::Roundup(Params::n_sites, Params::work_group_size)),
            cl::NDRange(Params::work_group_size, Params::work_group_size),
            NULL,
            NULL);
    }

    /*
     * Update lattice image for rendering.
     */
    {
        /* Create a sampler object for the image. */
        cl_sampler sampler = cl::Sampler::create(
            m_context,
            CL_FALSE,
            CL_ADDRESS_NONE,
            CL_FILTER_LINEAR);

        /* Set kernel arguments. */
        cl::Kernel::set_arg(m_kernels[KernelImageLattice], 0, sizeof(cl_long),    &Params::n_sites);
        cl::Kernel::set_arg(m_kernels[KernelImageLattice], 1, sizeof(cl_mem),     &m_buffers[BufferLattice]);
        cl::Kernel::set_arg(m_kernels[KernelImageLattice], 2, sizeof(cl_sampler), &sampler);
        cl::Kernel::set_arg(m_kernels[KernelImageLattice], 3, sizeof(cl_long),    &Params::image_width);
        cl::Kernel::set_arg(m_kernels[KernelImageLattice], 4, sizeof(cl_long),    &Params::image_height);
        cl::Kernel::set_arg(m_kernels[KernelImageLattice], 5, sizeof(cl_mem),     &m_images[ImageLattice]);

        /* Wait for OpenGL to finish and acquire the gl objects. */
        cl::gl::enqueue_acquire_gl_objects(
            m_queue,
            &m_images,
            NULL,
            NULL);

        /* Enqueue the OpenCL kernel for execution. */
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelImageLattice],
            cl::NDRange::Null,
            cl::NDRange(
                cl::NDRange::Roundup(Params::image_width,  Params::work_group_size),
                cl::NDRange::Roundup(Params::image_height, Params::work_group_size)),
            cl::NDRange(Params::work_group_size, Params::work_group_size),
            NULL,
            NULL);

        /* Wait for OpenCL to finish and release the gl objects. */
        cl::gl::enqueue_release_gl_objects(
            m_queue,
            &m_images,
            NULL,
            NULL);
    }

    return true;
}
