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

#include "atto/opencl/opencl.hpp"
#include "engine.hpp"
#include "engine.hpp"
using namespace atto;

/**
 * main test client
 */
int main(int argc, char const *argv[])
{
    /*
     * Setup renderer OpenGL context and initialize the GLFW library.
     */
    gl::Renderer::init(
        Params::window_width,
        Params::window_height,
        Params::window_title);
    gl::Renderer::enable_event(
        gl::Event::FramebufferSize |
        gl::Event::WindowClose     |
        gl::Event::Key);

    /*
     * Render loop:
     *  poll and handle events
     *  update, draw and swap buffers
     */
    Engine engine;
    gl::Timer timer;
    while (gl::Renderer::is_open()) {
        /* Poll events and handle. */
        gl::Renderer::poll_event(Params::poll_timeout);
        while (gl::Renderer::has_event()) {
            gl::Event event = gl::Renderer::pop_event();

            if (event.type == gl::Event::FramebufferSize) {
                int w = event.framebuffersize.width;
                int h = event.framebuffersize.height;
                gl::Renderer::viewport({0, 0, w, h});
            }

            if ((event.type == gl::Event::WindowClose) ||
                (event.type == gl::Event::Key &&
                 event.key.code == GLFW_KEY_ESCAPE)) {
                gl::Renderer::close();
            }

            /* Handle the event. */
            engine.handle(event);
        }

        /* Execute the engine and draw. */
        {
            engine.execute();
            gl::Renderer::clear(0.5f, 0.5f, 0.5f, 1.0f, 1.0f);
            engine.draw();
            gl::Renderer::display();

            if (timer.next()) {
                glfwSetWindowTitle(gl::Renderer::window(),
                    timer.to_string().c_str());
                timer.reset();
            }
        }
    }

    exit(EXIT_SUCCESS);
}

