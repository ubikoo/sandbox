
/** @brief Return the first node containing the specified key. */
uint grid_begin(
    const uint key,
    const uint capacity,
    const __global Node_t *grid);

/** @brief Return the next node containing the specified key. */
uint grid_next(
    uint node,
    const uint key,
    const uint capacity,
    const __global Node_t *grid);

/** @brief Return the coordinates of cell containing the specified position. */
int4 grid_cell_coord(
    const double4 pos,
    const double4 length,
    const int4 n_cells);

/** @brief Return the hash key of the cell coordinates. */
uint grid_coord_hash(
    const int4 cell,
    const int4 n_cells,
    const uint n_nodes);

/** Return the periodic image of the specified cell. */
int4 grid_cell_pbc(const int4 cell, const int4 n_cells);

/** ---------------------------------------------------------------------------
 * clear_grid
 * @brief Clear the grid map by setting their node keys to empty.
 */
__kernel void clear_grid(const uint capacity, __global Node_t *grid)
{
    const uint node_ix = get_global_id(0);
    if (node_ix < capacity) {
        grid[node_ix].key = kEmpty;
    }
}

/**
 * build_grid
 * @brief Build the grid map and store all the atoms in the grid node
 * locations keyed by the atom position.
 */
__kernel void build_grid(
    const uint n_atoms,
    const double4 length,
    const int4 n_cells,
    const uint n_nodes,
    const uint capacity,
    __global Node_t *grid,
    const __global Atom_t *atoms)
{
    /*
     * Compute the coordinates of the cell containing the atom position
     * and compute the corresponding hash function key in the grid map.
     * Starting at the first node in the map corresponding to the key,
     * search linearly for the first empty node and store the atom index.
     */
    const uint atom_idx = get_global_id(0);
    if (atom_idx < n_atoms) {
        int4 cell = grid_cell_coord(atoms[atom_idx].pos, length, n_cells);
        uint key = grid_coord_hash(cell, n_cells, n_nodes);

        uint node = key % capacity;         // & (capacity - 1)
        while (true) {
            uint prev = atomic_cmpxchg(
                (volatile __global uint *) (&grid[node].key),
                kEmpty,
                key);

            if (prev == kEmpty) {
                grid[node].atom = atom_idx;
                return;
            }

            node = (node + 1) % capacity;   // & (capacity - 1);
        }
    }
}

/** ---------------------------------------------------------------------------
 * grid_begin
 * @brief Return the index of the first node in the grid containing the
 * specified key. If no such key exists, then iteration function must
 * end in an empty node and returns empty.
 */
uint grid_begin(
    const uint key,
    const uint capacity,
    const __global Node_t *grid)
{
    if (key == kEmpty) {
        return kEmpty;
    }

    /*
     * Loop over the nodes until one with the specified key is found.
     */
    uint node = key % capacity;     // & (capacity - 1)
    while (true) {
        if (grid[node].key == key) {
            return node;
        }

        if (grid[node].key == kEmpty) {
            return kEmpty;
        }

        node = (node + 1) % capacity;   // & (capacity - 1)
    }
}

/**
 * grid_next
 * @brief Return the index of next node in the map containing the specified key.
 * If no such key exists, then iteration function must end in an empty node and
 * returns empty.
 */
uint grid_next(
    uint node,
    const uint key,
    const uint capacity,
    const __global Node_t *grid)
{
    while (true) {
        node = (node + 1) % capacity;   // & (capacity - 1)

        if (grid[node].key == key) {
            return node;
        }

        if (grid[node].key == kEmpty) {
            return kEmpty;
        }
    }
}

/** ---------------------------------------------------------------------------
 * grid_cell_coord
 * @brief Return the cell coordinates containing the specified position.
 * The position is assumed to be in the range of (-length/2, length/2):
 *      u_pos = 0.5 + (pos / length),   (0 <= u_pos <= 1)
 */
int4 grid_cell_coord(
    const double4 pos,
    const double4 length,
    const int4 n_cells)
{
    /* Get the position in normalized coordinates. */
    double4 u_pos = double4(0.5) + (pos / length);
    return int4(u_pos.x * n_cells.x,
                u_pos.y * n_cells.y,
                u_pos.z * n_cells.z,
                0 /*unused*/);
}

/**
 * grid_coord_hash
 * @brief Compute the hash key of the cell coordinates. If the cell coordinates
 * are outside the map, return empty.
 */
uint grid_coord_hash(
    const int4 cell,
    const int4 n_cells,
    const uint n_nodes)
{
    if (cell.x < 0 || cell.x >= n_cells.x ||
        cell.y < 0 || cell.y >= n_cells.y ||
        cell.z < 0 || cell.z >= n_cells.z ) {
        return kEmpty;
    }

    return n_nodes * (cell.x*n_cells.y*n_cells.z + cell.y*n_cells.z + cell.z);
}

/**
 * grid_cell_pbc
 * @brief Return the periodic image of the specified cell.
 */
int4 grid_cell_pbc(const int4 cell, const int4 n_cells)
{
    int4 image = cell;

    if (image.x < 0) {
        image.x += n_cells.x;
    }
    if (image.x >= n_cells.x) {
        image.x -= n_cells.x;
    }

    if (image.y < 0) {
        image.y += n_cells.y;
    }
    if (image.y >= n_cells.y) {
        image.y -= n_cells.y;
    }

    if (image.z < 0) {
        image.z += n_cells.z;
    }
    if (image.z >= n_cells.z) {
        image.z -= n_cells.z;
    }

    return image;
}
