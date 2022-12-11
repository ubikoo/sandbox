#version 330 core


layout (location = 0) in vec3 lattice_position;
layout (location = 1) in vec3 lattice_normal;
layout (location = 2) in vec3 lattice_color;
layout (location = 3) in vec2 lattice_texcoord;

out vec4 vert_lattice_normal;
out vec4 vert_lattice_color;
out vec2 vert_lattice_texcoord;

/*
 * vertex shader main
 */
void main(void)
{
    gl_Position = vec4(lattice_position, 1.0);
    vert_lattice_normal = vec4(lattice_normal, 1.0);
    vert_lattice_color = vec4(lattice_color, 1.0);
    vert_lattice_texcoord = lattice_texcoord;
}
