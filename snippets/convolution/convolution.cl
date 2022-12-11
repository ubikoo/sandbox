#define FILTER_WIDTH  5 // convolution filter_mask size is 5x5

/**
 * convolution kernel source
 *
 * @brief Compute the NDRange for a 2D array with dimensions (array_width, array_height).
 *
 * Assuming the maximum number of workitems in a workgroup to be 256, take the
 * workgroup size to be 16 x 16 = 256:
 *
 * int workgroup_width = 16
 * int workgroup_height = 16
 *
 * The filter_width has a filter_radius = filter_width / 2. For example, if
 * filter_width = 5, filter_radius = int(5 / 2) = 2.
 * When computing the NDRange, an outer shell width = filter_radius of elements
 * int the array will not compute output values because accecssing the neighbours
 * from an array element in the outer shell would cause an out-of-bounds exception.
 *
 * For an array with dimensions (array_width, array_height), only
 * (array_width - 2*filter_radius) * (array_height - 2*filter_radius) workitems
 * are needed. The filter_padding is therefore 2*filter_radius = 4.
 *
 * The total number of workitems for the data is the roundup multiple of the
 * array_width and array_height minus the filter_padding:
 *
 * int workitems_x = roundup(array_width - filter_padding, workgroup_width)
 * int workitems_y = roundup(array_height - filter_padding, workgroup_height)
 *
 */
__kernel void convolution(
    __global int *data_in,
    __global int *data_out,
    __constant int *filter_mask,
    int data_width,
    int data_height,
    __local int *local_data,
    int local_width,
    int local_height)
{
    // Determine the amount of padding
    int filter_radius = FILTER_WIDTH / 2;
    int filter_padding = 2 * filter_radius;

    // Determine the workgroup column and row offset.
    int workgroup_begin_col = get_group_id(0) * get_local_size(0);
    int workgroup_begin_row = get_group_id(1) * get_local_size(1);

    // Get the workitem local id
    int local_col = get_local_id(0);
    int local_row = get_local_id(1);

    // Get the workitem global id
    int global_col = workgroup_begin_col + local_col;
    int global_row = workgroup_begin_row + local_row;

    // Cache data to local memory
    for (int i = local_row; i < local_height; i += get_local_size(1)) {
        int cur_row = workgroup_begin_row + i;

        for (int j = local_col; j < local_width; j += get_local_size(0)) {
            int cur_col = workgroup_begin_col + j;

            // Bound check the global indices
            if (cur_row < data_width && cur_col < data_height) {
                local_data[i * local_width + j] = data_in[cur_row * data_width + cur_col];
            }
        }
    }

    // Ensure all threads in the workgroup finish updating the local memory
    barrier(CLK_LOCAL_MEM_FENCE);

    // Perform the convolution only for workitems mapping to locations within
    // the data range padding shell.
    if (global_row < data_height - filter_padding &&
        global_col < data_width - filter_padding) {
        int sum = 0;
        int filter_ix = 0;

        // Convolution loop
        for (int i = local_row; i < local_row + FILTER_WIDTH; ++i) {
            int offset = i * local_width;
            for (int j = local_col; j < local_col + FILTER_WIDTH; ++j) {
                sum += local_data[offset + j] * filter_mask[filter_ix++];
            }
        }

        // Write the output data
        int data_ix = (global_row + filter_radius) * data_width +
                      (global_col + filter_radius);
        data_out[data_ix] = sum;
    }
}