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
    {
        m_data.n_points = Params::n_points;
        m_data.n_cells = Params::n_cells;
        m_data.capacity = 1;
        while (m_data.capacity < (Params::load_factor * Params::n_points)) {
            m_data.capacity <<= 1;
        }

        m_data.domain_lo = Params::domain_lo;
        m_data.domain_hi = Params::domain_hi;

        m_data.points.clear();
        for (auto &point : create_box(
            m_data.n_points,
            m_data.domain_lo.s[0],
            m_data.domain_lo.s[1],
            m_data.domain_lo.s[2],
            m_data.domain_hi.s[0],
            m_data.domain_hi.s[1],
            m_data.domain_hi.s[2]))
        {
            m_data.points.push_back({
                (cl_float) point.x,     /* point coordinates */
                (cl_float) point.y,
                (cl_float) point.z,
                0.0f,                   /* point color */
                0.0f,
                0.0f});
        }

        m_data.hashmap.resize(m_data.capacity,
            KeyValue{Params::empty_state, Params::empty_state});
        m_data.random.init();
    }

    /*
     * Setup OpenCL data.
     */
    {
        /*
         * Create a context with a command queue on the specified device.
         */
        m_context = cl::Context::create(CL_DEVICE_TYPE_GPU);
        m_device = cl::Context::get_device(m_context, Params::device_index);
        m_queue = cl::Queue::create(m_context, m_device);
        std::cout << cl::Device::get_info_string(m_device) << "\n";

        /*
         * Create the program object.
         */
        m_program = cl::Program::create_from_file(m_context, "data/hashmap.cl");
        cl::Program::build(m_program, m_device, "");
        std::cout << cl::Program::get_source(m_program) << "\n";

        /*
         * Create the program kernels.
         */
        m_kernels.resize(NumKernels, NULL);
        m_kernels[KernelInsertPoints] = cl::Kernel::create(m_program, "hashmap_insert_points");
        m_kernels[KernelPointColors] = cl::Kernel::create(m_program, "hashmap_point_colors");

        /*
         * Create memory buffers.
         */
        m_buffers.resize(NumBuffers, NULL);
        m_buffers[BufferHashmap] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            m_data.capacity * sizeof(KeyValue),
            (void *) NULL);
        m_buffers[BufferPoints] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            m_data.n_points * sizeof(Point),
            (void *) NULL);
    }

    /*
     * Setup OpenGL data.
     */
    {
        /*
         * Setup camera.
         */
        m_camera.lookat(
            math::vec3f{0.0f, 0.0f, 2.0f},
            math::vec3f{0.0f, 0.0f, 0.0f},
            math::vec3f{0.0f, 1.0f, 0.0f});

        /*
         * Setup Point Data. Create buffer storage for point vertex data
         * with layout: {(xyzrgb)_1, (xyzrgb)_2, ...}
         */
        m_gl.n_points = Params::n_points;
        m_gl.point_vertex.resize(6 * Params::n_points, 0.0);
        m_gl.point_vbo = gl::create_buffer(
            GL_ARRAY_BUFFER,
            m_gl.point_vertex.size() * sizeof(GLfloat),
            GL_STREAM_DRAW);

        /*
         * Setup Sprite Data. Create buffer storage for sprite vertex data
         * with layout: {(uv)_1, (uv)_2, ...}
         */
        m_gl.sprite_vertex = std::vector<GLfloat>{
            0.0f,  0.0f,                        /* bottom left */
            1.0f,  0.0f,                        /* bottom right */
            0.0f,  1.0f,                        /* top left */
            1.0f,  1.0f};                       /* top right */
        m_gl.sprite_index = std::vector<GLuint>{
            0, 1, 2,                            /* first triangle */
            3, 2, 1};                           /* second triangle */

        m_gl.sprite_vbo = gl::create_buffer(
            GL_ARRAY_BUFFER,
            m_gl.sprite_vertex.size() * sizeof(GLfloat),
            GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, m_gl.sprite_vbo);
        glBufferSubData(
            GL_ARRAY_BUFFER,                            /* target binding point */
            0,                                          /* offset in data store */
            m_gl.sprite_vertex.size() * sizeof(GLfloat),/* data store size in bytes */
            m_gl.sprite_vertex.data());                 /* pointer to data source */
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_gl.sprite_ebo = gl::create_buffer(
            GL_ELEMENT_ARRAY_BUFFER,
            m_gl.sprite_index.size() * sizeof(GLuint),
            GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl.sprite_ebo);
        glBufferSubData(
            GL_ELEMENT_ARRAY_BUFFER,                    /* target binding point */
            0,                                          /* offset in data store */
            m_gl.sprite_index.size() * sizeof(GLuint),  /* data store size in bytes */
            m_gl.sprite_index.data());                  /* pointer to data source */
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        /*
         * Create Point shader program object.
         */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/point-shader.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/point-shader.frag")};
        m_gl.program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.program) << "\n";

        /* Create vertex array object. */
        m_gl.vao = gl::create_vertex_array();
        glBindVertexArray(m_gl.vao);

        /* Set point sprite vertex data format. */
        glBindBuffer(GL_ARRAY_BUFFER, m_gl.sprite_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl.sprite_ebo);

        gl::enable_attribute(m_gl.program, "a_sprite_coord");
        gl::attribute_pointer(
            m_gl.program,
            "a_sprite_coord",
            GL_FLOAT_VEC2,
            2*sizeof(GLfloat),  /* byte offset between consecutive attributes */
            0,                  /* byte offset of first element in the buffer */
            false);             /* normalized flag */

        /* Set point vertex data format. */
        glBindBuffer(GL_ARRAY_BUFFER, m_gl.point_vbo);

        gl::enable_attribute(m_gl.program, "a_point_pos");
        gl::attribute_pointer(
            m_gl.program,
            "a_point_pos",
            GL_FLOAT_VEC3,
            6*sizeof(GLfloat),  /* byte offset between consecutive attributes */
            0,                  /* byte offset of first element in the buffer */
            false);             /* normalized flag */
        gl::attribute_divisor(m_gl.program, "a_point_pos", 1);

        gl::enable_attribute(m_gl.program, "a_point_col");
        gl::attribute_pointer(
            m_gl.program,
            "a_point_col",
            GL_FLOAT_VEC3,
            6*sizeof(GLfloat),  /* byte offset between consecutive attributes */
            3*sizeof(GLfloat),  /* byte offset of first element in the buffer */
            false);             /* normalized flag */
        gl::attribute_divisor(m_gl.program, "a_point_col", 1);

        /* Unbind vertex array object. */
        glBindVertexArray(0);
    }
}

/** ---------------------------------------------------------------------------
 * Model::~Model
 * @brief Destroy the OpenCL context and associated objects.
 */
Model::~Model()
{
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
{
    const float move_scale = 0.02f;
    const float rotate_scale = 0.02f;
    const float size_scale = 1.01f;

    /*
     * Handle camera movement.
     */
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_W) {
        m_camera.move(-m_camera.eye() * move_scale);
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_S) {
        m_camera.move( m_camera.eye() * move_scale);
    }

    /*
     * Handle camera rotation.
     */
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_UP) {
        m_camera.rotate_pitch( rotate_scale*M_PI);
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_DOWN) {
        m_camera.rotate_pitch(-rotate_scale*M_PI);
    }

    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_LEFT) {
        m_camera.rotate_yaw( rotate_scale*M_PI);
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_RIGHT) {
        m_camera.rotate_yaw(-rotate_scale*M_PI);
    }

    /*
     * Handle point size changes.
     */
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_COMMA) {
        m_gl.point_scale /= size_scale;
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_PERIOD) {
        m_gl.point_scale *= size_scale;
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

    /* Update the vertex buffer data. */
    for (GLsizei i = 0; i < m_gl.n_points; i++) {
        GLsizei j = 6*i;
        m_gl.point_vertex[j++] = m_data.points[i].pos.s[0];
        m_gl.point_vertex[j++] = m_data.points[i].pos.s[1];
        m_gl.point_vertex[j++] = m_data.points[i].pos.s[2];
        m_gl.point_vertex[j++] = m_data.points[i].col.s[0];
        m_gl.point_vertex[j++] = m_data.points[i].col.s[1];
        m_gl.point_vertex[j++] = m_data.points[i].col.s[2];
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_gl.point_vbo);
    glBufferSubData(
        GL_ARRAY_BUFFER,                    /* target binding point */
        0,                                  /* offset in data store */
        m_gl.point_vertex.size() * sizeof(GLfloat), /* data store size in bytes */
        m_gl.point_vertex.data());     /* pointer to data source */
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /*
     * Specify draw state modes.
     */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Bind the shader program object and vertex array object. */
    glUseProgram(m_gl.program);
    glBindVertexArray(m_gl.vao);

    /* Set uniforms and draw. */
#if 0
    std::array<GLfloat,2> sizef = gl::Renderer::framebuffer_sizef();
    gl::set_uniform(m_gl.program, "u_width", GL_FLOAT, &sizef[0]);
    gl::set_uniform(m_gl.program, "u_height",GL_FLOAT, &sizef[1]);
#endif

    gl::set_uniform(m_gl.program, "u_scale", GL_FLOAT, &m_gl.point_scale);
    gl::set_uniform_matrix(m_gl.program, "u_view", GL_FLOAT_MAT4, true,
        m_camera.view().data());
    gl::set_uniform_matrix(m_gl.program, "u_persp", GL_FLOAT_MAT4, true,
        m_camera.persp().data());

    glDrawElementsInstanced(
        GL_TRIANGLES,               /* what kind of primitives? */
        m_gl.sprite_index.size(),   /* number of elements to render */
        GL_UNSIGNED_INT,            /* type of the values in indices */
        (GLvoid *) 0,               /* pointer to indices storage location */
        m_gl.n_points);             /* number of instances to be rendered */

    /* Unbind the vertex array object and shader program object. */
    glBindVertexArray(0);
    glUseProgram(0);
}

/** ---------------------------------------------------------------------------
 * Model::execute
 * @brief Execute the model.
 */
void Model::execute(void)
{
    /*
     * Update the point data.
     */
    {
        const float avg = 0.0f;
        const float sdev = 0.001f;
        math::rng::gauss<float> rand;
        for (auto &p : m_data.points) {
            p.pos.s[0] += rand(m_data.random, avg, sdev);
            p.pos.s[1] += rand(m_data.random, avg, sdev);
            p.pos.s[2] += rand(m_data.random, avg, sdev);
        }

        /* Update state of particle 0 */
        {
            math::vec3f domain_hi(
                m_data.domain_hi.s[0],
                m_data.domain_hi.s[1],
                m_data.domain_hi.s[2]);
            math::vec3f domain_lo(
                m_data.domain_lo.s[0],
                m_data.domain_lo.s[1],
                m_data.domain_lo.s[2]);

            float dt = 0.002f;
            float theta = 0.002f * glfwGetTime();
            float radius = std::cos(theta) * math::norm(domain_hi - domain_lo);

            float x = m_data.points[0].pos.s[0];
            float y = m_data.points[0].pos.s[1];

            m_data.points[0].pos.s[0] -= dt * radius * y;
            m_data.points[0].pos.s[1] += dt * radius * x;
            m_data.points[0].pos.s[2] += dt * radius * std::cos(theta);
        }

        /* Apply boundary conditions */
        cl_float3 domain_len = (cl_float3) {
            m_data.domain_hi.s[0] - m_data.domain_lo.s[0],
            m_data.domain_hi.s[1] - m_data.domain_lo.s[1],
            m_data.domain_hi.s[2] - m_data.domain_lo.s[2]};
        for (auto &p : m_data.points) {
            for (size_t i = 0; i < 3; ++i) {
                if (p.pos.s[i] < m_data.domain_lo.s[i]) {
                    p.pos.s[i] += domain_len.s[i];
                }

                if (p.pos.s[i] > m_data.domain_hi.s[i]) {
                    p.pos.s[i] -= domain_len.s[i];
                }
            }
        }
    }

    /*
     * Create the hashmap from the array of particles.
     */
    {
        /* Reset the hashmap data. */
        std::fill(
            m_data.hashmap.begin(),
            m_data.hashmap.end(),
            KeyValue{Params::empty_state, Params::empty_state});

        /* Update the gpu buffer store. */
        cl::Queue::enqueue_write_buffer(
            m_queue,
            m_buffers[BufferHashmap],
            CL_TRUE,
            0,
            m_data.capacity * sizeof(KeyValue),
            (void *) &m_data.hashmap[0],
            NULL,
            NULL);

        cl::Queue::enqueue_write_buffer(
            m_queue,
            m_buffers[BufferPoints],
            CL_TRUE,
            0,
            m_data.n_points * sizeof(Point),
            (void *) &m_data.points[0],
            NULL,
            NULL);

        /* Set kernel arguments. */
        cl::Kernel::set_arg(m_kernels[KernelInsertPoints], 0, sizeof(cl_mem),    &m_buffers[BufferHashmap]);
        cl::Kernel::set_arg(m_kernels[KernelInsertPoints], 1, sizeof(cl_mem),    &m_buffers[BufferPoints]);
        cl::Kernel::set_arg(m_kernels[KernelInsertPoints], 2, sizeof(cl_uint),   &m_data.capacity);
        cl::Kernel::set_arg(m_kernels[KernelInsertPoints], 3, sizeof(cl_uint),   &m_data.n_points);
        cl::Kernel::set_arg(m_kernels[KernelInsertPoints], 4, sizeof(cl_uint),   &m_data.n_cells);
        cl::Kernel::set_arg(m_kernels[KernelInsertPoints], 5, sizeof(cl_float3), &m_data.domain_lo);
        cl::Kernel::set_arg(m_kernels[KernelInsertPoints], 6, sizeof(cl_float3), &m_data.domain_hi);

        /* Set the size of the NDRange workgroups */
        cl::NDRange local_ws(Params::work_group_size);
        cl::NDRange global_ws(cl::NDRange::Roundup(m_data.n_points, Params::work_group_size));

        /* Run the kernel */
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelInsertPoints],
            cl::NDRange::Null,
            global_ws,
            local_ws,
            NULL,
            NULL);

        /* Wait for kernel to compute */
        cl::Queue::finish(m_queue);

        /* Read the hashmap buffer back to the host. */
        cl::Queue::enqueue_read_buffer(
            m_queue,
            m_buffers[BufferHashmap],
            CL_TRUE,
            0,
            m_data.capacity * sizeof(KeyValue),
            (void *) &m_data.hashmap[0],
            NULL,
            NULL);
    }

    /*
     * Color the particles based on the distance to the first particle.
     */
    {
        /* Update the gpu buffer store. */
        cl::Queue::enqueue_write_buffer(
            m_queue,
            m_buffers[BufferPoints],
            CL_TRUE,
            0,
            m_data.n_points * sizeof(Point),
            (void *) &m_data.points[0],
            NULL,
            NULL);

        cl::Kernel::set_arg(m_kernels[KernelPointColors], 0, sizeof(cl_mem),    &m_buffers[BufferPoints]);
        cl::Kernel::set_arg(m_kernels[KernelPointColors], 1, sizeof(cl_uint),   &m_data.n_points);
        cl::Kernel::set_arg(m_kernels[KernelPointColors], 2, sizeof(cl_uint),   &m_data.n_cells);
        cl::Kernel::set_arg(m_kernels[KernelPointColors], 3, sizeof(cl_float3), &m_data.domain_lo);
        cl::Kernel::set_arg(m_kernels[KernelPointColors], 4, sizeof(cl_float3), &m_data.domain_hi);

        /* Set the size of the NDRange workgroups */
        cl::NDRange local_ws(Params::work_group_size);
        cl::NDRange global_ws(cl::NDRange::Roundup(m_data.n_points, Params::work_group_size));

        /* Run the kernel */
        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelPointColors],
            cl::NDRange::Null,
            global_ws,
            local_ws,
            NULL,
            NULL);

        /* Wait for kernel to compute */
        cl::Queue::finish(m_queue);

        /* Read the hashmap buffer back to the host. */
        cl::Queue::enqueue_read_buffer(
            m_queue,
            m_buffers[BufferPoints],
            CL_TRUE,
            0,
            m_data.n_points * sizeof(Point),
            (void *) &m_data.points[0],
            NULL,
            NULL);
    }
}
