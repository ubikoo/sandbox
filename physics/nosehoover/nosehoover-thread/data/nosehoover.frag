#version 330 core

uniform float u_width;
uniform float u_height;
uniform sampler2D u_texsampler;

in vec4 v_canvas_normal;
in vec4 v_canvas_color;
in vec2 v_canvas_texcoord;

out vec4 frag_color;

/*
 * fragment shader main
 */
void main(void)
{
    frag_color = texture(u_texsampler, v_canvas_texcoord);
}
