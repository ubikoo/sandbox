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

#ifndef TEST_ITO_OPENGL_CAMERA_H_
#define TEST_ITO_OPENGL_CAMERA_H_

#include "ito/opengl.hpp"

struct Camera {
    ito::math::vec3f eye;       /* camera position */
    ito::math::vec3f look;      /* look direction */
    ito::math::vec3f up;        /* camera position */

    void Move(float step);
    void Strafe(float step);
    void Pitch(float angle);
    void Yaw(float angle);
    void Rotate(const math::mat4f &rot);

    ito::math::mat4f View(void);

    static Camera Create(
        const ito::math::vec3f &eye,
        const ito::math::vec3f &ctr,
        const ito::math::vec3f &up);

};

#endif /* TEST_ITO_OPENGL_CAMERA_H_ */
