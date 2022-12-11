#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

/**
 * reduce
 */
void reduce(__global double *group_sums, __local double *local_sums);

/**
 * pi
 * @brief Compute the pi integral
 */
__kernel void pi(
    __global double *group_sums,
    __local double *local_sums,
    const uint num_iters,
    const double step_size)
{
    const ulong local_id = get_local_id(0);
    const ulong local_size = get_local_size(0);
    const ulong group_id = get_group_id(0);

    const ulong begin = (group_id * local_size + local_id) * num_iters;
    const ulong end = begin + num_iters;

    /* Compute local sums */
    double sum = 0.0;
    for(ulong i = begin; i < end; ++i) {
        double x = (i + 0.5) * step_size;
        sum += 4.0 / (1.0 + x*x);
    }

    /* Ensure local memory consistency before summing the elements.  */
    local_sums[local_id] = sum * step_size;
    barrier(CLK_LOCAL_MEM_FENCE);

    reduce(group_sums, local_sums);
}

/**
 * reduce
 * @brief Perform a reduction sum of the local sums and store it
 * at the partial sums array at the corresponding work group index.
 */
void reduce(__global double *group_sums, __local double *local_sums)
{
    const uint local_id = get_local_id(0);
    const uint local_size = get_local_size(0);
    const uint group_id = get_group_id(0);

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