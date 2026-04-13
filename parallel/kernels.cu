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
) {
    extern __shared__ unsigned char s_tile[];

    int byteX = blockIdx.x * blockDim.x + threadIdx.x;
    int row   = blockIdx.y * blockDim.y + threadIdx.y;

    int leftHaloBytes = (left + 7) / 8;
    int rightHaloBytes = right / 8;

    int tile_byte_w = leftHaloBytes + rightHaloBytes + blockDim.x + 1;
    int tile_h = top + bottom + blockDim.y;

    int shared_size = tile_byte_w * tile_h;

    int tile_start_byte_x = blockIdx.x * blockDim.x - leftHaloBytes;
    int tile_start_row = blockIdx.y * blockDim.y - top;

    int tid = threadIdx.y * blockDim.x + threadIdx.x;
    int numThreads = blockDim.x * blockDim.y;

    for (int i = tid; i < shared_size; i += numThreads) {
        int s_x = i % tile_byte_w;
        int s_y = i / tile_byte_w;

        int g_x = tile_start_byte_x + s_x;
        int g_y = tile_start_row + s_y;

        if (g_x >= 0 && g_x < rowStride && g_y >= 0 && g_y < height) {
            s_tile[i] = d_in[g_y * rowStride + g_x];
        } else {
            s_tile[i] = 0;
        }
    }
    __syncthreads();

    if (byteX >= rowStride || row >= height) return;

    int local_row = threadIdx.y + top;
    int local_byte_x = threadIdx.x + leftHaloBytes;
    unsigned char acc = 0xFF;

    for (int k = 0; k < numOffsets; k++) {
        int2 offset = d_se[k];
        int dxBytes, dxBits;
        if(offset.x >= 0) {
            dxBytes = offset.x / 8;
            dxBits = offset.x % 8;
        } else {
            dxBytes = (offset.x - 7) / 8;
            dxBits = offset.x - dxBytes * 8;
        }
        int src_x = local_byte_x + dxBytes;
        int src_y = local_row + offset.y;

        unsigned char byteA = s_tile[src_y * tile_byte_w + src_x];
        unsigned char val;

        if (dxBits == 0) {
            val = byteA;
        } else {
            unsigned char byteB = s_tile[src_y * tile_byte_w + src_x + 1];
            val = (byteA << dxBits) | (byteB >> (8 - dxBits));
        }
        acc &= val;
        if (acc == 0) break;
    }
    d_out[row * rowStride + byteX] = acc;
}

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
) {
    int byteX = blockIdx.x * blockDim.x + threadIdx.x;
    int row   = blockIdx.y * blockDim.y + threadIdx.y;

    int tid        = threadIdx.y * blockDim.x + threadIdx.x;
    int numThreads = blockDim.x * blockDim.y;

    // halo invertito rispetto all'erosione
    int leftHaloBytes  = right / 8 + 1;
    int rightHaloBytes = (left + 7) / 8;

    int tile_byte_w = leftHaloBytes + blockDim.x + rightHaloBytes;
    int tile_h      = bottom + blockDim.y + top;
    int shared_size = tile_byte_w * tile_h;

    // tile_start si sposta in direzione opposta all'erosione
    int tile_start_byteX = blockIdx.x * blockDim.x - leftHaloBytes;
    int tile_start_row   = blockIdx.y * blockDim.y - bottom;

    extern __shared__ unsigned char s_tile[];

    // caricamento cooperativo identico all'erosione
    for (int t = tid; t < shared_size; t += numThreads) {
        int s_x = t % tile_byte_w;
        int s_y = t / tile_byte_w;
        int g_x = tile_start_byteX + s_x;
        int g_y = tile_start_row   + s_y;

        if (g_x >= 0 && g_x < rowStride && g_y >= 0 && g_y < height)
            s_tile[t] = d_in[g_y * rowStride + g_x];
        else
            s_tile[t] = 0;
    }

    __syncthreads();

    if (byteX >= rowStride || row >= height) return;

    // local_row usa bottom invece di top
    int local_row   = threadIdx.y + bottom;
    int local_byteX = threadIdx.x + leftHaloBytes;

    unsigned char acc = 0x00;

    for (int k = 0; k < numOffsets; k++) {
        int dx = d_se[k].x;
        int dy = d_se[k].y;

        int dxBytes, dxBits;
        if (dx >= 0) {
            dxBytes = dx / 8;
            dxBits  = dx % 8;
        } else {
            dxBytes = (dx - 7) / 8;
            dxBits  = dx - dxBytes * 8;
        }

        // direzione opposta all'erosione: - invece di +
        int s_y = local_row   - dy;
        int s_x = local_byteX - dxBytes;

        unsigned char byteA = s_tile[s_y * tile_byte_w + s_x];

        unsigned char val;
        if (dxBits == 0) {
            val = byteA;
        } else {
            // byteB sta a SINISTRA di byteA (- invece di +)
            unsigned char byteB = s_tile[s_y * tile_byte_w + s_x - 1];
            val = (byteA >> dxBits) | (byteB << (8 - dxBits));
        }

        acc |= val;
        if (acc == 0xFF) break;
    }

    d_out[row * rowStride + byteX] = acc;
}

void copy_se_to_constant(StructuringElementWithOffsets* SE) {
    int2 offsets[SE->numOffsets];
    for (int i = 0; i < SE->numOffsets; i++) {
        offsets[i].x = SE->offsets[i].dx;
        offsets[i].y = SE->offsets[i].dy;
    }
    cudaMemcpyToSymbol(d_se, offsets, SE->numOffsets * sizeof(int2));
}