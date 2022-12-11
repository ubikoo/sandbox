#version 330 core

uniform float u_width;
uniform float u_height;
uniform sampler2D u_texsampler;

in vec4 vert_quad_normal;
in vec4 vert_quad_color;
in vec2 vert_quad_texcoord;

out vec4 frag_color;

/*
 * fragment shader main
 */
void main(void)
{
    float gamma = 1.5;
    frag_color = texture(u_texsampler, vert_quad_texcoord);
    frag_color.r = pow(frag_color.r, gamma);
    frag_color.g = pow(frag_color.g, gamma);
    frag_color.b = pow(frag_color.b, gamma);
}
