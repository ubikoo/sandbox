/*
 * params.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef PARAMS_H_
#define PARAMS_H_

namespace Params {

/* OpenGL parameters */
static const int kWidth      = 800;
static const int kHeight     = 800;
static const char kTitle[]   = "Empty";
static const double kTimeout = 0.01;

/* Model parameters */
static const cl_ulong kNumPoints = 1048576;

/* OpenCL parameters */
static const cl_ulong kDeviceIndex = 2;
static const cl_ulong kWorkGroupSize = 256;
static const cl_ulong kNumWorkItems = ito::cl::NDRange::Roundup(
    kNumPoints, kWorkGroupSize);
static const cl_ulong kNumWorkGroups = kNumWorkItems / kWorkGroupSize;

} /* Params */

#endif /* PARAMS_H_ */
