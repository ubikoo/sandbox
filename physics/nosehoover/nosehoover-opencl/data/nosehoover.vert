#version 330 core

layout (location = 0) in vec3 canvas_position;
layout (location = 1) in vec3 canvas_normal;
layout (location = 2) in vec3 canvas_color;
layout (location = 3) in vec2 canvas_texcoord;

out vec4 v_canvas_normal;
out vec4 v_canvas_color;
out vec2 v_canvas_texcoord;

/*
 * vertex shader main
 */
void main(void)
{
    gl_Position = vec4(canvas_position, 1.0);
    v_canvas_normal = vec4(canvas_normal, 1.0);
    v_canvas_color = vec4(canvas_color, 1.0);
    v_canvas_texcoord = canvas_texcoord;
}
