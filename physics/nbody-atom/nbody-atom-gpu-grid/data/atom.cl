/** Compute pair interaction. */
Pair_t atom_force_pair(
    const ulong atom_1,
    const ulong atom_2,
    const double4 r_12,
    const __global Field_t *field);

/** Return the periodic image in primary cell of the fluid domain. */
double4 atom_pbc(const __global Domain_t *domain, const double4 pos);

/** ---------------------------------------------------------------------------
 * atom_begin_integrate
 * @brief Begin integration - first half of the integration step.
 */
__kernel void atom_begin_integrate(
    const double t_step,
    const ulong n_atoms,
    __global Atom_t *atoms,
    const __global Thermostat_t *thermostat)
{
    const ulong atom_ix = get_global_id(0);
    if (atom_ix < n_atoms) {
        const double half_t_step = 0.5 * t_step;

        /* Integrate momenta half time step */
        double exp_eta = exp(-thermostat->eta * half_t_step);
        atoms[atom_ix].mom += atoms[atom_ix].force * half_t_step;
        atoms[atom_ix].mom *= exp_eta;

        /* Integrate positions at full time step */
        double4 vel = atoms[atom_ix].mom * atoms[atom_ix].rmass;
        atoms[atom_ix].pos += vel * t_step;
        atoms[atom_ix].upos += vel * t_step;
    }
}

/** ---------------------------------------------------------------------------
 * atom_end_integrate
 * @brief End integration - second half of the integration step.
 */
__kernel void atom_end_integrate(
    const double t_step,
    const ulong n_atoms,
    __global Atom_t *atoms,
    const __global Thermostat_t *thermostat)
{
    const ulong atom_ix = get_global_id(0);
    if (atom_ix < n_atoms) {
        const double half_t_step = 0.5 * t_step;

        /* Integrate momenta half time step */
        double exp_eta = exp(-thermostat->eta * half_t_step);
        atoms[atom_ix].mom *= exp_eta;
        atoms[atom_ix].mom += atoms[atom_ix].force * half_t_step;
    }
}

/** ---------------------------------------------------------------------------
 * atom_update
 * @brief Update atom positions and associated data structures.
 */
__kernel void atom_update(
    const ulong n_atoms,
    __global Atom_t *atoms,
    const __global Domain_t *domain)
{
    const ulong atom_ix = get_global_id(0);
    if (atom_ix < n_atoms) {
        /* Apply pbc to the fluid particle positions. */
        atoms[atom_ix].pos = atom_pbc(domain, atoms[atom_ix].pos);
    }
}

/** ---------------------------------------------------------------------------
 * atom_force
 * @brief Compute fluid forces.
 */
__kernel void atom_force(
    const ulong n_atoms,
    const ulong n_neighbours,
    __global Atom_t *atoms,
    const __global Domain_t *domain,
    const __global Field_t *field,
    const double4 length,
    const int4 n_cells,
    const uint n_items,
    const uint capacity,
    const __global Grid_t *grid)
{
    const ulong atom_1 = get_global_id(0);
    if (atom_1 < n_atoms) {
        const double r_cut_sq = field->r_cut * field->r_cut;

        /*
         * Loop over the neighbours of the atom cell in the centre.
         * For each neighbour cell, loop over its atoms and compute
         * the pairwise interaction.
         */
        double4 force = (double4) (0.0);
        double energy = 0.0;
        double16 virial = (double16) (0.0);

        const ulong pair_begin = atom_1 * n_neighbours;
        const ulong pair_end = pair_begin + n_neighbours;
        ulong pair_ix = pair_begin;

        int4 base = grid_cell(atoms[atom_1].pos, length, n_cells);
        for (int ix = base.x - 1; ix <= base.x + 1; ++ix) {
            for (int iy = base.y - 1; iy <= base.y + 1; ++iy) {
                for (int iz = base.z - 1; iz <= base.z + 1; ++iz) {
                    int4 cell = grid_pbc((int4) (ix,iy,iz,0), n_cells);
                    uint key = grid_hash(cell, n_cells, n_items, capacity);
                    uint slot = grid_begin(key, capacity, grid);
                    while (slot != kEmpty) {
                        ulong atom_2 = grid_get(slot, grid);
                        slot = grid_next(slot, key, capacity, grid);

                        if (atom_1 ==  atom_2) {
                            continue;
                        }

                        double4 r_12 = atoms[atom_1].pos - atoms[atom_2].pos;
                        r_12 = atom_pbc(domain, r_12);

                        if (pair_ix < pair_end && dot(r_12, r_12) < r_cut_sq) {
                            Pair_t pair = atom_force_pair(atom_1, atom_2, r_12, field);
                            force  -= pair.gradient;
                            energy += 0.5 * pair.energy;
                            virial += 0.5 * pair.virial;
                            pair_ix++;
                        }
                    }
                }
            }
        }

        atoms[atom_1].force = force;
        atoms[atom_1].energy = energy;
        atoms[atom_1].virial = virial;
    }
}
/**
 * atom_force_pair
 * @brief Compute pair interaction.
 */
Pair_t atom_force_pair(
    const ulong atom_1,
    const ulong atom_2,
    const double4 r_12,
    const __global Field_t *field)
{
    /* Compute pair attributes - energy, gradient, laplacian and virial. */
    Pair_t pair;

    pair.atom_1 = atom_1;
    pair.atom_2 = atom_2;
    pair.r_12 = r_12;

    pair.energy = 0.0;
    pair.gradient = (double4) (0.0);
    pair.laplace = (double4) (0.0);
    pair.virial = (double16) (0.0);

    /* Interaction coefficients. */
    const double sigma_sq = field->sigma * field->sigma;
    const double r_hard_sq = field->r_hard * field->r_hard;

    /* Define pair energy and force coefficients */
    double energy_coeff = 4.0 * field->epsilon;
    double force_coeff = 24.0 * field->epsilon / sigma_sq;

    /* Ensure pair distance is larger than the hard sphere radius. */
    double energy_hard_sphere = 0.0;
    double r_12_sq = dot(r_12, r_12);
    if (r_12_sq < r_hard_sq) {
        /* Compute the hard sphere energy correction. */
        double r_12_len = sqrt(r_12_sq);

        double r_hard_2  = sigma_sq / r_hard_sq;
        double r_hard_4  = r_hard_2 * r_hard_2;
        double r_hard_6  = r_hard_4 * r_hard_2;
        double r_hard_12 = r_hard_6 * r_hard_6;

        energy_hard_sphere = -24.0 * field->epsilon * (2.0 * r_hard_12 - r_hard_6);
        energy_hard_sphere *= (r_12_len - field->r_hard) / r_12_len;

        /* Set the pair distance to the hard sphere radius. */
        r_12_sq = r_hard_sq;
    }

    double rr2  = sigma_sq / r_12_sq;
    double rr4  = rr2 * rr2;
    double rr6  = rr4 * rr2;
    double rr8  = rr4 * rr4;
    double rr12 = rr6 * rr6;
    double rr14 = rr8 * rr6;

    /* LJ energy */
    pair.energy = energy_coeff * (rr12 - rr6) + energy_hard_sphere;

    /* LJ gradient */
    pair.gradient = r_12;
    pair.gradient *= (double4) (-force_coeff * (2.0 * rr14 - rr8));

    /* LJ laplacian */
    double laplace_c1 = force_coeff * (28.0 * rr14 - 8.0 * rr8) / r_12_sq;
    double laplace_c2 = force_coeff * (2.0 * rr14 - rr8);

    pair.laplace.x = laplace_c1 * r_12.x * r_12.x - laplace_c2;
    pair.laplace.y = laplace_c1 * r_12.y * r_12.y - laplace_c2;
    pair.laplace.z = laplace_c1 * r_12.z * r_12.z - laplace_c2;

    /* LJ virial */
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

/** ---------------------------------------------------------------------------
 * atom_pbc
 * @brief Return the periodic image in primary cell of the fluid domain.
 */
double4 atom_pbc(const __global Domain_t *domain, const double4 pos)
{
    double4 image = pos;

    /* pbc along x-dimension */
    if (image.x < -domain->length_half.x) {
        image.x += domain->length.x;
    }
    if (image.x >  domain->length_half.x) {
        image.x -= domain->length.x;
    }

    /* pbc along y-dimension */
    if (image.y < -domain->length_half.y) {
        image.y += domain->length.y;
    }
    if (image.y >  domain->length_half.y) {
        image.y -= domain->length.y;
    }

    /* pbc along z-dimension */
    if (image.z < -domain->length_half.z) {
        image.z += domain->length.z;
    }
    if (image.z >  domain->length_half.z) {
        image.z -= domain->length.z;
    }

    return image;
}

/** ---------------------------------------------------------------------------
 * atom_copy_vertex
 * @brief Copy the atom positions onto the vertex buffer object.
 */
__kernel void atom_copy_vertex(
    const ulong n_atoms,
    __global Atom_t *atoms,
    __global float *vertex)
{
    const ulong atom_ix = get_global_id(0);
    if (atom_ix < n_atoms) {
        vertex[6 * atom_ix + 0] = atoms[atom_ix].pos.x;
        vertex[6 * atom_ix + 1] = atoms[atom_ix].pos.y;
        vertex[6 * atom_ix + 2] = atoms[atom_ix].pos.z;
        vertex[6 * atom_ix + 3] = 1.0;
        vertex[6 * atom_ix + 4] = 0.0;
        vertex[6 * atom_ix + 5] = 0.0;
    }
}

