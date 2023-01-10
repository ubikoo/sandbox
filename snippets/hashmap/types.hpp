/*
 * types.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef TYPES_H_
#define TYPES_H_

struct Point {
    cl_float3 pos;
    cl_float3 col;
};

struct KeyValue {
    cl_uint key;
    cl_uint value;
};

#endif /* TYPES_H_ */
