#version 330 core

uniform float u_znear;
uniform float u_zfar;
uniform sampler2D u_tex_depth;

in vec2 vert_quad_texcoord;
out vec4 frag_depth;

/*
 * Return the linearized depth at the specified uv-position
 */
float compute_depth(vec2 uv)
{
    float A = -(u_zfar + u_znear) / (u_zfar - u_znear);
    float B = -2.0 * u_zfar * u_znear / (u_zfar - u_znear);

    float depth = texture(u_tex_depth, uv).r;
    float z_ndc = 2.0 * depth - 1.0;
    float z_eye = B / (A + z_ndc);
    return (z_eye - u_znear) / (u_zfar - u_znear);
    // return (2.0 * u_znear) / (u_zfar + u_znear - depth * (u_zfar - u_znear));
}

/*
 * fragment shader main
 */
void main(void)
{
    float depth = compute_depth(vert_quad_texcoord);
    frag_depth = vec4(depth, depth, depth, 1.0);
}
