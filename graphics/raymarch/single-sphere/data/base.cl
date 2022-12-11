#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
#error "Double precision floating point not supported by OpenCL implementation."
#endif

#define kEmpty          0xffffffff

/**
 * @brief Sphere data type.
 */
typedef struct {
    float4 centre;
    float radius;
} Sphere_t;

/**
 * @brief Ray data type, starting at origin o and with direction d.
 */
typedef struct {
    float4 o;
    float4 d;
} Ray_t;

/**
 * @brief Ray-object intersection data type.
 */
typedef struct {
    float4 p;
    float4 n;
    float t;
} Isect_t;
