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

#include "model.hpp"
using namespace atto;

/**
 * main test client
 */
int main(int argc, char const *argv[])
{
    /* Execute the model */
    Model model;
    for (size_t i = 0; i < Params::n_steps; i++) {
        model.execute();
    }

    exit(EXIT_SUCCESS);
}
