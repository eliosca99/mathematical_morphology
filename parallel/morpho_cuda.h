#ifndef MORPHO_CUDA_H
#define MORPHO_CUDA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

extern double global_cuda_time_ms;
#ifdef __CUDACC__
#include <cuda_runtime.h>
#define CUDA_CHECK(call)                                                      \
    do {                                                                      \
        cudaError_t err = call;                                               \
        if (err != cudaSuccess) {                                             \
            fprintf(stderr, "Errore CUDA fatale!\n");                         \
            fprintf(stderr, "File: %s\n", __FILE__);                          \
            fprintf(stderr, "Riga: %d\n", __LINE__);                          \
            fprintf(stderr, "Codice errore: %d\n", err);                      \
            fprintf(stderr, "Descrizione: %s\n", cudaGetErrorString(err));    \
            exit(EXIT_FAILURE);                                               \
        }                                                                     \
    } while (0)
#endif

#include "sequential/image_utils.h"

int erosionCuda(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y);
int dilationCuda(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y);
int openingCuda(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y);
int closingCuda(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y);

int erosionNaive(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y);
int dilationNaive(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y);
int openingNaive(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y);
int closingNaive(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y);

int erosionByteImageCuda(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, int block_dim_x, int block_dim_y);
int dilationByteImageCuda(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, int block_dim_x, int block_dim_y);
int openingByteImageCuda(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, int block_dim_x, int block_dim_y);
int closingByteImageCuda(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, int block_dim_x, int block_dim_y);

int erosionUint64ImageCuda(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, int block_dim_x, int block_dim_y);
int dilationUint64ImageCuda(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, int block_dim_x, int block_dim_y);
int openingUint64ImageCuda(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, int block_dim_x, int block_dim_y);
int closingUint64ImageCuda(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, int block_dim_x, int block_dim_y);

#ifdef __cplusplus
}
#endif

#endif
