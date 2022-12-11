
/** @brief Return the index of the first particle node with the specified key. */
uint grid_begin(
    const uint key,
    const uint capacity,
    const __global Node_t *grid);

/** @brief Return the index of the next particle node with the specified key. */
uint grid_next(
    uint node,
    const uint key,
    const uint capacity,
    const __global Node_t *grid);

/** @brief Return the cell coordinates containing the specified position. */
int4 grid_coord(const float4 pos, const float cell_size);

/** @brief Compute the hash key of the cell coordinates. */
uint grid_hash(const int4 coord);

/** ---------------------------------------------------------------------------
 * clear_grid
 * @brief Clear the grid nodes by setting their keys to empty.
 */
__kernel void clear_grid(const uint capacity, __global Node_t *grid)
{
    const uint node_id = get_global_id(0);
    if (node_id < capacity) {
        grid[node_id].key = kEmpty;
    }
}

/**
 * build_grid
 * @brief grid all the fluid particles in the input grid onto a new grid node
 * at locations specified by the particle position.
 */
__kernel void build_grid(
    const uint n_particles,
    const uint capacity,
    const float cell_size,
    __global Node_t *grid,
    const __global Particle_t *particles)
{
    const uint id = get_global_id(0);
    if (id >= n_particles) {
        return;
    }

    /*
     * Compute the key from the hash value of the particle coordinates.
     * Starting at the first node in the grid corresponding to the key,
     * search linearly for the first empty node and store the particle.
     */
    int4 coord = grid_coord(particles[id].pos, cell_size);
    uint key = grid_hash(coord);

    uint node = key % capacity;         // & (capacity - 1)
    while (true) {
        uint prev = atomic_cmpxchg(
            (volatile __global uint *) (&grid[node].key),
            kEmpty,
            key);

        if (prev == kEmpty) {
            grid[node].id = id;
            return;
        }

        node = (node + 1) % capacity;   // & (capacity - 1);
    }
}

/** ---------------------------------------------------------------------------
 * grid_begin
 * @brief Return the index of the first node in the grid containing the
 * specified key. If no such key exists, then the function must end in
 * an empty node and return kEmpty.
 */
uint grid_begin(
    const uint key,
    const uint capacity,
    const __global Node_t *grid)
{
    if (key == kEmpty) {
        return kEmpty;
    }

    /* Loop over the nodes until one with the specified key is found. */
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
 * @brief Return the index of next node in the grid containing the
 * specified key. If no such key exists, then the function must end
 * in an empty node and return kEmpty.
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
 * grid_coord
 * @brief Return the cell coordinates containing the specified position.
 * The position is assumed to be arbitrary and so will the coordinates of
 * the grid cell containing it. The hash function maps the cell coordinates
 * onto an array with a given capacity, at a location given by the hash key.
 */
int4 grid_coord(const float4 pos, const float cell_size)
{
    float4 u_pos = pos / cell_size;
    return (int4) (u_pos.x, u_pos.y, u_pos.z, 0 /*unused*/);
}

/**
 * grid_hash
 * @brief Compute the hash key of the cell coordinates.
 */
uint grid_hash(const int4 coord)
{
    const uint4 p = (uint4) (74856093, 19349663, 83492791, 0 /*unused*/);
    return (coord.x * p.x) ^ (coord.y * p.y) ^ (coord.z * p.z);
}
