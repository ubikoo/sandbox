#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
#error "Double precision floating point not supported by OpenCL implementation."
#endif

#define kEmpty          0xffffffff
#define kBoundaryOpen   0
#define kBoundaryClosed 1
#define kFluid          0
#define kSolid          1

/**
 * @brief Fluid domain.
 */
typedef struct {
    float4 bound_lo;        /* domain lower bound */
    float4 bound_hi;        /* domain upper bound */
} Domain_t;

/**
 * @brief Fluid particles.
 */
typedef struct {
    float4 prev;            /* previous position */
    float4 pos;             /* position */
    float4 vel;             /* velocity */
    float4 force;           /* force */
    float mass;             /* mass */
    float dens;             /* density */
    float pres;             /* pressure from EoS, p = p(rho) */
} Particle_t;
