
/** @brief Return the periodic image in primary cell of the fluid domain. */
float4 compute_pbc(const float4 pos, const __global Domain_t *domain);

/** ---------------------------------------------------------------------------
 * compute_density
 * @brief Compute particle density and corresponding pressure using EoS.
 */
__kernel void compute_density(
    const uint n_particles,
    const float kernel_radius,
    const float eos_kappa,
    const float eos_density,
    __global Particle_t *particles,
    const __global Domain_t *domain)
{
    const uint id_1 = get_global_id(0);
    if (id_1 >= n_particles) {
        return;
    }

    /*
     * Compute particle density from its pairwise interactions.
     */
    float4 pos_1 = particles[id_1].pos;

    float density = 0.0f;
    for (uint id_2 = 0; id_2 < n_particles; ++id_2) {
        float4 r_12 = pos_1 - particles[id_2].pos;
        r_12 = compute_pbc(r_12, domain);
        float w_12 = kernel_density(length(r_12), kernel_radius);
        density += particles[id_2].mass * w_12;
    }
    particles[id_1].dens = density;

    /*
     * Compute particle pressure using the corresponding EoS.
     */
    float rho = density - eos_density;
    particles[id_1].pres = eos_kappa * rho * rho * rho;
}

/** ---------------------------------------------------------------------------
 * compute_forces
 * @brief Compute particle forces using the previously computed density.
 */
__kernel void compute_forces(
    const uint n_particles,
    const float kernel_radius,
    const float viscosity,
    __global Particle_t *particles,
    const __global Domain_t *domain)
{
    const uint id_1 = get_global_id(0);
    if (id_1 >= n_particles) {
        return;
    }

    /*
     * Compute pairwise forces (pressure and viscosity) between the
     * specified particle and its neighbours in the adjacent cells.
     */
    float4 pos_1 = particles[id_1].pos;
    float4 vel_1 = particles[id_1].vel;
    float dens_1 = particles[id_1].dens;
    float pres_1 = particles[id_1].pres;

    float4 force = (float4) (0.0f);
    for (uint id_2 = 0; id_2 < n_particles; ++id_2) {
        float dens_2 = particles[id_2].dens;
        float pres_2 = particles[id_2].pres;

        float4 r_12 = pos_1 - particles[id_2].pos;
        r_12 = compute_pbc(r_12, domain);
        float length_12 = length(r_12);

        /* Pressure forces. */
        if (length_12 > 0.0f) {
            float w_12 = kernel_pressure(length_12, kernel_radius);
            float4 f_12 = w_12 * r_12 / length_12;
            f_12 *= pres_1 / (dens_1 * dens_1) +
                    pres_2 / (dens_2 * dens_2);
            f_12 *= particles[id_2].mass;
            f_12 *= -dens_1;

            force += f_12;
        }

        /* Viscosity forces. */
        {
            float w_12 = kernel_viscosity(length_12, kernel_radius);
            float4 f_12 = particles[id_2].vel - vel_1;
            f_12 *= viscosity * particles[id_2].mass * w_12 / dens_2;

            force += f_12;
        }
    }

    particles[id_1].force = force;
}

/** ---------------------------------------------------------------------------
 * compute_pbc
 * @brief Return the periodic image in primary cell of the fluid domain.
 */
float4 compute_pbc(const float4 pos, const __global Domain_t *domain)
{
    float4 bound_lo = domain->bound_lo;
    float4 bound_hi = domain->bound_hi;
    float4 length = bound_hi - bound_lo;

    float4 image = pos;

    /* pbc along x-dimension */
    if (image.x < bound_lo.x) {
        image.x += length.x;
    }

    if (image.x > bound_hi.x) {
        image.x -= length.x;
    }

    /* pbc along y-dimension */
    if (image.y < bound_lo.y) {
        image.y += length.y;
    }

    if (image.y > bound_hi.y) {
        image.y -= length.y;
    }

    /* pbc along z-dimension */
    if (image.z < bound_lo.z) {
        image.z += length.z;
    }

    if (image.z > bound_hi.z) {
        image.z -= length.z;
    }

    return image;
}
