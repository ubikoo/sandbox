/*
 * model-cpu.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef MODEL_CPU_H_
#define MODEL_CPU_H_

#include <vector>
#include <utility>
#include "ito/opengl.hpp"
#include "ito/opencl.hpp"
#include "types.hpp"
#include "hashmap.hpp"

struct ModelCPU {
    Hashmap m_hashmap;
    std::vector<std::pair<uint32_t, uint32_t>> m_keys;

    const std::pair<uint32_t, uint32_t> &key(size_t i) const {
        return m_keys[i];
    }
    void Execute(const std::vector<Point> &points);

    static ModelCPU Create(void);
    static void Destroy(ModelCPU &model);
};

#endif /* MODEL_CPU_H_ */
