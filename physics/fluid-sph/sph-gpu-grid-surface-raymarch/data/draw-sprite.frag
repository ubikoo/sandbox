#version 330 core

uniform float u_scale;
uniform mat4 u_persp;
uniform uint u_replace_depth;

in vec2 vert_sprite_coord;
// out vec4 frag_color;

/* ----------------------------------------------------------------------------
 * Fragment shader main
 */
void main(void)
{
    /* Compute the normal to the sphere at the fragment position. */
    vec3 N;
    N.xy = 2.0 * vert_sprite_coord - 1.0;
    float r2 = dot(N.xy, N.xy);
    if (r2 > 1.0) {
        discard;                // discard fragments outside the sprite disc.
    }
    N.z = sqrt(1.0 - r2);

    /* Replace the depth to rasterize the spheres. */
    float A = u_persp[2][2];
    float B = u_persp[3][2];

    float z_ndc = 2.0 * gl_FragCoord.z - 1.0;
    float z_eye = -B / (A + z_ndc);

    z_eye += u_scale * N.z;
    z_ndc = -(A + B / z_eye);
    float depth = 0.5 * (z_ndc + 1.0);

    if (u_replace_depth == 0U) {
        gl_FragDepth = gl_FragCoord.z;
    } else {
        gl_FragDepth = depth;
    }

    /* Shade the fragment using a simple directional light. */
    // const vec3 light_dir = normalize(vec3(0.5, 0.5, 1.5));
    // float diffuse = max(0.0, dot(light_dir, N));
    // float alpha = 1.0;
    // vec3 color = vec3(0.0, 0.0, 1.0) * diffuse;
    // frag_color = vec4(color, alpha);
}
