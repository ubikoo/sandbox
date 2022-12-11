/*
 * engine.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef MD_ENGINE_H_
#define MD_ENGINE_H_

#include "atto/opencl/opencl.hpp"
#include "base.hpp"
#include "compute.hpp"
#include "sampler.hpp"
#include "generate.hpp"
#include "io.hpp"
#include "graph.hpp"

/**
 * Engine
 * @brief Molecular dynamics physics engine maintaining a collection of atoms
 * inside a domain.
 *
 * Atom is a plain data structure holding atom attributes - mass, inverse mass,
 * position, momentum and force.
 *
 * The graph structure is represented by an adjacency list where each atom has
 * a pre-specified maximum number of neighbours. The struct Pair represents any
 * edge connecting two neighbour atoms whose pairwise distance is smaller than
 * than a specified radius (>= cutoff radius).
 *
 * Domain is an axis-aligned geometric region centred at (0,0,0) bounded by
 * (-length/2, length/2).
 */
struct Engine {
    /* Engine member variables. */
    std::vector<Atom> m_atoms;          /* fluid atoms */
    Domain m_domain;                    /* fluid domain */
    Field m_field;                      /* fluid pair force field */
    Thermostat m_thermostat;            /* fluid thermostat */
    Thermo m_thermo;                    /* fluid thermodynamic properties */
    Sampler m_sampler;                  /* fluid thermodynamic sampler */
    Graph m_graph;                      /* graph of atom neighbours */

    /** Execute one integration step. */
    void execute(void);

    /** Return serialized fluid thermodynamic properties. */
    std::string sample(void);

    /** Generate atom positions and momenta. */
    void generate(void);

    /** Reset the engine state. */
    void reset(const double radius);

    /** Setup/teardown engine. */
    void setup(void);
    void teardown(void);

    /* Constructor/destructor. */
    Engine() = default;
    ~Engine() = default;
};

#endif /* MD_ENGINE_H_ */
