
/** @brief Compute pair interaction. */
Pair_t force_pair(const double4 r_12, const __global Field_t *field);

/** @brief Return the periodic image in primary cell of the fluid domain. */
double4 compute_pbc(const double4 pos, const __global Domain_t *domain);

/** ---------------------------------------------------------------------------
 * begin_integrate
 * @brief Begin integration - first half of the integration step.
 */
__kernel void begin_integrate(
    const double t_step,
    const uint n_atoms,
    __global Atom_t *atoms,
    const __global Thermostat_t *thermostat)
{
    const uint idx = get_global_id(0);
    if (idx < n_atoms) {
        const double half_t_step = 0.5 * t_step;

        /* Integrate momenta half time step */
        double exp_eta = exp(-thermostat->eta * half_t_step);
        atoms[idx].mom += atoms[idx].force * half_t_step;
        atoms[idx].mom *= exp_eta;

        /* Integrate positions at full time step */
        double4 vel = atoms[idx].mom * atoms[idx].rmass;
        atoms[idx].pos += vel * t_step;
        atoms[idx].upos += vel * t_step;
    }
}

/**
 * end_integrate
 * @brief End integration - second half of the integration step.
 */
__kernel void end_integrate(
    const double t_step,
    const uint n_atoms,
    __global Atom_t *atoms,
    const __global Thermostat_t *thermostat)
{
    const uint idx = get_global_id(0);
    if (idx < n_atoms) {
        const double half_t_step = 0.5 * t_step;

        /* Integrate momenta half time step */
        double exp_eta = exp(-thermostat->eta * half_t_step);
        atoms[idx].mom *= exp_eta;
        atoms[idx].mom += atoms[idx].force * half_t_step;
    }
}

/**
 * update_atoms
 * @brief Update atom positions and associated data structures.
 */
__kernel void update_atoms(
    const uint n_atoms,
    __global Atom_t *atoms,
    const __global Domain_t *domain)
{
    /* Apply pbc to atom positions. */
    const uint idx = get_global_id(0);
    if (idx < n_atoms) {
        atoms[idx].pos = compute_pbc(atoms[idx].pos, domain);
    }
}

/**
 * compute_forces
 * @brief Compute fluid forces.
 */
__kernel void compute_forces(
    const uint n_atoms,
    const uint n_neighbours,
    __global Atom_t *atoms,
    const __global uint *list,
    const __global Domain_t *domain,
    const __global Field_t *field)
{
    /*
     * Loop over the atom neighbours and compute the pair interaction.
     */
    const uint idx_1 = get_global_id(0);
    if (idx_1 < n_atoms) {
        const double r_cut_sq = field->r_cut * field->r_cut;

        double4 force = (double4) (0.0);
        double energy = 0.0;
        double16 virial = (double16) (0.0);

        const uint begin_idx = idx_1 * n_neighbours;
        const uint end_idx = begin_idx + n_neighbours;

        uint neig_idx = begin_idx;
        while (neig_idx < end_idx && list[neig_idx] != kEmpty) {
            const uint idx_2 = list[neig_idx];

            double4 r_12 = atoms[idx_1].pos - atoms[idx_2].pos;
            r_12 = compute_pbc(r_12, domain);

            if (dot(r_12, r_12) < r_cut_sq) {
                Pair_t pair = force_pair(r_12, field);
                force  -= pair.gradient;
                energy += 0.5 * pair.energy;
                virial += 0.5 * pair.virial;
            }

            neig_idx++;
        }

        atoms[idx_1].force = force;
        atoms[idx_1].energy = energy;
        atoms[idx_1].virial = virial;
    }
}

/**
 * copy_atom_points
 * @brief Copy the atom positions onto the vertex buffer object.
 */
__kernel void copy_atom_points(
    const uint n_atoms,
    __global float *vertex,
    const __global Atom_t *atoms)
{
    const uint idx = get_global_id(0);
    if (idx < n_atoms) {
        double r = (double) idx / n_atoms;
        vertex[6*idx + 0] = atoms[idx].pos.x;
        vertex[6*idx + 1] = atoms[idx].pos.y;
        vertex[6*idx + 2] = atoms[idx].pos.z;
        vertex[6*idx + 3] = r;
        vertex[6*idx + 4] = 0.0;
        vertex[6*idx + 5] = 0.0;
    }
}

/** ---------------------------------------------------------------------------
 * force_pair
 * @brief Compute pair interaction.
 */
Pair_t force_pair(const double4 r_12, const __global Field_t *field)
{
    /* Compute pair attributes - energy, gradient, laplacian and virial. */
    Pair_t pair;
    pair.r_12 = r_12;
    pair.energy = 0.0;
    pair.gradient = (double4) (0.0);
    pair.laplace = (double4) (0.0);
    pair.virial = (double16) (0.0);

    /* Interaction coefficients. */
    const double sigma_sq = field->sigma * field->sigma;
    const double r_hard_sq = field->r_hard * field->r_hard;

    /* Define pair energy and force coefficients. */
    double energy_coeff = 4.0 * field->epsilon;
    double force_coeff = 24.0 * field->epsilon / sigma_sq;

    /* Ensure pair distance is larger than the hard sphere radius. */
    double energy_hard_sphere = 0.0;
    double r_12_sq = dot(r_12, r_12);
    if (r_12_sq < r_hard_sq) {
        /* Compute the hard sphere energy correction. */
        double r_12_len = sqrt(r_12_sq);

        double r_hard2  = sigma_sq / r_hard_sq;
        double r_hard4  = r_hard2 * r_hard2;
        double r_hard6  = r_hard4 * r_hard2;
        double r_hard12 = r_hard6 * r_hard6;

        energy_hard_sphere = -24.0*field->epsilon * (2.0*r_hard12 - r_hard6);
        energy_hard_sphere *= (r_12_len - field->r_hard) / r_12_len;

        /* Set the pair distance to the hard sphere radius. */
        r_12_sq = r_hard_sq;
    }

    double rr2  = sigma_sq / r_12_sq;
    double rr4  = rr2 * rr2;
    double rr6  = rr4 * rr2;
    double rr8  = rr4 * rr4;
    double rr12 = rr8 * rr4;
    double rr14 = rr8 * rr6;

    /* LJ energy. */
    pair.energy = energy_coeff * (rr12 - rr6) + energy_hard_sphere;

    /* LJ gradient. */
    pair.gradient = r_12;
    pair.gradient *= (double4) (-force_coeff * (2.0 * rr14 - rr8));

    /* LJ laplacian. */
    double laplace_c1 = force_coeff * (28.0 * rr14 - 8.0 * rr8) / r_12_sq;
    double laplace_c2 = force_coeff * (2.0 * rr14 - rr8);

    pair.laplace.x = laplace_c1 * r_12.x * r_12.x - laplace_c2;
    pair.laplace.y = laplace_c1 * r_12.y * r_12.y - laplace_c2;
    pair.laplace.z = laplace_c1 * r_12.z * r_12.z - laplace_c2;

    /* LJ virial. */
    pair.virial.s0 = -r_12.x * pair.gradient.x;
    pair.virial.s1 = -r_12.x * pair.gradient.y;
    pair.virial.s2 = -r_12.x * pair.gradient.z;

    pair.virial.s3 = -r_12.y * pair.gradient.x;
    pair.virial.s4 = -r_12.y * pair.gradient.y;
    pair.virial.s5 = -r_12.y * pair.gradient.z;

    pair.virial.s6 = -r_12.z * pair.gradient.x;
    pair.virial.s7 = -r_12.z * pair.gradient.y;
    pair.virial.s8 = -r_12.z * pair.gradient.z;

    return pair;
}

/**
 * compute_pbc
 * @brief Return the periodic image in primary cell of the fluid domain.
 */
double4 compute_pbc(const double4 pos, const __global Domain_t *domain)
{
    double4 length_half = domain->length_half;
    double4 length = domain->length;
    double4 image = pos;

    /* pbc along x-dimension */
    if (image.x < -length_half.x) {
        image.x += length.x;
    }
    if (image.x >  length_half.x) {
        image.x -= length.x;
    }

    /* pbc along y-dimension */
    if (image.y < -length_half.y) {
        image.y += length.y;
    }
    if (image.y >  length_half.y) {
        image.y -= length.y;
    }

    /* pbc along z-dimension */
    if (image.z < -length_half.z) {
        image.z += length.z;
    }
    if (image.z >  length_half.z) {
        image.z -= length.z;
    }

    return image;
}
