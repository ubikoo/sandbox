/**
 * thermostat_force
 * @brief Compute the thermostat force.
 */
__kernel void thermostat_force(
    const uint n_atoms,
    const __global Atom_t *atoms,
    __global double *therm_group_grad_sq,
    __global double *therm_group_laplace,
    __local double *therm_local_grad_sq,
    __local double *therm_local_laplace)
{
    // const uint global_size = get_global_size(0);
    const uint global_id = get_global_id(0);
    const uint local_size = get_local_size(0);
    const uint local_id = get_local_id(0);
    const uint group_id = get_group_id(0);

    /* Setup the local atom data. */
    therm_local_grad_sq[local_id] = 0.0;
    therm_local_laplace[local_id] = 0.0;
    if (global_id < n_atoms) {
        double4 mom = atoms[global_id].mom;
        double rmass = atoms[global_id].rmass;
        therm_local_grad_sq[local_id] = dot(mom, mom) * rmass;
        therm_local_laplace[local_id] = 3.0;
    }

    /* Sum the array using a divide and conquer algorithm. */
    for (uint stride = local_size >> 1; stride > 0; stride >>= 1) {
        barrier(CLK_LOCAL_MEM_FENCE);
        if (local_id < stride) {
            therm_local_grad_sq[local_id] += therm_local_grad_sq[local_id + stride];
            therm_local_laplace[local_id] += therm_local_laplace[local_id + stride];
        }
    }

    if (local_id == 0) {
        therm_group_grad_sq[group_id] = therm_local_grad_sq[0];
        therm_group_laplace[group_id] = therm_local_laplace[0];
    }
}

/**
 * thermostat_integrate
 * @brief Integrate the thermostat EoM.
 */
__kernel void thermostat_integrate(
    const double t_step,
    __global double *therm_group_grad_sq,
    __global double *therm_group_laplace,
    __global Thermostat_t *thermostat)
{
    const uint global_id = get_global_id(0);
    if (global_id == 0) {
        double grad_sq = 0.0;
        double laplace = 0.0;
        for (uint i = 0; i < get_num_groups(0); ++i) {
            grad_sq += therm_group_grad_sq[i];
            laplace += therm_group_laplace[i];
        }

        double force = grad_sq - thermostat->temperature * laplace;
        thermostat->deta_dt = force / thermostat->mass;
        thermostat->eta += t_step * thermostat->deta_dt;
    }
}
