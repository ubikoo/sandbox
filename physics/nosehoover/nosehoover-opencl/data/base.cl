#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
#error "Double precision floating point not supported by OpenCL implementation."
#endif

#define kEmpty  0xffffffff

/** ---------------------------------------------------------------------------
 * @brief NoseHoover spring-thermostat parameters.
 */
typedef struct {
    double mass;            /* mass */
    double kappa;           /* force constant */
    double tau;             /* thermostat constant */
    double temperature;     /* thermostat temperature */
} NoseHooverParam_t;

/**
 * @brief NoseHoover spring-thermostat data type.
 */
typedef struct {
    double pos;             /* spring position */
    double mom;             /* spring momentum */
    double eta;             /* thermostat state */
    float4 color;           /* color attribute */
} NoseHoover_t;

