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

#include "engine.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * Engine::Engine
 */
Engine::Engine()
{
    /*
     * Setup Engine data
     */
    {
        /* Create random sampler. */
        m_sample_count = 0;
        m_sample = std::make_unique<Sample>();

        /* Create model camera and film. */
        m_camera = std::make_unique<Camera>(
            Params::eye,
            Params::ctr,
            Params::up,
            Params::fov,
            (double) Params::width / Params::height,
            Params::focus,
            Params::aperture);
        m_film = std::make_unique<Film>(Params::width, Params::height);

        /* Create world and light geometry. */
        // {
        //     Material diffuse = Material::CreateDiffuse(Color{0.5, 0.5, 0.5});
        //     m_world.push_back(Primitive::Create(math::vec3d{0.0, -100.5, -1.0}, 100.0, diffuse));
        //     m_world.push_back(Primitive::Create(math::vec3d{0.0,    0.0, -1.0},   0.5, diffuse));
        // }

        // {
        //     Material mat_ground = Material::CreateDiffuse(Color{0.8, 0.8, 0.0});
        //     Material mat_center = Material::CreateDiffuse(Color{0.1, 0.2, 0.5});
        //     Material mat_left   = Material::CreateDielectric(1.5);
        //     Material mat_right  = Material::CreateConductor(Color{0.8, 0.6, 0.2});

        //     m_world.push_back(Primitive::Create(math::vec3d{ 0.0, -100.5, -1.0}, 100.0, mat_ground));
        //     m_world.push_back(Primitive::Create(math::vec3d{ 0.0,    0.0, -1.0},   0.5, mat_center));
        //     m_world.push_back(Primitive::Create(math::vec3d{-1.0,    0.0, -1.0},   0.5, mat_left));
        //     m_world.push_back(Primitive::Create(math::vec3d{ 1.0,    0.0, -1.0},   0.5, mat_right));
        // }

        // {
        //     Material mat_ground = Material::CreateDiffuse(Color{0.8, 0.8, 0.0});
        //     Material mat_center = Material::CreateDiffuse(Color{0.1, 0.2, 0.5});
        //     Material mat_left   = Material::CreateDielectric(1.5);
        //     Material mat_right  = Material::CreateConductor(Color{0.8, 0.6, 0.2});
        //     Material mat_front  = Material::CreateDielectric(1.5);

        //     m_world.push_back(Primitive::Create(math::vec3d{ 0.0, -100.5, -1.0}, 100.0, mat_ground));
        //     m_world.push_back(Primitive::Create(math::vec3d{ 0.0,    0.0, -1.0},   0.5, mat_center));
        //     m_world.push_back(Primitive::Create(math::vec3d{-1.0,    0.0, -1.0},   0.5, mat_left));
        //     m_world.push_back(Primitive::Create(math::vec3d{ 1.0,    0.0, -1.0},   0.5, mat_right));
        // }

        // {
        //     Material mat_ground = Material::CreateDiffuse(Color{0.8, 0.8, 0.0});
        //     Material mat_center = Material::CreateDiffuse(Color{0.1, 0.2, 0.5});
        //     Material mat_left   = Material::CreateDielectric(1.5);
        //     Material mat_right  = Material::CreateConductor(Color{0.8, 0.6, 0.2});
        //     Material mat_front  = Material::CreateDielectric(1.5);

        //     m_world.push_back(Primitive::Create(math::vec3d{ 0.0, -100.5, -1.0}, 100.0, mat_ground));
        //     m_world.push_back(Primitive::Create(math::vec3d{ 0.0,    0.0, -1.0},   0.5, mat_center));
        //     m_world.push_back(Primitive::Create(math::vec3d{-1.0,    0.0, -1.0},   0.5, mat_left));
        //     m_world.push_back(Primitive::Create(math::vec3d{ 1.0,    0.0, -1.0},   0.5, mat_right));
        //     m_world.push_back(Primitive::Create(math::vec3d{ 0.0,    0.0,  1.0},   0.2, mat_front));
        // }
        {
            m_world = generate(3);
        }
    }

    /*
     * Setup OpenCL data
     */
    { /* Empty */ }


    /*
     * Setup OpenGL data
     */
    {
        /* Create the shader program object. */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/model.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/model.frag")};
        m_gl.program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.program) << "\n";

        /* Load the 2d-image from the specified filename */
        m_gl.texture = gl::create_texture2d(
            GL_RGB8,                    /* texture internal format */
            m_film->width(),            /* texture width */
            m_film->height(),           /* texture height */
            GL_RGB,                     /* pixel format */
            GL_UNSIGNED_BYTE,           /* pixel type */
            nullptr);                   /* pixel data */

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
            "quad",                     /* vertex attributes prefix */
            2,                          /* n1 vertices */
            2,                          /* n2 vertices */
            -1.0,                       /* xlo */
            1.0,                        /* xhi */
            -1.0,                       /* ylo */
            1.0);                       /* yhi */

        /* Create bitmap data. */
        m_gl.bitmap.resize(3 * m_film->width() * m_film->height(), 0);
    }
}

/**
 * Engine::~Engine
 * @brief Destroy the OpenCL context and associated objects.
 */
Engine::~Engine()
{
    /* Teardown model data. */
    {
        /* empty */
    }

    /* Teardown OpenCL data. */
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
 *
 * Engine::handle
 * @brief Handle the event.
 */
void Engine::handle(const gl::Event &event)
{}

/** ---------------------------------------------------------------------------
 * Engine::draw
 * @brief Render the drawable.
 */
void Engine::draw(void *data)
{
    GLFWwindow *window = gl::Renderer::window();
    if (window == nullptr) {
        return;
    }

    /*
     * Update the drawable state.
     */
    {
        /* Copy film bitmap to 2d-texture object. */
        uint8_t *px = &m_gl.bitmap[0];
        for (auto color : m_film->pixels()) {
            color /= (double) m_sample_count;
            *px++ = static_cast<uint8_t>(255.0 * std::sqrt(color.r));
            *px++ = static_cast<uint8_t>(255.0 * std::sqrt(color.g));
            *px++ = static_cast<uint8_t>(255.0 * std::sqrt(color.b));
        }

        glBindTexture(GL_TEXTURE_2D, m_gl.texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,                      /* level of detail - 0 is base bitmap */
            GL_RGB8,                /* texture internal format */
            m_film->width(),        /* texture width */
            m_film->height(),       /* texture height */
            0,                      /* border parameter - must be 0 (legacy) */
            GL_RGB,                 /* pixel format */
            GL_UNSIGNED_BYTE,       /* type of the pixel data(GLubyte) */
            m_gl.bitmap.data());    /* pointer to the pixel data */
        glBindTexture(GL_TEXTURE_2D, 0);
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

    /* Draw the mesh */
    m_gl.mesh->draw();

    /* Unbind the shader program object. */
    glUseProgram(0);
}

/** ---------------------------------------------------------------------------
 * Engine::execute
 * @brief Execute the model.
 */
void Engine::execute(void)
{
    if (m_sample_count++ == Params::num_samples) {
        return;
    }

    size_t width = m_film->width();
    size_t height = m_film->height();
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            math::vec2d u1 = m_sample->rand2d();
            math::vec2d u2 = m_sample->rand2d();
            Ray ray = m_camera->generate_ray(m_film->sample(x, y, u1), u2);
            m_film->add(x, y, radiance(ray));
        }
    }
}

/** ---------------------------------------------------------------------------
 * Engine::radiance
 * @brief Return the radiance along the primary ray using Monte Carlo
 * integration by tracing a path through the world.
 *
 * For each step in the tracing path, compute the closest point of intersection
 * of the ray with the world. At the intersection point x, compute the radiance
 * in the outgoing direction wo as a sum of self-emitted and reflected terms:
 *
 *  L(x->wo) = Le(x->wo) + Lr(x->wo)
 *
 * The term Lr(x->wo) represents the radiance that is reflected by the surface
 * at x in the outgoing direction. It has two contributions: radiance directly
 * reflected from light sources and radiance indirectly reflected from other
 * surfaces in the world.
 */
Color Engine::radiance(Ray &ray)
{
    Color L = Color::Black;         /* path radiance */
    Color beta = Color::White;      /* path attenuation coefficient */
    size_t depth = 0;               /* path depth */
    while (true) {
        /* Stop path tracing if we exceed the maximum path depth. */
        if (++depth >= Params::max_depth) {
            L = Color::Red;
            break;
        }

        /*
         * Compute closest intersection of ray with the world and return
         * the background color if not primitive is intersected.
         */
        Isect isect;
        double t_min = 0.001;
        double t_max = DBL_MAX;
        if (!Primitive::Intersect(m_world, ray, t_min, t_max, isect)) {
            /* Return the background color if no intersection */
            double tx = 0.5 * (ray.d.x + 1.0);
            double ty = 0.5 * (ray.d.y + 1.0);
            Color background = Color(1.0, 1.0, 1.0) * (1.0 - tx - ty) +
                               Color(0.7, 0.7, 0.9) * tx +
                               Color(0.7, 0.9, 0.9) * ty;
            L += beta * background;
            break;
        }

        /*
         * Compute scattering direction and corresponding bsdf
         */
        math::vec2d u = m_sample->rand2d();
        math::vec3d wo = isect.wo;
        math::vec3d wi;
        Color bsdf;
        double pdf;
        if (!Interaction::Scatter(isect, u, wo, wi, bsdf, pdf)) {
            break;
        }
        beta *= bsdf * (Interaction::AbsDot(isect.n, wi) / pdf);

        /* Spawn a ray in the direction oposite the incident direction */
        ray = isect.spawn(wi);
    }

    return L;
}

/** ---------------------------------------------------------------------------
 * Engine::generate
 * @brief Generate a random scene.
 */
std::vector<Primitive> Engine::generate(int32_t n_cells)
{
    std::vector<Primitive> world;

    std::random_device seed;    /* rng device */
    std::mt19937 rng(seed());   /* rng engine */
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    Material ground_material = Material::CreateDiffuse(Color{0.5, 0.5, 0.5});
    world.push_back(Primitive::Create(math::vec3d{0.0, -1000.0 , 0.0}, 1000, ground_material));

    for (int a = -n_cells; a < n_cells; a++) {
        for (int b = -n_cells; b < n_cells; b++) {
            math::vec3d centre{a + 0.9*dist(rng), 0.2, b + 0.9*dist(rng)};
            math::vec3d point{4.0, 0.2, 0.0};

            double choose_mat = dist(rng);
            if (math::norm(centre - point) > 0.9) {
                if (choose_mat < 0.8) {
                    /* Diffuse sphere */
                    Color rho = Color{dist(rng), dist(rng), dist(rng)} *
                                Color{dist(rng), dist(rng), dist(rng)};
                    Material sphere_material = Material::CreateDiffuse(rho);
                    world.push_back(Primitive::Create(centre, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    /* Conductor sphere */
                    Color rho = Color{0.5, 0.5, 0.5};
                    rho += 0.5 * Color{dist(rng), dist(rng), dist(rng)};

                    Material sphere_material = Material::CreateConductor(rho);
                    world.push_back(Primitive::Create(centre, 0.2, sphere_material));
                } else {
                    /* Glass Sphere */
                    Material sphere_material = Material::CreateDielectric(1.5);
                    world.push_back(Primitive::Create(centre, 0.2, sphere_material));
                }
            }
        }
    }

    Material material1 = Material::CreateDielectric(1.5);
    world.push_back(Primitive::Create(math::vec3d{0, 1, 0}, 1.0, material1));

    Material material2 = Material::CreateDiffuse(Color{0.4, 0.2, 0.1});
    world.push_back(Primitive::Create(math::vec3d{-4, 1, 0}, 1.0, material2));

    Material material3 = Material::CreateConductor(Color{0.7, 0.6, 0.5});
    world.push_back(Primitive::Create(math::vec3d{4, 1, 0}, 1.0, material3));

    return world;
}
