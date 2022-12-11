/*
 * ray.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef RAYTRACE_CPU_WEEK_2_COLOR_H_
#define RAYTRACE_CPU_WEEK_2_COLOR_H_

#include <cfloat>
#include "base.hpp"

/**
 * Color
 * @brief Color represents a rgb tuple where the red, green, and blue values
 * are unbounded [0, DBL_MAX].
 */
struct Color {
    /* Member variables/accessors. */
    union {
        double m_data[3] core_aligned(32);
        struct { double r, g, b; };
    };

    /** Black Colour. */
    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;

    /* Element access. */
    double *data(void) { return &m_data[0]; }
    const double *data(void) const { return &m_data[0]; }

    /* Return a map of this color using a clamp operator. */
    Color clamp(const double lo = 0.0, const double hi = 1.0) const;

    /* Unary arithmetic vector operators. */
    Color &operator+=(const Color &other);
    Color &operator-=(const Color &other);
    Color &operator*=(const Color &other);
    Color &operator/=(const Color &other);

    /* Unary arithmetic scalar operators. */
    Color &operator+=(const double scalar);
    Color &operator-=(const double scalar);
    Color &operator*=(const double scalar);
    Color &operator/=(const double scalar);

    /* Unary plus/negation operators. */
    Color operator+(void) const;
    Color operator-(void) const;

    /* Constructor/destructor. */
    Color() = default;
    explicit Color(const double c) {
        m_data[0] = c;
        m_data[1] = c;
        m_data[2] = c;
    }
    explicit Color(const double r, const double g, const double b) {
        m_data[0] = r;
        m_data[1] = g;
        m_data[2] = b;
    }
    ~Color() = default;
}; /* Color */

/** ---------------------------------------------------------------------------
 * Color::clamp
 * @brief Return a map of this color using a clamp operator.
 */
core_inline
Color Color::clamp(const double lo, const double hi) const
{
    return Color(
        std::min(std::max(m_data[0], lo), hi),
        std::min(std::max(m_data[1], lo), hi),
        std::min(std::max(m_data[2], lo), hi));
}

/** ---------------------------------------------------------------------------
 * @brief Unary arithmetic vector operators.
 */
core_inline
Color &Color::operator+=(const Color &other)
{
    m_data[0] += other.m_data[0];
    m_data[1] += other.m_data[1];
    m_data[2] += other.m_data[2];
    return *this;
}

core_inline
Color &Color::operator-=(const Color &other)
{
    m_data[0] -= other.m_data[0];
    m_data[1] -= other.m_data[1];
    m_data[2] -= other.m_data[2];
    return *this;
}

core_inline
Color &Color::operator*=(const Color &other)
{
    m_data[0] *= other.m_data[0];
    m_data[1] *= other.m_data[1];
    m_data[2] *= other.m_data[2];
    return *this;
}

core_inline
Color &Color::operator/=(const Color &other)
{
    m_data[0] /= other.m_data[0];
    m_data[1] /= other.m_data[1];
    m_data[2] /= other.m_data[2];
    return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Unary arithmetic scalar operators.
 */
core_inline
Color &Color::operator+=(const double scalar)
{
    m_data[0] += scalar;
    m_data[1] += scalar;
    m_data[2] += scalar;
    return *this;
}

core_inline
Color &Color::operator-=(const double scalar)
{
    m_data[0] -= scalar;
    m_data[1] -= scalar;
    m_data[2] -= scalar;
    return *this;
}

core_inline
Color &Color::operator*=(const double scalar)
{
    m_data[0] *= scalar;
    m_data[1] *= scalar;
    m_data[2] *= scalar;
    return *this;
}

core_inline
Color &Color::operator/=(const double scalar)
{
    m_data[0] /= scalar;
    m_data[1] /= scalar;
    m_data[2] /= scalar;
    return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Unary plus/negation operators.
 */
core_inline
Color Color::operator+(void) const
{
    Color result(*this);
    return result;
}

core_inline
Color Color::operator-(void) const
{
    Color result(*this);
    result *= (double) (-1);
    return result;
}

/** ---------------------------------------------------------------------------
 * @brief Binary arithmetic operators between two vectors.
 */
core_inline
Color operator+(Color lhs, const Color &rhs)
{
    lhs += rhs;
    return lhs;
}

core_inline
Color operator-(Color lhs, const Color &rhs)
{
    lhs -= rhs;
    return lhs;
}

core_inline
Color operator*(Color lhs, const Color &rhs)
{
    lhs *= rhs;
    return lhs;
}

core_inline
Color operator/(Color lhs, const Color &rhs)
{
    lhs /= rhs;
    return lhs;
}

/** ---------------------------------------------------------------------------
 * @brief Binary arithmetic operators between a vector and a scalar.
 */
core_inline
Color operator+(Color lhs, const double scalar)
{
    lhs += scalar;
    return lhs;
}

core_inline
Color operator-(Color lhs, const double scalar)
{
    lhs -= scalar;
    return lhs;
}

core_inline
Color operator*(Color lhs, const double scalar)
{
    lhs *= scalar;
    return lhs;
}

core_inline
Color operator/(Color lhs, const double scalar)
{
    lhs /= scalar;
    return lhs;
}

/** ---------------------------------------------------------------------------
 * @brief Binary arithmetic operators between a scalar and a vector.
 * Division is not commutative, so its not implemented.
 */
core_inline
Color operator+(const double scalar, Color rhs)
{
    rhs += scalar;
    return rhs;
}

core_inline
Color operator-(const double scalar, Color rhs)
{
    rhs -= scalar;
    return rhs;
}

core_inline
Color operator*(const double scalar, Color rhs)
{
    rhs *= scalar;
    return rhs;
}

core_inline
Color operator/(const double scalar, Color rhs)
{
    rhs /= scalar;
    return rhs;
}

#endif /* RAYTRACE_CPU_WEEK_2_COLOR_H_ */
