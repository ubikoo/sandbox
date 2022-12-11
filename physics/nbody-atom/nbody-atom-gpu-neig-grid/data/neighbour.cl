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
 * @brief Build the neighbour list using a grid map spatial data structure.
 */
__kernel void build_nlist(
    const uint n_atoms,
    const uint n_neighbours,
    const double radius,
    __global uint *list,
    const double4 length,
    const int4 n_cells,
    const uint n_nodes,
    const uint capacity,
    const __global Node_t *grid,
    const __global Atom_t *atoms,
    const __global Domain_t *domain)
{
    const uint idx_1 = get_global_id(0);
    if (idx_1 < n_atoms) {
        double radius_sq = radius * radius;

        Atom_t atom_1 = atoms[idx_1];

        const uint begin_idx = idx_1 * n_neighbours;
        const uint end_idx = begin_idx + n_neighbours;

        uint neig_idx = begin_idx;
        int4 base = grid_cell_coord(atom_1.pos, length, n_cells);

        for (int ix = base.x - 1; ix <= base.x + 1; ++ix) {
            for (int iy = base.y - 1; iy <= base.y + 1; ++iy) {
                for (int iz = base.z - 1; iz <= base.z + 1; ++iz) {

                    int4 cell = grid_cell_pbc((int4) (ix, iy, iz, 0), n_cells);
                    uint key = grid_coord_hash(cell, n_cells, n_nodes);
                    uint node = grid_begin(key, capacity, grid);

                    while (node != kEmpty) {
                        uint idx_2 = grid[node].atom;
                        node = grid_next(node, key, capacity, grid);

                        if (idx_1 == idx_2) {
                            continue;
                        }

                        Atom_t atom_2 = atoms[idx_2];
                        double4 r_12 = atom_1.pos - atom_2.pos;
                        r_12 = compute_pbc(r_12, domain);

                        if (neig_idx < end_idx && dot(r_12, r_12) < radius_sq) {
                            list[neig_idx] = idx_2;
                            neig_idx++;
                        }
                    }
                }
            }
        }
    }
}
