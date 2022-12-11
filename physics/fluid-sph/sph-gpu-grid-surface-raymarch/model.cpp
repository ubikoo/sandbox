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
        /* Setup camera. */
        m_gl.camera.lookat(
            math::vec3f{0.0f, 0.0f, 2.0f},
            math::vec3f{0.0f, 0.0f, 0.0f},
            math::vec3f{0.0f, 1.0f, 0.0f});

        /*
         * Setup Point Data. Create buffer storage for point vertex data
         * with layout: {(xyz)_1, (xyz)_2, ...}
         */
        m_gl.n_points = Params::n_particles;
        m_gl.point_vbo = gl::create_buffer(
            GL_ARRAY_BUFFER,
            3 * m_gl.n_points * sizeof(GLfloat),
            GL_STREAM_DRAW);
        m_gl.point_scale = 1.0f;

        /*
         * Setup Sprite Data. Create buffer storage for sprite vertex
         * data with layout: {(uv)_1, (uv)_2, ...}
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
            m_gl.sprite_vertex.size() * sizeof(GLfloat),/* data store size in bytes */
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
            m_gl.sprite_index.size() * sizeof(GLuint),  /* data store size in bytes */
            m_gl.sprite_index.data());          /* pointer to data source */
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        /*
         * Setup render framebuffers
         */
        m_gl.fbosize = gl::Renderer::framebuffer_sizef();
        m_gl.iodepth = std::make_unique<IODepth>(m_gl.fbosize[0], m_gl.fbosize[1]);
        m_gl.iobuffer[0] = std::make_unique<IOBuffer>(m_gl.fbosize[0], m_gl.fbosize[1]);
        m_gl.iobuffer[1] = std::make_unique<IOBuffer>(m_gl.fbosize[0], m_gl.fbosize[1]);
        m_gl.read_ix = 0;
        m_gl.write_ix = 1;
    }

    /*
     * Setup draw sprite shader.
     */
    {
        /* Don't replace depth initially */
        m_gl.replace_depth = 0;

        /* Create shader program object. */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/draw-sprite.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/draw-sprite.frag")};
        m_gl.draw_sprite_program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.draw_sprite_program) << "\n";

        /* Create vertex array object. */
        m_gl.draw_sprite_vao = gl::create_vertex_array();
        glBindVertexArray(m_gl.draw_sprite_vao);

        /* Set point sprite vertex data format. */
        glBindBuffer(GL_ARRAY_BUFFER, m_gl.sprite_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl.sprite_ebo);

        gl::enable_attribute(m_gl.draw_sprite_program, "sprite_coord");
        gl::attribute_pointer(
            m_gl.draw_sprite_program,
            "sprite_coord",
            GL_FLOAT_VEC2,
            2*sizeof(GLfloat),  /* byte offset between consecutive attributes */
            0,                  /* byte offset of first element in the buffer */
            false);             /* normalized flag */

        /* Set point vertex data format. */
        glBindBuffer(GL_ARRAY_BUFFER, m_gl.point_vbo);

        gl::enable_attribute(m_gl.draw_sprite_program, "point_pos");
        gl::attribute_pointer(
            m_gl.draw_sprite_program,
            "point_pos",
            GL_FLOAT_VEC3,
            3*sizeof(GLfloat),  /* byte offset between consecutive attributes */
            0,                  /* byte offset of first element in the buffer */
            false);             /* normalized flag */
        gl::attribute_divisor(m_gl.draw_sprite_program, "point_pos", 1);

        /* Unbind vertex array object. */
        glBindVertexArray(0);
    }

    /*
     * Setup draw depth shader.
     */
    {
        /* Create shader program object. */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/depth-compute.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/depth-compute.frag")};
        m_gl.depth_compute_program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.depth_compute_program) << "\n";

        /* Create a mesh and set the vertex attributes in the program. */
        m_gl.depth_compute_quad = gl::Mesh::Plane(
            m_gl.depth_compute_program,    /* shader program object */
            "quad",                     /* vertex attributes prefix */
            2,                          /* n1 vertices */
            2,                          /* n2 vertices */
            -1.0,                       /* xlo */
             1.0,                       /* xhi */
            -1.0,                       /* ylo */
             1.0);                      /* yhi */
    }

    /*
     * Draw curvature flow smooth shader.
     */
    {
        /* Don't replace depth initially */
        m_gl.compute_normal = 0;

        /* Create shader program object. */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/depth-smooth.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/depth-smooth.frag")};
        m_gl.depth_smooth_program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.depth_smooth_program) << "\n";

        /* Create a mesh and set the vertex attributes in the program. */
        m_gl.depth_smooth_quad = gl::Mesh::Plane(
            m_gl.depth_smooth_program,   /* shader program object */
            "quad",                     /* vertex attributes prefix */
            2,                          /* n1 vertices */
            2,                          /* n2 vertices */
            -1.0,                       /* xlo */
             1.0,                       /* xhi */
            -1.0,                       /* ylo */
             1.0);                      /* yhi */
    }

    /*
     * Setup draw surface shader.
     */
    {
        /* Create shader program object. */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/draw-surface.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/draw-surface.frag")};
        m_gl.draw_surface_program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.draw_surface_program) << "\n";

        /* Create a mesh and set the vertex attributes in the program. */
        m_gl.draw_surface_quad = gl::Mesh::Plane(
            m_gl.draw_surface_program,  /* shader program object */
            "quad",                     /* vertex attributes prefix */
            2,                          /* n1 vertices */
            2,                          /* n2 vertices */
            -1.0,                       /* xlo */
             1.0,                       /* xhi */
            -1.0,                       /* ylo */
             1.0);                      /* yhi */
    }

    /* ------------------------------------------------------------------------
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
    }

    /* ------------------------------------------------------------------------
     * Setup engine data.
     */
    {
        m_engine.setup(m_context, m_device, m_queue, m_gl.point_vbo);
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
        m_engine.teardown();
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
 * @brief Handle the event.
 */
void Model::handle(const gl::Event &event)
{
    const float move_scale = 0.02f;
    const float rotate_scale = 0.02f;
    const float size_scale = 1.01f;

    /*
     * Handle camera
     */
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_W) {
        m_gl.camera.move(-m_gl.camera.eye() * move_scale);
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_S) {
        m_gl.camera.move( m_gl.camera.eye() * move_scale);
    }

    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_UP) {
        m_gl.camera.rotate_pitch( rotate_scale*M_PI);
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_DOWN) {
        m_gl.camera.rotate_pitch(-rotate_scale*M_PI);
    }

    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_LEFT) {
        m_gl.camera.rotate_yaw( rotate_scale*M_PI);
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_RIGHT) {
        m_gl.camera.rotate_yaw(-rotate_scale*M_PI);
    }

    /*
     * Handle point size changes.
     */
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_COMMA) {
        m_gl.point_scale /= size_scale;
        std::cout << "point scale " << m_gl.point_scale << "\n";
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_PERIOD) {
        m_gl.point_scale *= size_scale;
        std::cout << "point scale " << m_gl.point_scale << "\n";
    }

    /*
     * Change the external force direction.
     */
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_1) {
        m_engine.m_gravity = cl_float4{Params::gravity_coeff, 0.0f, 0.0f, 0.0f};
    }

    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_2) {
        m_engine.m_gravity = cl_float4{0.0f, Params::gravity_coeff, 0.0f, 0.0f};
    }

    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_3) {
        m_engine.m_gravity = cl_float4{0.0f, 0.0f, Params::gravity_coeff, 0.0f};
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

    /* Specify draw state. */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /*
     * Render particle sprites onto the depth render texture.
     */
    {
        /* Bind the depth framebuffer */
        m_gl.iodepth->bind();
        auto viewport_save = gl::Renderer::viewport();
        gl::Renderer::viewport({0, 0, m_gl.fbosize[0], m_gl.fbosize[1]});
        gl::Renderer::clear(0.5f, 0.5f, 0.5f, 1.0f, 1.0f);

        /* Bind the shader program object and vertex array object. */
        glUseProgram(m_gl.draw_sprite_program);
        glBindVertexArray(m_gl.draw_sprite_vao);

        /* Set uniforms and draw. */
        gl::set_uniform(m_gl.draw_sprite_program, "u_replace_depth", GL_UNSIGNED_INT,
            &m_gl.replace_depth);
        gl::set_uniform(m_gl.draw_sprite_program, "u_scale", GL_FLOAT,
            &m_gl.point_scale);
        gl::set_uniform_matrix(m_gl.draw_sprite_program, "u_view", GL_FLOAT_MAT4, true,
            m_gl.camera.view().data());
        gl::set_uniform_matrix(m_gl.draw_sprite_program, "u_persp", GL_FLOAT_MAT4, true,
            m_gl.camera.persp().data());

        glDrawElementsInstanced(
            GL_TRIANGLES,               /* what kind of primitives? */
            m_gl.sprite_index.size(),   /* number of elements to render */
            GL_UNSIGNED_INT,            /* type of the values in indices */
            (GLvoid *) 0,               /* pointer to indices storage location */
            m_gl.n_points);             /* number of instances to be rendered */

        /* Unbind the vertex array object and shader program object. */
        glBindVertexArray(0);
        glUseProgram(0);

        /* Unbind the depth framebuffer */
        m_gl.iodepth->unbind();
        gl::Renderer::viewport(viewport_save);
        gl::Renderer::clear(0.5f, 0.5f, 0.5f, 1.0f, 1.0f);
    }

    /*
     * Render particle the depth render texture onto the iobuffer.
     */
    {
        /* Swap read/write buffers. */
        std::swap(m_gl.read_ix, m_gl.write_ix);

        /* Bind the begin shader */
        m_gl.iobuffer[m_gl.write_ix]->bind();
        glUseProgram(m_gl.depth_compute_program);

        /* Set uniforms and draw. */
        gl::set_uniform(m_gl.depth_compute_program, "u_znear", GL_FLOAT, &m_gl.camera.znear());
        gl::set_uniform(m_gl.depth_compute_program, "u_zfar", GL_FLOAT, &m_gl.camera.zfar());

        /* Set the sampler uniform with the texture unit and bind the texture */
        GLenum texunit = 0;
        gl::set_uniform(m_gl.depth_compute_program, "u_tex_depth", GL_SAMPLER_2D, &texunit);
        gl::active_bind_texture(GL_TEXTURE_2D, GL_TEXTURE0 + texunit, m_gl.iodepth->texture());

        /* Draw the quad mesh */
        m_gl.depth_compute_quad->draw();

        /* Unbind the shader program object. */
        glUseProgram(0);
        m_gl.iobuffer[m_gl.write_ix]->unbind();
    }

    /*
     * Draw curvature flow smooth shader.
     */
    for (size_t iter = 0; iter < 5; ++iter)
    {
        /* Swap read/write buffers. */
        std::swap(m_gl.read_ix, m_gl.write_ix);

        /* Bind the begin shader */
        m_gl.iobuffer[m_gl.write_ix]->bind();
        glUseProgram(m_gl.depth_smooth_program);

        /* Set uniforms and draw. */
        std::array<GLfloat,2> sizef = gl::Renderer::framebuffer_sizef();
        gl::set_uniform(m_gl.depth_smooth_program, "u_width", GL_FLOAT, &sizef[0]);
        gl::set_uniform(m_gl.depth_smooth_program, "u_height", GL_FLOAT, &sizef[1]);

        /* Set the sampler uniform with the texture unit and bind the texture */
        GLenum texunit = 0;
        gl::set_uniform(m_gl.depth_smooth_program, "u_tex_depth", GL_SAMPLER_2D, &texunit);
        gl::active_bind_texture(GL_TEXTURE_2D, GL_TEXTURE0 + texunit,
            m_gl.iobuffer[m_gl.read_ix]->texture());

        /* Draw the quad mesh */
        m_gl.depth_smooth_quad->draw();

        /* Unbind the shader program object. */
        glUseProgram(0);
        m_gl.iobuffer[m_gl.write_ix]->unbind();
    }

    /*
     * Draw surface shader.
     */
    {
        /* Swap read/write buffers. */
        std::swap(m_gl.read_ix, m_gl.write_ix);

        /* Bind the begin shader */
        glUseProgram(m_gl.draw_surface_program);

        /* Set uniforms and draw. */
        std::array<GLfloat,2> sizef = gl::Renderer::framebuffer_sizef();
        gl::set_uniform(m_gl.draw_surface_program, "u_width", GL_FLOAT, &sizef[0]);
        gl::set_uniform(m_gl.draw_surface_program, "u_height", GL_FLOAT, &sizef[1]);
        gl::set_uniform(m_gl.draw_surface_program, "u_compute_normal", GL_UNSIGNED_INT,
            &m_gl.compute_normal);

        /* Set the sampler uniform with the texture unit and bind the texture */
        GLenum texunit = 0;
        gl::set_uniform(m_gl.draw_surface_program, "u_tex_depth", GL_SAMPLER_2D, &texunit);
        gl::active_bind_texture(GL_TEXTURE_2D, GL_TEXTURE0 + texunit,
            m_gl.iobuffer[m_gl.read_ix]->texture());

        /* Draw the quad mesh */
        m_gl.draw_surface_quad->draw();

        /* Unbind the shader program object. */
        glUseProgram(0);
    }
}

/** ---------------------------------------------------------------------------
 * Model::execute
 * @brief Execute the model.
 */
bool Model::execute(void)
{
    /* Execute an engine step. */
    m_engine.execute();

    /* Return true if engine finished integration. */
    return m_engine.finished();
}
