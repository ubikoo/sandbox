/*
 * io.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "atto/opencl/opencl.hpp"
#include "io.hpp"
using namespace atto;

namespace io {

/**
 * read_xyz
 * @brief Read atom positions configuration from xyz file.
 */
void read_xyz(
    std::vector<Atom> &atoms,
    std::string &comment,
    core::FileIn &file)
{
    core_assert(file.is_open(), "file is not open");
    core_assert(!file.is_binary(), "file is binary");
    std::string buffer;

    /* Read number of atoms. */
    core_assert(file.readline(buffer), "failed to read number of atoms");
    size_t n_atoms = core::str_cast<size_t>(buffer);
    core_assert(n_atoms != atoms.size(), "invalid number of atoms");

    /* Read comment line. */
    core_assert(file.readline(buffer), "failed to read comment");
    comment = buffer;

    /* Read atom positions. */
    for (auto &atom : atoms) {
        core_assert(file.readline(buffer), "failed to read atom");

        std::istringstream entry(buffer);
        std::string name;
        entry >> name >> atom.pos.x >> atom.pos.y >> atom.pos.z;
    }
}

/**
 * write_xyz
 * @brief Write fluid positions configuration into xyz file.
 */
void write_xyz(
    const std::vector<Atom> &atoms,
    const std::string &comment,
    core::FileOut &file)
{
    core_assert(file.is_open(), "file is not open");
    core_assert(!file.is_binary(), "file is binary");
    std::string buffer;

    /* Write number of atoms */
    buffer = core::str_format("%lu\n", atoms.size());
    core_assert(file.writeline(buffer), "failed to write number of atoms");

    /* Write comment line. */
    buffer = core::str_format("%s\n", comment.c_str());
    core_assert(file.writeline(buffer), "failed to write comment");

    /* Write atom positions. */
    for (auto &atom : atoms) {
        buffer = core::str_format(
            "C %lf %lf %lf\n",
            atom.pos.x,
            atom.pos.y,
            atom.pos.z);
        core_assert(file.writeline(buffer), "failed to write atom");
    }
}

} /* io */
