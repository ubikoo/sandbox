
/** @brief Compute the collision between the vector (p_1 - p_0)
 * and a plane passing through ctr with normal nor.
 */
bool collide(
    const float4 p_0,
    float4 *p_1,
    float4 *vel,
    const float4 ctr,
    const float4 nor,
    const float friction,
    const float elastic);

/**
 * @brief Compute the intersection parameter between vector (p_1 - p_0)
 * and a plane passing through ctr with normal nor.
 */
bool intersect(
    const float4 p_0,
    const float4 p_1,
    const float4 ctr,
    const float4 nor,
    float *t_isect);

/** ---------------------------------------------------------------------------
 * update_boundaries
 * @brief Apply boundary conditions and update particle positions.
 */
__kernel void update_boundaries(
    const uint n_particles,
    const float friction,
    const float elastic,
    __global Particle_t *particles,
    const __global Domain_t *domain)
{
    const uint id = get_global_id(0);
    if (id >= n_particles) {
        return;
    }

    const float4 bound_lo = domain->bound_lo;
    const float4 bound_hi = domain->bound_hi;

    float4 p_0 = particles[id].prev;
    p_0.w = 0.0f;
    float4 p_1 = particles[id].pos;
    p_1.w = 0.0f;
    float4 vel = particles[id].vel;
    vel.w = 0.0f;

    float4 nx_lo = (float4) (-1.0f, 0.0f, 0.0f, 0.0f);
    float4 ny_lo = (float4) ( 0.0f,-1.0f, 0.0f, 0.0f);
    float4 nz_lo = (float4) ( 0.0f, 0.0f,-1.0f, 0.0f);

    float4 nx_hi = (float4) ( 1.0f, 0.0f, 0.0f, 0.0f);
    float4 ny_hi = (float4) ( 0.0f, 1.0f, 0.0f, 0.0f);
    float4 nz_hi = (float4) ( 0.0f, 0.0f, 1.0f, 0.0f);

    bool has_collision = true;
    while (has_collision) {
        has_collision = false;

        /* lo-boundary */
        has_collision |= collide(p_0, &p_1, &vel, bound_lo, nx_lo, friction, elastic);
        has_collision |= collide(p_0, &p_1, &vel, bound_lo, ny_lo, friction, elastic);
        has_collision |= collide(p_0, &p_1, &vel, bound_lo, nz_lo, friction, elastic);

        /* hi-boundary */
        has_collision |= collide(p_0, &p_1, &vel, bound_hi, nx_hi, friction, elastic);
        has_collision |= collide(p_0, &p_1, &vel, bound_hi, ny_hi, friction, elastic);
        has_collision |= collide(p_0, &p_1, &vel, bound_hi, nz_hi, friction, elastic);
    }
    particles[id].pos = p_1;
    particles[id].vel = vel;
}

/**
 * collide
 * @brief Compute the collision between the vector (p_1 - p_0)
 * and a plane passing through ctr with normal nor.
 * If the vector intersects the plane, reflect the end position p_1 and
 * the velocity.
 */
bool collide(
    const float4 p_0,
    float4 *p_1,
    float4 *vel,
    const float4 ctr,
    const float4 nor,
    const float friction,
    const float elastic)
{
    float t;
    if (!intersect(p_0, *p_1, ctr, nor, &t)) {
        return false;
    }

    float4 p_isect = p_0 + t * (*p_1 - p_0);
    *p_1 -= 2.0f * dot(nor, *p_1 - p_isect) * nor;

    float4 v_nor = dot(nor, *vel) * nor;
    float4 v_par = *vel - v_nor;
    *vel = (friction * v_par) - (elastic * v_nor);

    return true;
}


/**
 * intersect
 * @brief Compute the intersection parameter between vector (p_1 - p_0)
 * and a plane passing through ctr with normal nor.
 *
 * Assuming p_0 is on the negative side of the plane,
 *      dot(p_0 - ctr, nor) < 0
 * if p_1 is on the positive side of the plane,
 *      dot(p_1 - ctr, nor) > 0,
 * then compute the intersection between (p_1 - p_0) and the plane:
 *      0 = dot(p_0 + t*(p_1 - p_0) - ctr, nor)
 *        = dot(p_0 - ctr, nor) + t*dot(p_1 - p_0, nor)
 *      t = -dot(p_0 - ctr, nor) / dot(p_1 - p_0, nor)
 *
 * If p_1 is inside the domain, dot(p_1 - ctr, nor) < 0.0,
 * or (p_1 - p_0) is parallel, dot(p_1 - p_0, nor) = 0.0, there
 * is no intersection.
 */
bool intersect(
    const float4 p_0,
    const float4 p_1,
    const float4 ctr,
    const float4 nor,
    float *t_isect)
{
    if (dot(p_1 - ctr, nor) > 0.0f) {
        float numer = dot(p_0 - ctr, nor);
        float denom = dot(p_1 - p_0, nor);
        if (fabs(denom) > 0.0f) {
            *t_isect = -numer / denom;
            return true;
        }
    }
    return false;
}

/** ---------------------------------------------------------------------------
 * copy_vertex_data
 * @brief Copy the particle positions onto the vertex buffer object.
 */
__kernel void copy_vertex_data(
    const uint n_particles,
    __global float *vertex,
    const __global Particle_t *particles)
{
    const uint id = get_global_id(0);
    if (id < n_particles) {
        vertex[3*id + 0] = particles[id].pos.x;
        vertex[3*id + 1] = particles[id].pos.y;
        vertex[3*id + 2] = particles[id].pos.z;
    }
}
