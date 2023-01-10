/*
 * main.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "ito/opengl.hpp"
#include "ito/opencl.hpp"
#include "params.hpp"
#include "model.hpp"

using namespace ito;

/** ---------------------------------------------------------------------------
 * @brief Constants and globals.
 */
Model gModel;
size_t gStep = 0;

/** ---------------------------------------------------------------------------
 * @brief Handle events.
 */
static void Handle(void)
{
    /* Poll events and handle. */
    glfw::PollEvent(Params::kTimeout);
    while (glfw::HasEvent()) {
        glfw::Event event = glfw::PopEvent();

        if (event.type == glfw::Event::FramebufferSize) {
            int w = event.framebuffersize.width;
            int h = event.framebuffersize.height;
            glfw::SetViewport({0, 0, w, h});
        }

        if ((event.type == glfw::Event::WindowClose) ||
            (event.type == glfw::Event::Key &&
             event.key.code == GLFW_KEY_ESCAPE)) {
            glfw::Close();
        }

        gModel.Handle(event);
    }
}

/** ---------------------------------------------------------------------------
 * @brief Update state.
 */
static void Update(void)
{
    std::cout << "Step " << gStep << "\n";
    if (gStep++ == Params::kNumSteps) {
        glfw::Close();
    }
    gModel.Update();
}

/** ---------------------------------------------------------------------------
 * @brief Draw and swap buffers.
 */
static void Render(void)
{
    glfw::ClearBuffers(0.5f, 0.5f, 0.5f, 1.0f, 1.0f);
    gModel.Render();
    glfw::SwapBuffers();
}

/** ---------------------------------------------------------------------------
 * @brief empty model client
 */
int main(int argc, char const *argv[])
{
    /* Initalize GLFW library and OpenGL context. */
    glfw::Init(Params::kWidth, Params::kHeight, Params::kTitle);
    glfw::EnableEvent(
        glfw::Event::FramebufferSize |
        glfw::Event::WindowClose     |
        glfw::Event::Key);

    /* Initialize OpenCL context on the specified device. */
    clfw::Init(CL_DEVICE_TYPE_GPU, Params::kDeviceIndex);

    /* Create the map object. */
    gModel = Model::Create();

    /* Render loop: handle events, update state, and render. */
    while (glfw::IsOpen()) {
        Handle();
        Update();
        Render();
    }

    /* Create the map object. */
    Model::Destroy(gModel);

    /* Initialize OpenCL context on the specified device. */
    clfw::Terminate();

    /* Initalize GLFW library and create OpenGL context. */
    glfw::Terminate();

    exit(EXIT_SUCCESS);
}
