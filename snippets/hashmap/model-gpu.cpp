/*
 * model-gpu.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include <vector>
#include <utility>
#include <algorithm>
#include "ito/opengl.hpp"
#include "ito/opencl.hpp"
#include "point.hpp"
#include "params.hpp"
#include "types.hpp"
#include "model-gpu.hpp"

using namespace ito;

/**
 * @brief Static constants.
 */
const KeyValue ModelGPU::kEmptySlot = {Params::kEmpty, Params::kEmpty};
const std::pair<uint32_t, uint32_t> ModelGPU::kEmptyPair = {0, 0};

/**
 * @brief Create a Hashmap model executing on the GPU.
 */
ModelGPU ModelGPU::Create()
{
    ModelGPU model;

    /*
     * Model data.
     */
    {
        model.m_hashmap.resize(Params::kCapacity, kEmptySlot);
        model.m_keys.resize(Params::kNumPoints, kEmptyPair);
    }

    /*
     * OpenCL data.
     */
    {
        /* OpenCL context, device and queue data. */
        model.m_context = clfw::Context();
        model.m_device = clfw::Device();
        model.m_queue = clfw::Queue();
        std::cout << cl::GetDeviceInfoString(model.m_device) << "\n";

        /* Create the program object. */
        model.m_program = cl::CreateProgramFromFile(
            model.m_context, "data/hashmap.cl");
        cl::BuildProgram(model.m_program, model.m_device, "");
        std::cout << cl::GetProgramSource(model.m_program) << "\n";

        /* Create the hashmap kernel in the program object. */
        model.m_kernels.resize(ModelGPU::NumKernels, NULL);
        model.m_kernels[ModelGPU::KernelHashmap] = cl::CreateKernel(
            model.m_program, "hashmap_insert");

        /* Create memory buffers. */
        model.m_buffers.resize(NumBuffers, NULL);

        model.m_buffers[ModelGPU::BufferHashmap] = cl::CreateBuffer(
            model.m_context,
            CL_MEM_READ_WRITE,
            Params::kCapacity * sizeof(KeyValue),
            (void *) NULL);

        model.m_buffers[BufferPoints] = cl::CreateBuffer(
            model.m_context,
            CL_MEM_READ_ONLY,
            Params::kNumPoints * sizeof(Point),
            (void *) NULL);
    }

    return model;
}

/**
 * @brief Destroy the Hashmap model.
 */
void ModelGPU::Destroy(ModelGPU &model)
{
    /* OpenCL data. */
    {
        for (auto &it : model.m_images) {
            cl::ReleaseMemObject(it);
        }
        for (auto &it : model.m_buffers) {
            cl::ReleaseMemObject(it);
        }
        for (auto &it : model.m_kernels) {
            cl::ReleaseKernel(it);
        }
        cl::ReleaseProgram(model.m_program);
    }
}

/**
 * @brief Compute the hashmap on the GPU.
 */
void ModelGPU::Execute(const std::vector<Point> &points)
{
    /*
     * Update the hashmap data on the gpu.
     */
    {
        std::fill(m_hashmap.begin(), m_hashmap.end(), kEmptySlot);

        cl::EnqueueWriteBuffer(
            m_queue,
            m_buffers[BufferHashmap],
            CL_TRUE,
            0,
            Params::kCapacity * sizeof(KeyValue),
            (void *) &m_hashmap[0],
            NULL,
            NULL);

        cl::EnqueueWriteBuffer(
            m_queue,
            m_buffers[BufferPoints],
            CL_TRUE,
            0,
            Params::kNumPoints * sizeof(Point),
            (void *) &points[0],
            NULL,
            NULL);
    }

    /*
     * Enqueue hashmap insert kernel.
     */
    {
        /* Set kernel arguments. */
        cl::SetKernelArg(m_kernels[KernelHashmap], 0, sizeof(cl_mem), &m_buffers[BufferHashmap]);
        cl::SetKernelArg(m_kernels[KernelHashmap], 1, sizeof(cl_mem), &m_buffers[BufferPoints]);
        cl::SetKernelArg(m_kernels[KernelHashmap], 2, sizeof(cl_uint), &Params::kCapacity);
        cl::SetKernelArg(m_kernels[KernelHashmap], 3, sizeof(cl_uint), &Params::kNumPoints);
        cl::SetKernelArg(m_kernels[KernelHashmap], 4, sizeof(cl_uint), &Params::kNumCells);
        cl::SetKernelArg(m_kernels[KernelHashmap], 5, sizeof(cl_float3), &Params::kDomainLo);
        cl::SetKernelArg(m_kernels[KernelHashmap], 6, sizeof(cl_float3), &Params::kDomainHi);

        /* Set the size of the NDRange workgroups */
        cl::NDRange global_ws = cl::NDRange::Make(cl::NDRange::Roundup(
            Params::kNumPoints, Params::kWorkGroupSize));
        cl::NDRange local_ws = cl::NDRange::Make(Params::kWorkGroupSize);

        /* Run the kernel */
        cl::EnqueueNDRangeKernel(
            m_queue,
            m_kernels[KernelHashmap],
            cl::NDRange::Null,
            global_ws,
            local_ws,
            NULL,
            NULL);

        /* Wait for kernel to compute */
        cl::Finish(m_queue);
    }

    /*
     * Read the hashmap data back to the host.
     */
    {
        cl::EnqueueReadBuffer(
            m_queue,
            m_buffers[BufferHashmap],
            CL_TRUE,
            0,
            Params::kCapacity * sizeof(KeyValue),
            (void *) &m_hashmap[0],
            NULL,
            NULL);
    }

    /*
     * Query the hashmap for all valid key-value pairs.
     */
    {
        std::fill(m_keys.begin(), m_keys.end(), kEmptyPair);
        for (auto &slot : m_hashmap) {
            if (slot.key != Params::kEmpty) {
                m_keys[slot.value] = std::make_pair(
                    slot.key, slot.key % Params::kCapacity);
            }
        }
    }
}
