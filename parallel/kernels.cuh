#ifndef KERNELS_CUH
#define KERNELS_CUH

#include "sequential/image_utils.h"

#define MAX_SE_DIM 37
#define MAX_SE_SIZE (MAX_SE_DIM * MAX_SE_DIM)

__global__ void erosionKernel(
    const unsigned char* __restrict__ d_in,
    unsigned char* __restrict__ d_out,
    int width,
    int height,
    int numOffsets,
    int top, 
    int bottom,
    int left,
    int right
);

__global__ void dilationKernel(
    const unsigned char* __restrict__ d_in,
    unsigned char* __restrict__ d_out,
    int width,
    int height,
    int numOffsets,
    int top,
    int bottom,
    int left,
    int right
);

__global__ void erosionNaiveKernel(
    const unsigned char* __restrict__ d_in,
    unsigned char* __restrict__ d_out,
    int width,
    int height,
    int numOffsets,
    int top,
    int bottom,
    int left,
    int right
);

__global__ void dilationNaiveKernel(
    const unsigned char* __restrict__ d_in,
    unsigned char* __restrict__ d_out,
    int width,
    int height,
    int numOffsets
);

__global__ void erosionByteImageKernel(
    const unsigned char* __restrict__ d_in,
    unsigned char* __restrict__ d_out,
    int width,
    int height,
    int rowStride,
    int numOffsets,
    int top,
    int bottom,
    int left,
    int right
);

__global__ void dilationByteImageKernel(
    const unsigned char* __restrict__ d_in,
    unsigned char* __restrict__ d_out,
    int width,
    int height,
    int rowStride,
    int numOffsets,
    int top,
    int bottom,
    int left,
    int right
);

__global__ void erosionUint64ImageKernel(
    const unsigned char* __restrict__ d_in,
    unsigned char* __restrict__ d_out,
    int width,
    int height,
    const int2* __restrict__ d_offset,
    int numOffsets
);

__global__ void dilationUint64ImageKernel(
    const unsigned char* __restrict__ d_in,
    unsigned char* __restrict__ d_out,
    int width,
    int height,
    const int2* __restrict__ d_offset,
    int numOffsets
);

void copy_se_to_constant(StructuringElementWithOffsets* SE);

#endif
