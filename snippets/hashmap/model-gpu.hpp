/*
 * model-gpu.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef MODEL_GPU_H_
#define MODEL_GPU_H_

#include <vector>
#include "ito/opengl.hpp"
#include "ito/opencl.hpp"
#include "point.hpp"
#include "params.hpp"
#include "types.hpp"

struct ModelGPU {
    static const KeyValue kEmptySlot;
    static const std::pair<uint32_t, uint32_t> kEmptyPair;

    std::vector<KeyValue> m_hashmap;
    std::vector<std::pair<uint32_t, uint32_t>> m_keys;

    cl_context m_context = NULL;
    cl_device_id m_device = NULL;
    cl_command_queue m_queue = NULL;
    cl_program m_program = NULL;
    enum {
        KernelHashmap = 0,
        NumKernels
    };
    std::vector<cl_kernel> m_kernels;
    enum {
        BufferHashmap = 0,
        BufferPoints,
        NumBuffers,
    };
    std::vector<cl_mem> m_buffers;
    enum {
        NumImages = 0
    };
    std::vector<cl_mem> m_images;

    const std::pair<uint32_t, uint32_t> &key(size_t i) const {
        return m_keys[i];
    }
    void Execute(const std::vector<Point> &points);

    static ModelGPU Create(void);
    static void Destroy(ModelGPU &model);
};

#endif /* MODEL_GPU_H_ */
