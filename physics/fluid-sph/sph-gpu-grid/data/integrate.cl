/** ---------------------------------------------------------------------------
 * begin_integrate
 * @brief Begin integration - first half of the integration step.
 */
__kernel void begin_integrate(
    const float t_step,
    const uint n_particles,
    __global Particle_t *particles)
{
    const uint id = get_global_id(0);
    if (id < n_particles) {
        /* Integrate momenta half time step. */
        float4 accel = particles[id].dens > 0.0f
            ? particles[id].force / particles[id].dens
            : (float4) (0.0f);
        particles[id].vel += 0.5f * t_step * accel;

        /* Integrate positions full time step. */
        particles[id].prev = particles[id].pos;
        particles[id].pos += t_step * particles[id].vel;
    }
}

/**
 * end_integrate
 * @brief End integration - second half of the integration step.
 */
__kernel void end_integrate(
    const float t_step,
    const uint n_particles,
    __global Particle_t *particles)
{
    const uint id = get_global_id(0);
    if (id < n_particles) {
        /* Integrate momenta half time step. */
        float4 accel = particles[id].dens > 0.0f
            ? particles[id].force / particles[id].dens
            : (float4) (0.0f);
        particles[id].vel += 0.5f * t_step * accel;
    }
}
