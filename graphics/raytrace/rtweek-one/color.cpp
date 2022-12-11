/*
 * color.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "color.hpp"
using namespace atto;

/**
 * Color::Black
 * @brief Static black colour.
 */
const Color Color::Black = Color{0.0, 0.0, 0.0};
const Color Color::White = Color{1.0, 1.0, 1.0};
const Color Color::Red   = Color{1.0, 0.0, 0.0};
const Color Color::Green = Color{0.0, 1.0, 0.0};
const Color Color::Blue  = Color{0.0, 0.0, 1.0};
