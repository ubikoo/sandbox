/*
 * iobuffer.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef SPH_IOBUFFER_H_
#define SPH_IOBUFFER_H_

/**
 * @brief IOBuffer maintains a framebuffer object using a color attachment.
 */
struct IOBuffer {
    GLint m_width;
    GLint m_height;
    GLuint m_fbo;
    GLuint m_texture;

    const GLint &width(void) const { return m_width; }
    const GLint &height(void) const { return m_height; }
    const GLuint &fbo(void) const { return m_fbo; }
    const GLuint &texture(void) const { return m_texture; }

    /* Bind the framebuffer for reading/writing */
    void bind(void);
    void unbind(void);

    /* Constructor/destructor. */
    explicit IOBuffer(const GLint width, const GLint height);
    ~IOBuffer();

    IOBuffer(const IOBuffer &) = delete;
    IOBuffer &operator=(const IOBuffer &) = delete;
};

#endif /* SPH_IOBUFFER_H_ */
