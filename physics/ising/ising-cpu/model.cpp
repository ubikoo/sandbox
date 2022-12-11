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

        /* Create a lattice of n_sites * n_sites */
        m_lattice = Lattice(Params::n_sites, Params::ising_J, Params::ising_h);

        /* Create lattice graph and connected components object. */
        m_graph = Graph(Params::n_sites * Params::n_sites);
        m_graph_cc = GraphCC(Params::n_sites * Params::n_sites);

        /* Create lattice samplers. */
        m_sampler[MAGNETIC] = Sampler("magnetic");
        m_sampler[MAGNETIC_DENS] = Sampler("magnetic_dens");
        m_sampler[ENERGY] = Sampler("energy");
        m_sampler[CC_COUNT] = Sampler("cc_count");

        /* Setup the random number engine */
        m_random.init();
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
         * Create the shader program object.
         */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/ising.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/ising.frag")};
        m_gl.program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.program) << "\n";

        /*
         * Create a bitmap of n_sites * n_sites with 4 8-bit channels.
         */
        m_gl.image = std::make_unique<gl::Image>(
            Params::n_sites, Params::n_sites, 32);
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
            GL_NEAREST,                     /* filter_min */
            GL_NEAREST);                    /* filter_mag */
        glBindTexture(GL_TEXTURE_2D, 0);

        /*
         * Create a mesh over a rectangle with screen size and set attributes.
         */
        m_gl.mesh = gl::Mesh::Plane(
            m_gl.program,                   /* shader program object */
            "lattice",                      /* vertex attributes prefix name */
            2,                              /* n1 vertices */
            2,                              /* n2 vertices */
            -1.0,                           /* xlo */
            1.0,                            /* xhi */
            -1.0,                           /* ylo */
            1.0);                           /* yhi */
    }
}

/**
 * Model::~Model
 * @brief Destroy the OpenCL context and associated objects.
 */
Model::~Model()
{}

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

    /* Update the vertex buffer data. */
    {
        core_pragma_omp(parallel for default(none) schedule(static))
        for (size_t x = 0; x < m_lattice.n_sites(); ++x) {
            for (size_t y = 0; y < m_lattice.n_sites(); ++y) {
                uint8_t *px = m_gl.image->pixel(x, y);

                if (m_lattice.site(x,y) > 0) {
                    *px++ = 196;
                    *px++ = 0;
                    *px++ = 0;
                }

                if (m_lattice.site(x,y) < 0) {
                    *px++ = 0;
                    *px++ = 0;
                    *px++ = 196;
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
bool Model::execute(void)
{
    /* Set lattice parameters */
    m_lattice.ising_params(m_ising_J, m_ising_h);

    /* Generate a new ensemble */
    if (m_step++ == 0) {
        m_lattice.generate();
        for (size_t i = 0; i < Params::n_equil; ++i) {
            flip();
        }
    }

    /* End of iterations */
    if (m_step == Params::n_iters) {
        return false;
    }

    /* Try to perform a transition on the lattice state */
    for (size_t i = 0; i < Params::n_steps; ++i) {
        flip();
    }

    /* Sample statistics */
    {
        /* Update the connected components of the graph of lattice sites */
        m_graph.clear();
        for (uint32_t x = 0; x < m_lattice.n_sites(); ++x) {
            for (uint32_t y = 0; y < m_lattice.n_sites(); ++y) {
                uint8_t spin0 = m_lattice.site(x, y);
                uint8_t spin1 = m_lattice.site(x - 1, y);
                uint8_t spin2 = m_lattice.site(x, y - 1);

                if (spin0 == spin1) {
                    size_t v = m_lattice.index(x, y);
                    size_t w = m_lattice.index(x - 1, y);
                    m_graph.add_edge(v, w);
                }

                if (spin0 == spin2) {
                    size_t v = m_lattice.index(x, y);
                    size_t w = m_lattice.index(x, y - 1);
                    m_graph.add_edge(v, w);
                }
            }
        }
        m_graph_cc.compute(m_graph);

        /* Sample lattice magnetization. */
        double n_vertices = (double) m_lattice.n_sites() * m_lattice.n_sites();
        m_sampler[MAGNETIC].add(m_lattice.magnetic());
        m_sampler[MAGNETIC_DENS].add(m_lattice.magnetic() / n_vertices);
        m_sampler[ENERGY].add(m_lattice.energy());
        m_sampler[CC_COUNT].add(m_graph_cc.count());

        /* Output statistics */
        if ((16*m_step % Params::n_iters) == 0) {
            for (auto &s : m_sampler) {
                std::cout << s.to_string() << "\n";
            }
        }
    }

    return true;
}

/**
 * Model::flip
 * @brief Flip a lattice site.
 */
void Model::flip(void)
{
    /* Select a random site */
    int32_t x = m_randi(m_random, 0, Params::n_sites);
    int32_t y = m_randi(m_random, 0, Params::n_sites);

    /* Flip the spin and compute the change in energy. */
    double old_energy = m_lattice.energy(x, y);
    m_lattice.site(x, y) *= -1;
    double new_energy = m_lattice.energy(x, y);
    double del_energy = new_energy - old_energy;

    /* Accept/reject the transition. */
    double exp_e = std::exp(-m_ising_beta * del_energy);
    double r = m_randf(m_random, 0.0, 1.0);
    if (r >= exp_e) {
        m_lattice.site(x, y) *= -1;     /* reject and restore the state */
    }
}
