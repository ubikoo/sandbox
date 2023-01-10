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

#include <vector>
#include "ito/opencl.hpp"
using namespace ito;

/** ---------------------------------------------------------------------------
 * main
 */
int main(int argc, char const *argv[])
{
    std::vector<cl_platform_id> platforms = cl::GetPlatformIDs();

    for (auto &platform_it : platforms) {
        std::cout << cl::GetPlatformInfoString(platform_it) << "\n";

        /* Platform CPU devices */
        {
            std::vector<cl_device_id> devices = cl::GetDeviceIDs(platform_it,
                CL_DEVICE_TYPE_CPU);

            size_t count = 0;
            for (auto &device_it : devices) {
                std::cout << "CPU DEVICE: " << count++ << "\n";
                std::cout << cl::GetDeviceInfoString(device_it) << "\n";
            }

            for (auto &device_it : devices) {
                cl::ReleaseDevice(device_it);
            }
        }

        /* Platform GPU devices */
        {
            std::vector<cl_device_id> devices = cl::GetDeviceIDs(platform_it,
                CL_DEVICE_TYPE_GPU);

            size_t count = 0;
            for (auto &device_it : devices) {
                std::cout << "GPU DEVICE: " << count++ << "\n";
                std::cout << cl::GetDeviceInfoString(device_it) << "\n";
            }

            for (auto &device_it : devices) {
                cl::ReleaseDevice(device_it);
            }
        }
   }

    exit(EXIT_SUCCESS);
}