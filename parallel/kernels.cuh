#ifndef KERNELS_CUH
#define KERNELS_CUH

#define MAX_SE_DIM 37
#define MAX_SE_SIZE (MAX_SE_DIM * MAX_SE_DIM)

__global__ void erosionKernel(
    const unsigned char* d_in,
    unsigned char* d_out,
    int width,
    int height,
    const int2* d_offset,
    int numOffsets
);

__global__ void erosionByteImageKernel(
    const unsigned char* d_in,
    unsigned char* d_out,
    int width,
    int height,
    const int2* d_offset,
    int numOffsets
);

__global__ void erosionUint64ImageKernel(
    const unsigned char* d_in,
    unsigned char* d_out,
    int width,
    int height,
    const int2* d_offset,
    int numOffsets
);

void launchErosionKernel(
    const unsigned char* d_in,
    unsigned char* d_out,
    int width,
    int height,
    const int2* d_offsets,
    int numOffsets
);

void copy_se_to_constant(char* h_se, int se_bytes);

#endif
