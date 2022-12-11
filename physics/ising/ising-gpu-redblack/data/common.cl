#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
#error "cl_double precision floating point not supported by OpenCL implementation."
#endif

#define kEmpty  0xffffffff
