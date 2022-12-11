#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
#error "Double precision floating point not supported by OpenCL implementation."
#endif

#define kEmpty  0xffffffff

/**
 * @brief Domain data type.
 */
typedef struct {
    double4 length;         /* domain period length */
    double4 length_half;    /* domain half length */
} Domain_t;

/**
 * @brief Atom data type.
 */
typedef struct {
    double mass;            /* atom mass */
    double rmass;           /* inverse mass */
    double4 pos;            /* periodic image in primary cell */
    double4 upos;           /* unfolded position */
    double4 mom;            /* momentum */
    double4 force;          /* force */
    double energy;          /* energy */
    double16 virial;        /* virial */
} Atom_t;

/**
 * @brief Field data type.
 */
typedef struct {
    double epsilon;         /* LJ pair energy */
    double sigma;           /* LJ pair size */
    double r_cut;           /* LJ cutoff radius */
    double r_hard;          /* LJ hard sphere radius */
} Field_t;

/**
 * @brief Pair data type.
 */
typedef struct {
    double4 r_12;           /* pairwise vector */
    double energy;          /* energy */
    double4 gradient;       /* gradient */
    double4 laplace;        /* laplace */
    double16 virial;        /* virial */
} Pair_t;

/**
 * @brief Thermostat data type.
 */
typedef struct {
    double mass;            /* mass */
    double eta;             /* velocity */
    double deta_dt;         /* acceleration */
    double temperature;     /* temperature */
} Thermostat_t;
