/*
 * io.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef MD_IO_H_
#define MD_IO_H_

#include "atto/opencl/opencl.hpp"
#include "base.hpp"

namespace io {

/** Read atom positions configuration from xyz file. */
void read_xyz(
    std::vector<Atom> &atoms,
    std::string &comment,
    atto::core::FileIn &file);

/** Write fluid positions configuration into xyz file. */
void write_xyz(
    const std::vector<Atom> &atoms,
    const std::string &comment,
    atto::core::FileOut &file);

} /* io */

#endif /* MD_IO_H_ */
