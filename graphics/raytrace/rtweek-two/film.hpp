/*
 * film.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef RAYTRACE_CPU_WEEK_2_FILM_H_
#define RAYTRACE_CPU_WEEK_2_FILM_H_

#include "atto/opencl/opencl.hpp"
#include "color.hpp"

/**
 * Film
 * @brief Film maintains an array of pixels with a specified width and height.
 */
struct Film {
    /* Film member data. */
    uint32_t m_width;
    uint32_t m_height;
    std::vector<Color> m_pixels;

    /* Return the film width and height. */
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    /* Return a reference to the film pixels. */
    std::vector<Color> &pixels(void) { return m_pixels; }
    const std::vector<Color> &pixels(void) const { return m_pixels; }

    /* Clear the film pixels. */
    void clear(void);

    /* Set the film pixel to the specified color. */
    void set(const uint32_t x, const uint32_t y, const Color &color);

    /* Add the specified color the film pixel. */
    void add(const uint32_t x, const uint32_t y, const Color &color);

    /* Get the specified color of the film pixel. */
    const Color &get(const uint32_t x, const uint32_t y) const;

   /* Sample the normalized coordinates of a point in the film pixel. */
    atto::math::vec2d sample(
        const uint32_t x,
        const uint32_t y,
        const atto::math::vec2d &u) const;

    /* Film ctor/dtor. */
    Film(const uint32_t width, const uint32_t height);
    ~Film() = default;
};

#endif /* RAYTRACE_CPU_WEEK_2_FILM_H_ */
