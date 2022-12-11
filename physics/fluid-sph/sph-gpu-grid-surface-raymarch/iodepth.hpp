/*
 * iodepth.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef SPH_IODEPTH_H_
#define SPH_IODEPTH_H_

/**
 * @brief IODepth maintains a framebuffer object using only the depth attachment.
 */
struct IODepth {
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
    explicit IODepth(const GLint width, const GLint height);
    ~IODepth();

    IODepth(const IODepth &) = delete;
    IODepth &operator=(const IODepth &) = delete;
};

#endif /* SPH_IODEPTH_H_ */
