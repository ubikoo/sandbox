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

        m_data.hashmap = std::make_unique<Hashmap>(
            Params::load_factor * Params::n_points);
        m_data.random.init();
    }

    /*
     * Setup OpenCL data.
     */
    { /* Empty */ }

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
            GL_ARRAY_BUFFER,                    /* target binding point */
            0,                                  /* offset in data store */
            m_gl.sprite_vertex.size() * sizeof(GLfloat), /* data store size in bytes */
            m_gl.sprite_vertex.data());         /* pointer to data source */
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_gl.sprite_ebo = gl::create_buffer(
            GL_ELEMENT_ARRAY_BUFFER,
            m_gl.sprite_index.size() * sizeof(GLuint),
            GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl.sprite_ebo);
        glBufferSubData(
            GL_ELEMENT_ARRAY_BUFFER,            /* target binding point */
            0,                                  /* offset in data store */
            m_gl.sprite_index.size() * sizeof(GLuint), /* data store size in bytes */
            m_gl.sprite_index.data());          /* pointer to data source */
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
    // /* Teardown OpenCL data. */
    // {
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
    //     cl::Queue::release(m_queue);
    //     cl::Device::release(m_device);
    //     cl::Context::release(m_context);
    // }
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
    /**
     * hash
     * Compute the hash value of a cell index.
     */
    auto hash = [&] (const cl_uint3 &v)
    {
        const uint32_t c1 = static_cast<uint32_t>(73856093);
        const uint32_t c2 = static_cast<uint32_t>(19349663);
        const uint32_t c3 = static_cast<uint32_t>(83492791);

        uint32_t h1 = c1 * v.s[0];
        uint32_t h2 = c2 * v.s[1];
        uint32_t h3 = c3 * v.s[2];

        return (h1 ^ h2 ^ h3);
        // return (7*h1 + 503*h2 + 24847*h3);
    };

    /**
     * cell_ix
     * Compute the cell index of a given point.
     */
    auto cell_ix = [&] (const cl_float3 &point)
    {
        cl_float cell_unit = 1.0f / (cl_float) m_data.n_cells;

        cl_float3 cell_length = (cl_float3) {
            (m_data.domain_hi.s[0] - m_data.domain_lo.s[0]) * cell_unit,
            (m_data.domain_hi.s[1] - m_data.domain_lo.s[1]) * cell_unit,
            (m_data.domain_hi.s[2] - m_data.domain_lo.s[2]) * cell_unit};

        cl_float3 u_point = (cl_float3) {
            (point.s[0] - m_data.domain_lo.s[0]) / cell_length.s[0],
            (point.s[1] - m_data.domain_lo.s[1]) / cell_length.s[1],
            (point.s[2] - m_data.domain_lo.s[2]) / cell_length.s[2]};

        const uint32_t v1 = static_cast<uint32_t>(u_point.s[0]);
        const uint32_t v2 = static_cast<uint32_t>(u_point.s[1]);
        const uint32_t v3 = static_cast<uint32_t>(u_point.s[2]);

        return (cl_uint3) {v1, v2, v3};
    };

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

        {
        /* Update state of particle 0 */
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
            float radius = std::cos(theta) * math::norm(domain_hi-domain_lo);

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
    m_data.hashmap->clear();
    for (size_t i = 0; i < m_data.points.size(); ++i) {
         m_data.hashmap->insert(hash(cell_ix(m_data.points[i].pos)), (uint32_t) i);
    }

    /*
     * Color the particles based on the distance to the first particle.
     */
    {
        for (auto &it : m_data.points) {
            it.col = (cl_float3) {0.3f, 0.3f, 0.3f};
        }

        uint32_t key = hash(cell_ix(m_data.points[0].pos));
        uint32_t slot = m_data.hashmap->begin(key);
        while (slot != m_data.hashmap->end()) {
            uint32_t ix = m_data.hashmap->get(slot);

           cl_float3 col = (cl_float3) {
                m_data.points[ix].pos.s[0] - m_data.domain_lo.s[0],
                m_data.points[ix].pos.s[1] - m_data.domain_lo.s[1],
                m_data.points[ix].pos.s[2] - m_data.domain_lo.s[2]};

            m_data.points[ix].col = (cl_float3) {
                col.s[0] / (m_data.domain_hi.s[0] - m_data.domain_lo.s[0]),
                col.s[1] / (m_data.domain_hi.s[1] - m_data.domain_lo.s[1]),
                col.s[2] / (m_data.domain_hi.s[2] - m_data.domain_lo.s[2])};

            slot = m_data.hashmap->next(key, slot);
        }

        m_data.points[0].col = (cl_float3) {1.0f, 1.0f, 1.0f};
    }
}
