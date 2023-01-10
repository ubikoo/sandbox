#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

#define k_empty_state 0xffffffff

/** --------------------------------------------------------------------------
 * Point data type.
 */
typedef struct {
    float3 pos;
    float3 col;
} Point_t;

/**
 * KeyValue data type.
 */
typedef struct {
    uint key;
    uint value;
} KeyValue_t;

/**  * Hashmap hash function. */
uint hash(const uint3 v);

/** --------------------------------------------------------------------------
 * hash
 * Hashmap hash function.
 */
uint hash(const uint3 v)
{
    uint c1 = 73856093;
    uint c2 = 19349663;
    uint c3 = 83492791;

    uint h1 = c1 * v.s0;
    uint h2 = c2 * v.s1;
    uint h3 = c3 * v.s2;

    return (h1 ^ h2 ^ h3);
    // return (7*h1 + 503*h2 + 24847*h3);
}

/** --------------------------------------------------------------------------
 * hashmap_insert
 * Insert a set of points into the hashmap KeyValue array.
 */
__kernel void hashmap_insert(
    __global KeyValue_t *hashmap,
    const __global Point_t *points,
    const uint capacity,
    const uint n_points,
    const uint n_cells,
    const float3 domain_lo,
    const float3 domain_hi)
{
    const long ix = get_global_id(0);
    if (ix < n_points) {
        float3 u_pos = (points[ix].pos - domain_lo) / (domain_hi - domain_lo);
        uint3 cell_ix = (uint3) (
            (float) n_cells * u_pos.x,
            (float) n_cells * u_pos.y,
            (float) n_cells * u_pos.z);
        uint key = hash(cell_ix);

        uint slot = key % capacity;
        while (true) {
            uint prev = atomic_cmpxchg(
                (volatile __global unsigned int *) (&hashmap[slot].key),
                k_empty_state,
                key);

            if (prev == k_empty_state) {
                hashmap[slot].value = ix;
                return;
            }

            slot = (slot + 1) % capacity;   // & (capacity - 1);
        }
    }
}
