/** @brief Kernel used for everything except pressure and viscosity. */
float kernel_density(float r, float h);

/** @brief Kernel with negative gradient for every value 0 <= r <= h. */
float kernel_pressure(float r, float h);

/** @brief Kernel with positive laplacian for every value 0 <= r <= h. */
float kernel_viscosity(float r, float h);

/** ---------------------------------------------------------------------------
 * @brief Kernel used for everything except pressure and viscosity.
 */
float kernel_density(float r, float h)
{
    if (r < h) {
        const float r2 = r * r;
        const float h2 = h * h;
        const float h3 = h * h * h;
        const float h9 = h3 * h3 * h3;
        const float C = 315.0f / (64.0f * M_PI * h9);

        float z = h2 - r2;
        return C * z * z * z;
    }
    return 0.0;
}

/** ---------------------------------------------------------------------------
 * @brief Kernel with negative gradient for every value 0 <= r <= h.
 */
float kernel_pressure(float r, float h)
{
    if (r > 0.0 && r < h) {
        const float h2 = h * h;
        const float h6 = h2 * h2 * h2;
        const float C = -45.0f / (M_PI * h6);

        float z = h - r;
        return C * z * z;
    }
    return 0.0;
}

/** ---------------------------------------------------------------------------
 * @brief Kernel with positive laplacian for every value 0 <= r <= h.
 */
float kernel_viscosity(float r, float h)
{
    if (r < h) {
        const float h2 = h * h;
        const float h6 = h2 * h2 * h2;
        const float C = 45.0f / (M_PI * h6);

        float z = h - r;
        return C * z;
    }
    return 0.0;
}
