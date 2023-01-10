/**
 * camera.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include "ito/opengl.hpp"

namespace common {

struct Camera {
    ito::math::vec3f eye;       /* camera position */
    ito::math::vec3f look;      /* look direction */
    ito::math::vec3f up;        /* camera position */

    void move(float step);
    void strafe(float step);

    void pitch(float angle);
    void yaw(float angle);

    void update(const math::mat4f &rot);

    ito::math::mat4f view(void);

    static Camera Create(
        const ito::math::vec3f &eye,
        const ito::math::vec3f &ctr,
        const ito::math::vec3f &up);
};

} /* common */

#endif /* CAMERA_H_ */
