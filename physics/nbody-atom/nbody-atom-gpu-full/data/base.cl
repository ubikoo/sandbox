#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
#error "Double precision floating point not supported by OpenCL implementation."
#endif

#define kEmpty  0xffffffff

/** ---------------------------------------------------------------------------
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
 * @brief Pair data type.
 */
typedef struct {
    ulong atom_1;           /* first atom index */
    ulong atom_2;           /* second atom index */
    double4 r_12;           /* pairwise vector */
    double energy;          /* energy */
    double4 gradient;       /* gradient */
    double4 laplace;        /* laplace */
    double16 virial;        /* virial */
} Pair_t;

/**
 * @brief Domain data type.
 */
typedef struct {
    double4 length;         /* domain period length */
    double4 length_half;    /* domain half length */
} Domain_t;

/**
 * @brief Field data type.
 */
typedef struct {
    double epsilon;         /* LJ pair energy */
    double sigma;           /* LJ pair size */
    double r_cut;           /* LJ cutoff radius */
    double r_skin;          /* Neighbour list skin radius */
    double r_hard;          /* Hard sphere truncation radius */
} Field_t;

/**
 * Thermostat data type.
 */
typedef struct {
    double mass;            /* mass */
    double eta;             /* velocity */
    double deta_dt;         /* acceleration */
    double temperature;     /* temperature */
} Thermostat_t;

/**
 * Thermo data type.
 */
typedef struct {
    double com_mass;        /* Total mass of the fluid */
    double4 com_pos;        /* CoM position of the fluid */
    double4 com_upos;       /* CoM unfolded position of the fluid */
    double4 com_vel;        /* CoM velocity of the fluid */
    double4 com_mom;        /* Total momentum of the fluid */
    double4 com_force;      /* Total force  of the fluid */
    double density;         /* Mass density */
    double energy_kin;      /* Kinetic energy */
    double energy_pot;      /* Potential energy */
    double temp_grad_sq;    /* Kinetic energy square gradient */
    double temp_laplace;    /* Kinetic energy laplacian */
    double temp_kinetic;    /* Kinetic temperature */
    double16 pres_kinetic;  /* Kinetic pressure */
    double16 pres_virial;   /* Virial pressure */
} Thermo_t;
