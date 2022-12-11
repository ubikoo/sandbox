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

    /* Lattice enumerated color. */
    enum : int32_t {
        RED = 0,
        BLACK,
    };

    /* Array of random number generators */
    struct Rng {
        atto::math::rng::Kiss engine;
        atto::math::rng::uniform<int32_t> randi;
        atto::math::rng::uniform<double> randf;
    };
    std::vector<Rng> m_rng;

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

    /** Flip the lattice. */
    void flip(void);

    /** Flip the lattice row with a specified shift. */
    void flip(int32_t row, int32_t shift, Rng &rng);

    /* Constructor/destructor. */
    Model();
    ~Model();
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
};

#endif /* ISING_MODEL_H_ */
