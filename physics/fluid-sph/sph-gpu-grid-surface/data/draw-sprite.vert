#version 330 core

uniform float u_scale;
uniform mat4 u_view;
uniform mat4 u_persp;

layout (location = 0) in vec2 sprite_coord;
layout (location = 1) in vec3 point_pos;
out vec2 vert_sprite_coord;

/*
 * vertex shader main
 */
void main(void)
{
    /* Pass through point position and sprite uv-coordinates. */
    vert_sprite_coord = sprite_coord;

    /* Compute the vertex position from the sprite and point positions. */
    vec3 pos_local = u_scale * vec3(2.0 * sprite_coord - 1.0, 0.0);
    vec4 pos_world = vec4(point_pos, 1.0) + inverse(u_view) * vec4(pos_local, 1.0);
    gl_Position = u_persp * (u_view * pos_world);
}
