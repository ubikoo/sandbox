/*
 * model.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef ISING_MODEL_H_
#define ISING_MODEL_H_

#include "base.hpp"
#include "lattice.hpp"
#include "sampler.hpp"
#include "graph.hpp"

struct Model : atto::gl::Drawable {
    /* ---- Model data ----------------------------------------------------- */
    size_t m_step;          /* iteration counter */
    double m_ising_J;       /* Ising energy coefficient */
    double m_ising_h;       /* Ising external field coefficient */
    double m_ising_beta;    /* Inverse reduced temperature */

    Lattice m_lattice;      /* model lattice */
    Graph m_graph;          /* graph data structure */
    GraphCC m_graph_cc;     /* graph connected components */

    /* Sampler enumerated type */
    enum : uint32_t {
        /* Fluid mass */
        MAGNETIC = 0,
        MAGNETIC_DENS,
        ENERGY,
        CC_COUNT,
        NUM_SAMPLERS
    };
    std::array<Sampler, NUM_SAMPLERS> m_sampler;

    /* Random number generator */
    atto::math::rng::Kiss m_random;
    atto::math::rng::uniform<int32_t> m_randi;
    atto::math::rng::uniform<double> m_randf;

    /* ---- Model OpenGL data ---------------------------------------------- */
    struct GLData {
        GLuint program;                         /* shader program object */
        std::unique_ptr<atto::gl::Mesh> mesh;   /* lattice quad mesh */
        std::unique_ptr<atto::gl::Image> image; /* lattice image */
        GLuint texture;                         /* lattice texture */
    } m_gl;

    /* ---- Model member functions ----------------------------------------- */
    void handle(const atto::gl::Event &event) override;
    void draw(void *data = nullptr) override;
    bool execute(void);

    /** Flip a lattice site. */
    void flip(void);

    /* Constructor/destructor. */
    Model();
    ~Model();
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
};

#endif /* ISING_MODEL_H_ */
