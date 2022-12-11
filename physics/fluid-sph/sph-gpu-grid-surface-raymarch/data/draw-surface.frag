#version 330 core

uniform float u_width;
uniform float u_height;
uniform uint u_compute_normal;
uniform sampler2D u_tex_depth;

in vec2 vert_quad_texcoord;
out vec4 frag_color;

/*
 * Return the linearized depth at the specified uv-position
 */
float compute_depth(vec2 uv)
{
    return texture(u_tex_depth, uv).r;
}

/*
 * Return the normal at the specified uv-position.
 */
vec3 compute_normal(vec2 uv)
{
    float dx = 0.5 / u_width;
    float dy = 0.5 / u_height;

    vec2 xlo = uv + vec2(-dx, 0.0f);
    vec2 xhi = uv + vec2( dx, 0.0f);
    vec2 ylo = uv + vec2(0.0f, -dy);
    vec2 yhi = uv + vec2(0.0f,  dy);

    float depth_xlo = compute_depth(xlo);
    float depth_xhi = compute_depth(xhi);
    float depth_ylo = compute_depth(ylo);
    float depth_yhi = compute_depth(yhi);

    float z_xlo = -depth_xlo;
    float z_xhi = -depth_xhi;
    float z_ylo = -depth_ylo;
    float z_yhi = -depth_yhi;

    vec3 delta_x = vec3(xhi - xlo, z_xhi - z_xlo);
    vec3 delta_y = vec3(yhi - ylo, z_yhi - z_ylo);
    vec3 normal = normalize(cross(delta_x, delta_y));

    return normal;
}

/*
 * fragment shader main
 */
void main(void)
{
    vec3 normal = compute_normal(vert_quad_texcoord);
    if (u_compute_normal == 0U) {
        frag_color = texture(u_tex_depth, vert_quad_texcoord);
    } else {
        frag_color = vec4(0.5*normal + vec3(0.5), 1.0);
    }
}
