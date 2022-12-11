#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

/**
 * 2d Point data structure.
 */
typedef struct {
    double x;
    double y;
} Point_t;

/**
 * pi
 * @brief Compute the pi integral
 */
__kernel void pi(
    __global double *group_sums,
    __local double *local_sums,
    const __global Point_t *points,
    const uint n_points)
{
    // const ulong global_size = get_global_size(0);
    const ulong global_id = get_global_id(0);
    const ulong local_size = get_local_size(0);
    const ulong local_id = get_local_id(0);
    const ulong group_id = get_group_id(0);

    /* Setup the local point data. */
    local_sums[local_id] = 0.0;
    if (global_id < n_points - 1) {
        double del_x = points[global_id + 1].x - points[global_id].x;
        double mid_y = points[global_id + 1].y + points[global_id].y;
        local_sums[local_id] = 0.5 * mid_y * del_x;
    }

    /*
     * Sum the array using a divide and conquer algorithm:
     *  - partition the work group in two halves.
     *  - add the upper half of the work group to the lower half.
     */
    for (uint stride = local_size / 2; stride > 0; stride >>=1) {
        /* Ensure all work item writes are complete before adding again. */
        barrier(CLK_LOCAL_MEM_FENCE);
        if (local_id < stride) {
            local_sums[local_id] += local_sums[local_id + stride];
        }
    }

    if (local_id == 0) {
        group_sums[group_id] = local_sums[0];
    }
}
