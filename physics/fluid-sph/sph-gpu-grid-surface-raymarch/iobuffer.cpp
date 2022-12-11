/*
 * iobuffer.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "atto/opencl/opencl.hpp"
#include "iobuffer.hpp"
using namespace atto;

/**
 * @brief IOBuffer represents a framebuffer object with a specified size,
 * using a single depth attachment.
 */
IOBuffer::IOBuffer(const GLint width, const GLint height)
{
    m_width = width;
    m_height = height;
    m_fbo = gl::create_framebuffer_texture(
        m_width,
        m_height,
        1,                          /* num color attachments */
        GL_RGB32F,                  /* color buffer internal format */
        &m_texture);
}

/**
 * IOBuffer::~IOBuffer
 */
IOBuffer::~IOBuffer()
{
    glDeleteTextures(1, &m_texture);
    glDeleteFramebuffers(1, &m_fbo);
}

/**
 * IOBuffer::bind
 * @brief Bind the IOBuffer framebuffer as the render target.
 */
void IOBuffer::bind(void)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

/**
 * IOBuffer::unbind
 * @brief Unbind the IOBuffer framebuffer as the render target.
 */
void IOBuffer::unbind(void)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
