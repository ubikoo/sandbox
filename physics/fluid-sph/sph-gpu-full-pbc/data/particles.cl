
/** ---------------------------------------------------------------------------
 * update_boundaries
 * @brief Apply boundary conditions and update particle positions.
 */
__kernel void update_boundaries(
    const uint n_particles,
    __global Particle_t *particles,
    const __global Domain_t *domain)
{
    const uint id = get_global_id(0);
    if (id < n_particles) {
        particles[id].pos = compute_pbc(particles[id].pos, domain);
    }
}

/** ---------------------------------------------------------------------------
 * copy_vertex_data
 * @brief Copy the particle positions onto the vertex buffer object.
 */
__kernel void copy_vertex_data(
    const uint n_particles,
    const float max_density,
    __global float *vertex,
    const __global Particle_t *particles)
{
    const uint id = get_global_id(0);
    if (id < n_particles) {
        float r = 0.0;
        float g = 0.0;
        float b = 0.0;

        float dens = particles[id].dens;
        b = dens * (1.0f + dens / (max_density * max_density)) / (1.0f + dens);

        vertex[6*id + 0] = particles[id].pos.x;
        vertex[6*id + 1] = particles[id].pos.y;
        vertex[6*id + 2] = particles[id].pos.z;
        vertex[6*id + 3] = r;
        vertex[6*id + 4] = g;
        vertex[6*id + 5] = b;
    }
}
