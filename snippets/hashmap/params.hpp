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
static const cl_uint kNumSteps = 10;
static const cl_uint kLoadFactor = 2;
static const cl_uint kNumPoints = 1048576;
static const cl_uint kCapacity = kLoadFactor * kNumPoints;
static const cl_uint kNumCells = 16;
static const cl_uint kEmpty = 0xffffffff;
static const cl_float3 kDomainLo = {-1.0f, -1.0f, -1.0f};
static const cl_float3 kDomainHi = { 1.0f,  1.0f,  1.0f};

/* OpenCL parameters */
static const cl_ulong kDeviceIndex = 2;
static const cl_ulong kWorkGroupSize = 256;
// static const cl_ulong kNumWorkItems = ito::cl::NDRange::Roundup(
//     kNumPoints, kWorkGroupSize);
// static const cl_ulong kNumWorkGroups = kNumWorkItems / kWorkGroupSize;

} /* Params */

#endif /* PARAMS_H_ */
