#version 330 core

layout (location = 0) in vec3 quad_position;
layout (location = 1) in vec3 quad_normal;
layout (location = 2) in vec3 quad_color;
layout (location = 3) in vec2 quad_texcoord;

out vec3 vert_quad_position;
out vec3 vert_quad_normal;
out vec3 vert_quad_color;
out vec2 vert_quad_texcoord;

/*
 * vertex shader main
 */
void main(void)
{
    vert_quad_position = quad_position;
    vert_quad_normal = quad_normal;
    vert_quad_color = quad_color;
    vert_quad_texcoord = quad_texcoord;

    gl_Position = vec4(quad_position, 1.0);
}
