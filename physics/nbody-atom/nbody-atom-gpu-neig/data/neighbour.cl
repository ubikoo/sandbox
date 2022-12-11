/** ---------------------------------------------------------------------------
 * clear_nlist
 * @brief Clear the neighbour list.
 */
__kernel void clear_nlist(const uint capacity, __global uint *list)
{
    const uint neig_idx = get_global_id(0);
    if (neig_idx < capacity) {
        list[neig_idx] = kEmpty;
    }
}

/** ---------------------------------------------------------------------------
 * build_nlist
 * @brief Build the neighbour list using full N^2 iteration over atom pairs.
 */
__kernel void build_nlist(
    const uint n_atoms,
    const uint n_neighbours,
    const double radius,
    __global uint *list,
    const __global Atom_t *atoms,
    const __global Domain_t *domain)
{
    /*
     * Loop over all atoms and add the indices of those whose distance
     * to the current atom is smaller than the neighbour list max radius.
     */
    const uint idx_1 = get_global_id(0);
    if (idx_1 < n_atoms) {
        double radius_sq = radius * radius;

        const uint begin_idx = idx_1 * n_neighbours;
        const uint end_idx = begin_idx + n_neighbours;

        uint neig_idx = begin_idx;
        for (uint idx_2 = 0; idx_2 < n_atoms; ++idx_2) {
            if (idx_1 == idx_2) {
                continue;
            }

            double4 r_12 = atoms[idx_1].pos - atoms[idx_2].pos;
            r_12 = compute_pbc(r_12, domain);

            if (neig_idx < end_idx && dot(r_12, r_12) < radius_sq) {
                list[neig_idx] = idx_2;
                neig_idx++;
            }
        }
    }
}
