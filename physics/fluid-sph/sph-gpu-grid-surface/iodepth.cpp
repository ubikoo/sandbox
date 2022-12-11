/*
 * iodepth.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "atto/opencl/opencl.hpp"
#include "iodepth.hpp"
using namespace atto;

/**
 * @brief IODepth represents a framebuffer object with a specified size,
 * using a single depth attachment.
 */
IODepth::IODepth(const GLint width, const GLint height)
{
    m_width = width;
    m_height = height;
    m_fbo = gl::create_framebuffer_depth(
        m_width,
        m_height,
        GL_DEPTH_COMPONENT32F,
        &m_texture,
        GL_NEAREST,
        GL_NEAREST);
}

/**
 * IODepth::~IODepth
 */
IODepth::~IODepth()
{
    glDeleteTextures(1, &m_texture);
    glDeleteFramebuffers(1, &m_fbo);
}

/**
 * IODepth::bind
 * @brief Bind the IODepth framebuffer as the render target.
 */
void IODepth::bind(void)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

/**
 * IODepth::unbind
 * @brief Unbind the IODepth framebuffer as the render target.
 */
void IODepth::unbind(void)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
