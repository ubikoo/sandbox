/**
 * camera.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "ito/opengl.hpp"
using namespace ito;
#include "camera.hpp"

/**
 * @brief Move the camera forward and backward along its look vector.
 */
void Camera::Move(float step)
{
    eye += look * step;
}

/**
 * @brief Strafe the camera right and left along its right vector.
 */
void Camera::Strafe(float step)
{
    math::vec3f right = math::normalize(math::cross(look, up));
    eye += right * step;
}

/**
 * @brief Rotate camera up and down around its right vector.
 */
void Camera::Pitch(float angle)
{
    math::vec3f right = math::normalize(math::cross(look, up));
    Rotate(math::rotate(right, angle));
}

/**
 * @brief Rotate camera left and right around its up vector.
 */
void Camera::Yaw(float angle)
{
    math::vec3f right = math::normalize(math::cross(look, up));
    math::vec3f updir = math::normalize(math::cross(right, look));
    Rotate(math::rotate(updir, angle));
}

/**
 * @brief Rotate the camera look direction.
 */
void Camera::Rotate(const math::mat4f &rot)
{
    math::vec4f d = {look.x, look.y, look.z, 0.0f};
    d = math::dot(rot, d);
    look = math::vec3f{d.x, d.y, d.z};
}

/**
 * @brief Return the camera view transform.
 */
math::mat4f Camera::View(void)
{
    return math::lookat(eye, eye + look, up);
}

/**
 * @brief Return the camera view transform.
 */
Camera Camera::Create(
    const math::vec3f &eye,
    const math::vec3f &ctr,
    const math::vec3f &up)
{
    Camera camera = {};
    camera.eye = eye;
    camera.look = math::normalize(ctr - eye);
    camera.up = up;
    return camera;
}
