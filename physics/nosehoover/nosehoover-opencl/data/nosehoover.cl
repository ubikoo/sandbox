/**  @brief Compute Nose-Hoover time derivative. */
double4 deriv(
    const double mass,
    const double kappa,
    const double tau,
    const double temperature,
    const double4 therm);

/** ---------------------------------------------------------------------------
 * @brief Integrate Nose-Hoover thermostat.
 */
__kernel void integrate(
    const double t_step,
    const double max_err,
    const uint max_iter,
    const uint n_thermostats,
    const __global NoseHooverParam_t *param,
    __global NoseHoover_t *thermostat)
{
    const uint therm_id = get_global_id(0);
    if (therm_id < n_thermostats) {
        /* Unpack thermostat parameters. */
        const double mass = param->mass;
        const double kappa = param->kappa;
        const double tau = param->tau;
        const double temp = param->temperature;

        /* Fixed point iteration of Gauss-Legendre formula. */
        double4 x_beg = (double4)(
            thermostat[therm_id].pos,
            thermostat[therm_id].mom,
            thermostat[therm_id].eta,
            0.0 /* unused */);
        double4 x_new = x_beg;

        double err = DBL_MAX;
        uint n_iter = 0;

        while (err > max_err && n_iter < max_iter) {
            double4 x_mid = 0.5 * (x_beg + x_new);
            double4 dx_dt = deriv(mass, kappa, tau, temp, x_mid);

            double4 x_old = x_new;
            x_new = x_beg + t_step * dx_dt;
            err = length(x_new - x_old);

            ++n_iter;
        }

        thermostat[therm_id].pos = x_new.s0;
        thermostat[therm_id].mom = x_new.s1;
        thermostat[therm_id].eta = x_new.s2;
    }
}

/**
 * @brief Compute Nose-Hoover time derivative.
 */
double4 deriv(
    const double mass,
    const double kappa,
    const double tau,
    const double temp,
    const double4 therm)
{
    double veloc = therm.s1 / mass;
    double force = -kappa * therm.s0 - therm.s2 * therm.s1;
    double etadot = (therm.s1 * therm.s1 / (mass * temp) - 1.0) / tau;
    return (double4) (veloc, force, etadot, 0.0 /*unused*/);
}

/** ---------------------------------------------------------------------------
 * @brief Reset the canvas cells.
 */
__kernel void reset_canvas(
    const uint canvas_width,
    const uint canvas_height,
    __global uint *canvas)
{
    const long x = get_global_id(0);    // global pos in x-direction
    const long y = get_global_id(1);    // global pos in y-direction
    if (x < canvas_width && y < canvas_height) {
        uint slot = y * canvas_width + x;
        canvas[slot] = kEmpty;
    }
}

/** ---------------------------------------------------------------------------
 * @brief Compute depth test of the Nose-Hoover thermostats. For each
 * thermostat with (pos, mom, eta), compute the normalized coordinates (u, v)
 * and corresponding cell coordinates (iu, iv). For a give canvas cell, store
 * the index of the thermostat whose thermostat eta value is largest.
 */
__kernel void depth_canvas(
    const uint n_thermostats,
    const uint canvas_width,
    const uint canvas_height,
    const double canvas_x_range,
    const double canvas_y_range,
    const __global NoseHoover_t *thermostat,
    __global uint *canvas)
{
    const uint therm_id = get_global_id(0);
    if (therm_id < n_thermostats) {
        double pos = thermostat[therm_id].pos;
        double mom = thermostat[therm_id].mom;
        double eta = thermostat[therm_id].eta;
        // float4 c = thermostat[therm_id].color;
        // printf(" depth_canvas color %2.2v4hlf\n", c);

        if (2.0 * fabs(pos) < canvas_x_range &&
            2.0 * fabs(mom) < canvas_y_range) {
            double x = 0.5 + pos / canvas_x_range;
            double y = 0.5 + mom / canvas_y_range;

            uint ix = (uint) (x * canvas_width);
            uint iy = (uint) (y * canvas_height);
            uint slot = iy * canvas_width + ix;

            while (true) {
                uint cur_id = canvas[slot];

                if (cur_id == kEmpty) {
                    uint old_id = atom_cmpxchg(
                        (volatile __global uint *) (&canvas[slot]),
                        kEmpty,
                        therm_id);

                    if (old_id == kEmpty) {
                        return;
                    }
                } else {
                    double cur_eta = thermostat[cur_id].eta;
                    uint new_id = eta > cur_eta ? therm_id : cur_id;
                    uint old_id = atom_cmpxchg(
                        (volatile __global uint *) (&canvas[slot]),
                        cur_id,
                        new_id);

                    if (old_id == cur_id) {
                        // printf(" therm_id %u, eta %lf", therm_id, eta);
                        // printf(" cur_id %u, cur_eta %lf,", cur_id, cur_eta);
                        // printf(" new_id %u, old_id %u\n", new_id, old_id);
                        return;
                    }
                }
            }
        }
    }
}

/** ---------------------------------------------------------------------------
 * @brief Draw the canvas to image.
 */
__kernel void draw_canvas(
    const uint canvas_width,
    const uint canvas_height,
    __write_only image2d_t image,
    const __global NoseHoover_t *thermostat,
    const __global uint *canvas)
{
    const uint x = get_global_id(0);    // global pos in x-direction
    const uint y = get_global_id(1);    // global pos in y-direction
    if (x < canvas_width && y < canvas_height) {
        uint slot = y * canvas_width + x;
        uint therm_id = canvas[slot];
        float4 color = (therm_id != kEmpty)
            ? thermostat[therm_id].color
            : (float4) (0.25f, 0.25f, 0.25f, 1.0f);
        write_imagef(image, (int2) (x, y), color);
    }
}
