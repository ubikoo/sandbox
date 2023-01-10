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

#include <vector>
#include <utility>
#include <chrono>
#include "ito/opengl.hpp"
#include "ito/opencl.hpp"
#include "point.hpp"
#include "model-cpu.hpp"
#include "model-gpu.hpp"
#include "model.hpp"

using namespace ito;

/**
 * @brief Create a new model and associated objects.
 */
Model Model::Create()
{
    Model model;
    model.m_points.resize(Params::kNumPoints, {});
    model.m_cpu = ModelCPU::Create();
    model.m_gpu = ModelGPU::Create();
    return model;
}

/**
 * @brief Destroy the model and associated objects.
 */
void Model::Destroy(Model &model)
{
    ModelCPU::Destroy(model.m_cpu);
    ModelGPU::Destroy(model.m_gpu);
}


/**
 * @brief Handle the event in the model.
 */
void Model::Handle(glfw::Event &event)
{}

/**
 * @brief Update the model.
 */
void Model::Update()
{
    size_t n = 0;
    for (auto &p : common::CreateBox(
        Params::kNumPoints,
        Params::kDomainLo.s[0],
        Params::kDomainLo.s[1],
        Params::kDomainLo.s[2],
        Params::kDomainHi.s[0],
        Params::kDomainHi.s[1],
        Params::kDomainHi.s[2])) {
        m_points[n++] = Point{
            (cl_float3) {p.x, p.y, p.z},
            (cl_float3) {0.0, 0.0, 0.0}};
    }

    {
        auto tic = std::chrono::high_resolution_clock::now();
        m_cpu.Execute(m_points);
        auto toc = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double,std::ratio<1,1000>> msec = toc-tic;
        std::printf("CPU time %lf\n", msec.count());
    }
    {
        auto tic = std::chrono::high_resolution_clock::now();
        m_gpu.Execute(m_points);
        auto toc = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double,std::ratio<1,1000>> msec = toc-tic;
        std::printf("GPU time %lf\n", msec.count());
    }

    for (size_t i = 0; i < Params::kNumPoints; i++) {
        if (m_cpu.key(i).first != m_gpu.key(i).first ||
            m_cpu.key(i).second != m_gpu.key(i).second) {
            std::cerr << ito::str::format(
                "%lu:"
                " CPU %10u %10u ,"
                " GPU %10u %10u ,"
                " %8.4f %8.4f %8.4f\n",
                i,
                m_cpu.key(i).first,
                m_cpu.key(i).second,
                m_gpu.key(i).first,
                m_gpu.key(i).second,
                m_points[i].pos.s[0],
                m_points[i].pos.s[1],
                m_points[i].pos.s[2]);
            ito_throw("FAIL");
        }
    }
}

/**
 * @brief Render the model.
 */
void Model::Render(void)
{
    GLFWwindow *window = glfw::Window();
    if (window == nullptr) {
        return;
    }
}
