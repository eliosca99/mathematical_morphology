#include "kernels.cuh"

__constant__ int2 d_se[MAX_SE_SIZE]; // SE salvato come array di offset con int2, dx e dy

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
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= left && x < (width - right) && y >= top && y < (height - bottom)) {
        bool match = true;
        int base = y * width + x;

        for (int k = 0; k < numOffsets; k++) {
            int2 offset = d_se[k];

            if (d_in[base + (offset.y * width) + offset.x] == 0) {
                match = false;
                break;
            }
        }
        if (match) {
            d_out[base] = 1;
        }
    }
}

__global__ void dilationNaiveKernel(
    const unsigned char* __restrict__ d_in,
    unsigned char* __restrict__ d_out,
    int width,
    int height,
    int numOffsets
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < height) {
        bool match = false;
    
        for (int k = 0; k < numOffsets; k++) {
            int2 offset = d_se[k];
            int cur_x = x - offset.x;
            int cur_y = y - offset.y;
            if (cur_x >= 0 && cur_x < width && cur_y >= 0 && cur_y < height) {
                if (d_in[(cur_y * width) + cur_x] == 1) {
                    match = true;
                    break;
                }
            }
        }
        if (match) {
            d_out[y * width + x] = 1;
        }

    }
}

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
) {
    extern __shared__ unsigned char s_tile[];

    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    int tid = threadIdx.y * blockDim.x + threadIdx.x;
    int numThreads = blockDim.x * blockDim.y;

    int tile_w = left + right + blockDim.x;
    int tile_h = top + bottom + blockDim.y;
    int shared_size = tile_w * tile_h;

    int tile_start_x = blockIdx.x * blockDim.x - left;
    int tile_start_y = blockIdx.y * blockDim.y - top;

    for (int t = tid; t < shared_size; t += numThreads) {
        int s_x = t % tile_w;
        int s_y = t / tile_w;
        int g_x = tile_start_x + s_x;
        int g_y = tile_start_y + s_y;

        if (g_x >= 0 && g_x < width && g_y >= 0 && g_y < height) {
            s_tile[t] = d_in[g_y * width + g_x];
        } else {
            s_tile[t] = 1;
        }
    }

    __syncthreads();

    if (x < width && y < height) {
        int local_x = threadIdx.x + left;
        int local_y = threadIdx.y + top;
        bool match = true;

        for (int k = 0; k < numOffsets; k++) {
            int2 offset = d_se[k];
            int target_x = local_x + offset.x;
            int target_y = local_y + offset.y;

            if (s_tile[target_y * tile_w + target_x] == 0) {
                match = false;
                break;
            } 
        }
        if (match) {
            d_out[y * width + x] = 1;
        }
    }
}

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
) {
    extern __shared__ unsigned char s_tile[];

    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    int tid = threadIdx.y * blockDim.x + threadIdx.x;
    int numThreads = blockDim.x * blockDim.y;

    int tile_w = left + right + blockDim.x;
    int tile_h = top + bottom + blockDim.y;
    int shared_size = tile_w * tile_h;

    int tile_start_x = blockIdx.x * blockDim.x - right;
    int tile_start_y = blockIdx.y * blockDim.y - bottom;

    for (int t = tid; t < shared_size; t += numThreads) {
        int s_x = t % tile_w;
        int s_y = t / tile_w;
        int g_x = tile_start_x + s_x;
        int g_y = tile_start_y + s_y;

        if (g_x >= 0 && g_x < width && g_y >= 0 && g_y < height) {
            s_tile[t] = d_in[g_y * width + g_x];
        } else {
            s_tile[t] = 0;
        }
    }

    __syncthreads();

    if (x < width && y < height) {
        int local_x = threadIdx.x + right;
        int local_y = threadIdx.y + bottom;
        bool match = false;

        for (int k = 0; k < numOffsets; k++) {
            int2 offset = d_se[k];
            int target_x = local_x - offset.x;
            int target_y = local_y - offset.y;

            if (s_tile[target_y * tile_w + target_x] == 1) {
                match = true;
                break;
            } 
        }
        if (match) {
            d_out[y * width + x] = 1;
        }
    }
}

void copy_se_to_constant(StructuringElementWithOffsets* SE) {
    int2 offsets[SE->numOffsets];
    for (int i = 0; i < SE->numOffsets; i++) {
        offsets[i].x = SE->offsets[i].dx;
        offsets[i].y = SE->offsets[i].dy;
    }
    cudaMemcpyToSymbol(d_se, offsets, SE->numOffsets * sizeof(int2));
}