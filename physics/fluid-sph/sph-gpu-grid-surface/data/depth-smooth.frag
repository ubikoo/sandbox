#version 330 core

uniform float u_width;
uniform float u_height;
uniform sampler2D u_tex_depth;

in vec2 vert_quad_texcoord;
out vec4 frag_depth;

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
 * Return the curvature at the specified uv-position.
 */
float compute_curvature(vec2 uv)
{
    float dx = 0.4 / u_width;
    float dy = 0.4 / u_height;

    vec3 normal   = compute_normal(uv);
    vec3 normal_x = compute_normal(uv + vec2(dx, 0.0f));
    vec3 normal_y = compute_normal(uv + vec2(0.0f, dy));

    float dndx = (normal_x.x - normal.x) / dx;
    float dndy = (normal_y.y - normal.y) / dy;

    return 0.5*(dndx + dndy);
}

/*
 * fragment shader main
 */
void main(void)
{
    // ------------------------------------------------------------------------
    // frag_depth = texture(u_tex_depth, vert_quad_texcoord);

    // ------------------------------------------------------------------------
    // float curv = compute_curvature(vert_quad_texcoord);
    // float depth = compute_depth(vert_quad_texcoord);
    // depth -= 0.1*curv;
    // frag_depth = vec4(depth, depth, depth, 1.0);

    // ------------------------------------------------------------------------
    // float dx = 2.0 / u_width;
    // float dy = 2.0 / u_height;

    // vec2 xlo = vert_quad_texcoord + vec2(-dx, 0.0f);
    // vec2 xhi = vert_quad_texcoord + vec2( dx, 0.0f);
    // vec2 ylo = vert_quad_texcoord + vec2(0.0f, -dy);
    // vec2 yhi = vert_quad_texcoord + vec2(0.0f,  dy);

    // float depth_xlo = compute_depth(xlo);
    // float depth_xhi = compute_depth(xhi);
    // float depth_ylo = compute_depth(ylo);
    // float depth_yhi = compute_depth(yhi);

    // float depth = 0.25*(depth_xlo + depth_xhi + depth_ylo + depth_yhi);
    // frag_depth = vec4(depth, depth, depth, 1.0);





    // ------------------------------------------------------------------------
    vec2 radius = vec2(1.0);
    float max_radius = sqrt(radius.x * radius.x + radius.y * radius.y);
    float blur = 0.0;
    float sum = 0.0;
    for(float u = -radius.x; u <= radius.x; ++u) {
        for(float v = -radius.y; v <= radius.y; ++v) {
            float weight = max_radius - sqrt(u * u + v * v);
            vec2 uv = vert_quad_texcoord + vec2(u, v) / vec2(u_width, u_height);
            blur += weight * compute_depth(uv);
            sum += weight;
        }
    }
    blur /= sum;
    frag_depth = vec4(blur, blur, blur, 1.0);


    // ------------------------------------------------------------------------
    // float n_directions = 16.0;    // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    // float n_radius = 4.0;        // BLUR QUALITY (Default 4.0 - More is better but slower)

    // float pi = 6.28318530718;       // 2*pi
    // float del_pi = pi / n_directions;

    // vec2 radius = 0.5 / vec2(u_width, u_height);
    // vec2 del_radius = radius / n_radius;



    // vec2 uv = vert_quad_texcoord;
    // float depth = compute_depth(uv);
    // for (float d = 0.0; d < pi; d += del_pi) {
    //     for (float n = 1; n <= n_radius; ++n) {
    //         vec2 r = n * del_radius;
    //         depth += compute_depth(uv + r * vec2(cos(d), sin(d)));
    //     }
    // }

    // depth /= (n_radius * n_directions);
    // frag_depth = vec4(depth, depth, depth, 1.0);
}
