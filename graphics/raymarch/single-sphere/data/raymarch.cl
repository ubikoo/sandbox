/** @brief March a ray through the sphere-plane scene. */
Isect_t march(
    const Ray_t ray,
    const float t_min,
    const float t_max,
    const uint maxsteps,
    const __global Sphere_t *sphere);

/** @brief Compute the normal. */
float4 compute_normal(const float4 p, const __global Sphere_t *sphere);

/** @brief Compute the minimum signed distance function. */
float compute_sdf(const float4 p, const __global Sphere_t *sphere);

/** --------------------------------------------------------------------------
 * raymarch
 * @brief Render the spheres using a sphere tracing algorithm.
 */
__kernel void raymarch(
    const uint width,
    const uint height,
    const float depth,
    const float time,
    const float t_min,
    const float t_max,
    const uint maxsteps,
    const __global Sphere_t *sphere,
    __write_only image2d_t image)
{
   /* Compute normalized pixel coordinates. */
    const uint idx = get_global_id(0);       // global pos in x-direction
    const uint idy = get_global_id(1);       // global pos in y-direction
    if (!(idx < width && idy < height)) {
        return;
    }

    /*
     * Generate a ray passing through the specified pixel.
     */
    Ray_t ray;
    {
        float2 uv = (float2) ((float) idx / width, (float) idy / height);
        uv -= (float2) (0.5f, 0.5f);
        float aspect = (float) width / height;
        if (aspect < 1.0) {
            uv.x *= aspect;
        } else {
            uv.y /= aspect;
        }
        ray.o = (float4) (0.0f, 1.0f, 0.0f, 0.0f);
        ray.d = (float4) (uv.x, uv.y, -depth, 0.0f/*unused*/);
        ray.d = normalize(ray.d);
    }

    /*
     * March the ray and illuminate the intersection point.
     */
    Isect_t isect = march(ray, t_min, t_max, maxsteps, sphere);

    float4 color = (float4) (0.0f);
    {
        float4 light_pos = (float4) (0.0f, 5.0f, -6.0f, 0.0f);
        light_pos.xz += (float2) (2.0f * sin(time), 2.0f * cos(time));

        float4 light_dir = normalize(light_pos - isect.p);
        float light_dist = length(light_pos - isect.p);

        Ray_t light_ray;
        light_ray.o = isect.p + (2.0f * t_min) * isect.n;
        light_ray.d = light_dir;

        Isect_t light_isect = march(light_ray, t_min, t_max, maxsteps, sphere);
        float diffuse = clamp(dot(isect.n, light_dir), 0.0f, 1.0f);
        if (light_isect.t < light_dist) {
            diffuse *= 0.1f;
        }

        // color = isect.n;
        color = (float4) (diffuse, diffuse, diffuse, diffuse);
        color = pow(color, (float4) (.4545));	// gamma correction
    }

    write_imagef(image, (int2) (idx, idy), color);
}

/** --------------------------------------------------------------------------
 * march
 * @brief March a ray through the sphere-plane scene.
 */
Isect_t march(
    const Ray_t ray,
    const float t_min,
    const float t_max,
    const uint maxsteps,
    const __global Sphere_t *sphere)
{
    float4 p = ray.o;
    float t = compute_sdf(p, sphere);
    ulong step = 0;

    while (fabs(t) > t_min && fabs(t) < t_max && step < maxsteps) {
        p += t * ray.d;
        t = compute_sdf(p, sphere);
        step++;
    }

    Isect_t isect;
    isect.p = p;
    isect.n = compute_normal(p, sphere);
    isect.t = t;
    return isect;
}

/** --------------------------------------------------------------------------
 * compute_normal
 * @brief Compute the normal
 */
float4 compute_normal(const float4 p, const __global Sphere_t *sphere)
{
    float eps = 0.01f;
    float4 ex = (float4) (eps, 0.0f, 0.0f, 0.0f);
    float4 ey = (float4) (0.0f, eps, 0.0f, 0.0f);
    float4 ez = (float4) (0.0f, 0.0f, eps, 0.0f);

    float t  = compute_sdf(p, sphere);
    float tx = compute_sdf(p + ex, sphere);
    float ty = compute_sdf(p + ey, sphere);
    float tz = compute_sdf(p + ez, sphere);

    float4 n = (float4) (tx - t, ty - t, tz - t, 0.0f);
    return normalize(n);
}

/** --------------------------------------------------------------------------
 * compute_sdf
 * @brief Compute the minimum signed distance function of the specified point.
 */
float compute_sdf(const float4 p, const __global Sphere_t *sphere)
{
    float t_sphere = length(p - sphere->centre) - sphere->radius;
    float t_plane = fabs(p.y);
    return min(t_sphere, t_plane);
}
