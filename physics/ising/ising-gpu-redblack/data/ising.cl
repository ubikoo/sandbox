/** site_img */
long site_img(long x, const long n_sites);

/** site_idx */
long site_idx(long x, long y, const long n_sites);

/** site_energy */
double site_energy(
    double ising_J,
    double ising_h,
    int site,
    int site_xlo,
    int site_xhi,
    int site_ylo,
    int site_yhi);

/** George Marsaglias xorshift algorithm */
uint rand_xorshift(uint rng_state);

/** ---------------------------------------------------------------------------
 * random_lattice
 * @brief Sample a random lattice.
 */
__kernel void random_lattice(const long n_sites, __global uint *random)
{
    const long x = get_global_id(0);    // global pos in x-direction
    const long y = get_global_id(1);    // global pos in y-direction

    if (x < n_sites && y < n_sites) {
        long ix = site_idx(x, y, n_sites);
        uint state = random[ix];

        state ^= (state << 13);
        state ^= (state >> 17);
        state ^= (state << 5);

        random[ix] = state;
    }
}

/** ---------------------------------------------------------------------------
 * init_lattice
 * @brief Initialize the lattice state.
 */
__kernel void init_lattice(
    const long n_sites,
    __global int *sites,
    __global uint *random)
{
    const long x = get_global_id(0);    // global pos in x-direction
    const long y = get_global_id(1);    // global pos in y-direction

    if (x < n_sites && y < n_sites) {
        long ix = site_idx(x, y, n_sites);

        /* Generate a random number generator */
        double r = (double) random[ix] / UINT_MAX;
        sites[ix] = r < 0.5 ? 1 : -1;
    }
}

/** ---------------------------------------------------------------------------
 * flip_lattice
 * @brief Flip the lattice row sites with offset.
 */
__kernel void flip_lattice(
    const double ising_J,
    const double ising_h,
    const double ising_beta,
    const long redblack,
    const long n_sites,
    __global int *sites,
    __global uint *random)
{
    const long row = get_global_id(0);    // global pos in x-direction
    const long col = get_global_id(1);    // global pos in y-direction

    if (row < n_sites && col < n_sites / 2) {
        /*
        * Ensure redblack is 0 or 1.
        * If redblack is 0, even rows have offset 0 and odd rows have offset 1
        * If redblack is 1, even rows have offset 1 and odd rows have offset 0
        */
        long offset = redblack % 2;
        offset = (row + offset) % 2;

        /* Try to flip the sites in the specified row and column. */
        long x = row;
        long y = 2 * col + offset;

        /* Flip the spin and compute the change in energy. */
        int site = sites[site_idx(x, y, n_sites)];
        int site_xlo = sites[site_idx(x - 1, y, n_sites)];
        int site_xhi = sites[site_idx(x + 1, y, n_sites)];
        int site_ylo = sites[site_idx(x, y - 1, n_sites)];
        int site_yhi = sites[site_idx(x, y + 1, n_sites)];

        double old_energy = site_energy(
            ising_J, ising_h, site, site_xlo, site_xhi, site_ylo, site_yhi);
        site *= -1;
        double new_energy = site_energy(
            ising_J, ising_h, site, site_xlo, site_xhi, site_ylo, site_yhi);
        double del_energy = new_energy - old_energy;

        /* Accept/reject the transition. */
        double exp_e = exp(-ising_beta * del_energy);
        double r = (double) random[site_idx(x, y, n_sites)] / UINT_MAX;
        if (r < exp_e) {
            sites[site_idx(x, y, n_sites)] = site;
        }
    }
}

/** ---------------------------------------------------------------------------
 * image_lattice kernel source
 */
__kernel void image_lattice(
    const long n_sites,
    __global const int *sites,
    sampler_t sampler,
    const long width,
    const long height,
    __write_only image2d_t image)
{
    const long x = get_global_id(0);    // global pos in x-direction
    const long y = get_global_id(1);    // global pos in y-direction

    if (x < width && y < height) {
        /* Compute location of the data to move into (x,y). */
        float u = x / (float) width;
        float v = y / (float) height;

        long site_x = (long) (u * n_sites);
        long site_y = (long) (v * n_sites);
        long idx = site_idx(site_x, site_y, n_sites);

        float4 color = (float4) (0.8, 0.0, 0.0, 1.0);
        if (sites[idx] < 0) {
            color = (float4) (0.0, 0.0, 0.8, 1.0);
        }
        write_imagef(image, (int2) (x, y), color);
    }
}

/** ---------------------------------------------------------------------------
 * site_img
 * @brief Return the periodic image of position x.
 */
long site_img(long x, const long n_sites)
{
    if (x < 0) {
        x += n_sites;
    }
    if (x >= n_sites) {
        x -= n_sites;
    }
    return x;
}

/**
 * site_idx
 * @brief Return the index of the site (x,y)
 */
long site_idx(long x, long y, const long n_sites)
{
    x = site_img(x, n_sites);
    y = site_img(y, n_sites);
    return x * n_sites + y;
}

/**
 * site_energy
 * @brief Return the periodic image in primary cell of the fluid domain.
 */
double site_energy(
    double ising_J,
    double ising_h,
    int site,
    int site_xlo,
    int site_xhi,
    int site_ylo,
    int site_yhi)
{
    double e_J = -site * ising_J * (site_xlo + site_xhi + site_ylo + site_yhi);
    double e_h = -site * ising_h;
    return e_J + e_h;
}

/**
 * rand_xorshift
 * @brief George Marsaglia's xorshift algorithm
 */
uint rand_xorshift(uint rng_state)
{
    rng_state ^= (rng_state << 13);
    rng_state ^= (rng_state >> 17);
    rng_state ^= (rng_state << 5);
    return rng_state;
}
