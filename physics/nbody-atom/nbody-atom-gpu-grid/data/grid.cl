/** ---------------------------------------------------------------------------
 * @brief Return the first slot containing the specified key.
 */
uint grid_begin(
    const uint key,
    const uint capacity,
    const __global Grid_t *grid);

/** Return the next slot containing the specified key. */
uint grid_next(
    uint slot,
    const uint key,
    const uint capacity,
    const __global Grid_t *grid);

/** Return the value of the current slot. */
uint grid_get(uint slot, const __global Grid_t *grid);

/** Return the cell coordinates containing the specified position. */
int4 grid_cell(const double4 pos, const double4 length, const int4 n_cells);

/** Compute the hash key of the cell coordinates. */
uint grid_hash(
    const int4 cell_coord,
    const int4 n_cells,
    const uint n_items,
    const uint capacity);

/** Return the periodic image of the specfied cell. */
int4 grid_pbc(const int4 cell_coord, const int4 n_cells);

/** ---------------------------------------------------------------------------
 * grid_clear
 * @brief Clear all key-value pairs in the grid and reset their count.
 */
__kernel void grid_clear(const uint capacity, __global Grid_t *grid)
{
    const uint item_ix = get_global_id(0);
    if (item_ix < capacity) {
        grid[item_ix].key = kEmpty;
        grid[item_ix].value = kEmpty;
    }
}

/**
 * grid_insert
 * @brief Insert the atom positions into the grid.
 */
__kernel void grid_insert(
    const ulong n_atoms,
    __global Atom_t *atoms,
    const double4 length,
    const int4 n_cells,
    const uint n_items,
    const uint capacity,
    __global Grid_t *grid)
{
    const ulong atom_ix = get_global_id(0);
    if (atom_ix < n_atoms) {
        int4 cell = grid_cell(atoms[atom_ix].pos, length, n_cells);
        uint key = grid_hash(cell, n_cells, n_items, capacity);
        uint slot = key % capacity;         // & (capacity - 1)
        while (true) {
            uint prev = atomic_cmpxchg(
                (volatile __global unsigned int *) (&grid[slot].key),
                kEmpty,
                key);

            if (prev == kEmpty) {
                grid[slot].value = atom_ix;
                return;
            }

            slot = (slot + 1) % capacity;   // & (capacity - 1);
        }
   }
}

/** ---------------------------------------------------------------------------
 * grid_begin
 * @brief Return the first slot containing the specified key. If no such key
 * exists, return empty signalling no further slots contain the specified key.
 */
uint grid_begin(
    const uint key,
    const uint capacity,
    const __global Grid_t *grid)
{
    /* Do nothing if key is empty */
    if (key == kEmpty) {
        return kEmpty;
    }

    /* Loop over the cell data until a slot with the specified key is found. */
    uint slot = key % capacity;     // & (capacity - 1)
    while (true) {
        if (grid[slot].key == key) {
            return slot;
        }

        if (grid[slot].key == kEmpty) {
            return kEmpty;
        }

        slot = (slot + 1) % capacity;   // & (capacity - 1)
    }
}

/**
 * grid_next
 * @brief Return the next slot containing the specified key.
 * Return empty if we reached the end of the list.
 */
uint grid_next(
    uint slot,
    const uint key,
    const uint capacity,
    const __global Grid_t *grid)
{
    while (true) {
        slot = (slot + 1) % capacity;   // & (capacity - 1)

        if (grid[slot].key == key) {
            return slot;
        }

        if (grid[slot].key == kEmpty) {
            return kEmpty;
        }
    }
}

/**
 * grid_get
 * @brief Return the value of the current slot.
 */
uint grid_get(uint slot, const __global Grid_t *grid)
{
    return grid[slot].value;
}

/** ---------------------------------------------------------------------------
 * grid_cell
 * @brief Return the cell coordinates containing the specified position.
 * The position is assumed to be in the range of (-length/2, length/2):
 *  u_pos = 0.5 + (pos / length) and 0 <= u_pos <= 1.
 */
int4 grid_cell(const double4 pos, const double4 length, const int4 n_cells)
{
    /* Get the position in normalized coordinates. */
    double4 u_pos = double4(0.5) + (pos / length);
    return int4(u_pos.x * n_cells.x,
                u_pos.y * n_cells.y,
                u_pos.z * n_cells.z,
                0 /*unused*/);
}

/**
 * grid_hash
 * @brief Compute the hash key of the cell coordinates.
 * Return the key if the cell is inside the grid range. Return empty otherwise.
 */
uint grid_hash(
    const int4 cell_coord,
    const int4 n_cells,
    const uint n_items,
    const uint capacity)
{
    if (cell_coord.x < 0 || cell_coord.x >= n_cells.x ||
        cell_coord.y < 0 || cell_coord.y >= n_cells.y ||
        cell_coord.z < 0 || cell_coord.z >= n_cells.z ) {
        return kEmpty;
    }

    uint slot = n_items * (cell_coord.x * n_cells.y * n_cells.z +
                           cell_coord.y * n_cells.z +
                           cell_coord.z);
    return slot % capacity;
}

/**
 * grid_pbc
 * @brief Return the periodic image of the specfied cell.
 */
int4 grid_pbc(const int4 cell_coord, const int4 n_cells)
{
    int4 cell_image = cell_coord;

    if (cell_image.x < 0) {
        cell_image.x += n_cells.x;
    }
    if (cell_image.x >= n_cells.x) {
        cell_image.x -= n_cells.x;
    }

    if (cell_image.y < 0) {
        cell_image.y += n_cells.y;
    }
    if (cell_image.y >= n_cells.y) {
        cell_image.y -= n_cells.y;
    }

    if (cell_image.z < 0) {
        cell_image.z += n_cells.z;
    }
    if (cell_image.z >= n_cells.z) {
        cell_image.z -= n_cells.z;
    }

    return cell_image;
}
