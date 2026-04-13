#include "morpho_cuda.h"
#include "kernels.cuh"

int erosionNaive(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y) {
    // funzione wrapper lanciata dall'host che alloca e inizializza la memoria sia su host
    // che su device e lancia il kernel
    
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi");
        return -1;
    }

    int w = image->width;
    int h = image->height;
    int numOffsets = SE->numOffsets;

    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    // il SE è comune per tutti i blocchi, quindi lo alloco nella constant memory.
    // ogni blocco invece avrà la sua porzione di immagine da erodere, e quindi le immagini di input e output
    // le alloco nella global memory. della definizione del kernel poi, sfrutterò la shared memory

    copy_se_to_constant(SE);

    unsigned char* d_in = nullptr;
    unsigned char* d_out = nullptr;
    size_t size = w * h * sizeof(unsigned char);

    CUDA_CHECK(cudaMalloc((void**)&d_in, size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));

    // ora posso lanciare il kernel, ma prima definisco dimensione di blocco e grid

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize((w + blockSize.x - 1) / blockSize.x, (h + blockSize.y - 1) / blockSize.y);

    erosionNaiveKernel<<<gridSize, blockSize>>>(d_in, d_out, w, h, numOffsets, top, bottom, left, right);

    // verifico la presenza di errori e aspetto la fine dell'esecuzione dei thread 
    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    // copio il risultato dalla memoria della gpu alla ram
    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));

    return 0;
}

int dilationNaive(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y) {
    // funzione wrapper lanciata dall'host che alloca e inizializza la memoria sia su host
    // che su device e lancia il kernel
    
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi");
        return -1;
    }

    int w = image->width;
    int h = image->height;
    int numOffsets = SE->numOffsets;

    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    // il SE è comune per tutti i blocchi, quindi lo alloco nella constant memory.
    // ogni blocco invece avrà la sua porzione di immagine da erodere, e quindi le immagini di input e output
    // le alloco nella global memory. della definizione del kernel poi, sfrutterò la shared memory

    copy_se_to_constant(SE);

    unsigned char* d_in = nullptr;
    unsigned char* d_out = nullptr;
    size_t size = w * h * sizeof(unsigned char);

    CUDA_CHECK(cudaMalloc((void**)&d_in, size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));

    // ora posso lanciare il kernel, ma prima definisco dimensione di blocco e grid

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize((w + blockSize.x - 1) / blockSize.x, (h + blockSize.y - 1) / blockSize.y);

    dilationNaiveKernel<<<gridSize, blockSize>>>(d_in, d_out, w, h, numOffsets);

    // verifico la presenza di errori e aspetto la fine dell'esecuzione dei thread 
    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    // copio il risultato dalla memoria della gpu alla ram
    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));

    return 0;
}

int openingNaive(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi");
        return -1;
    }

    int w = image->width;
    int h = image->height;
    int numOffsets = SE->numOffsets;

    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    copy_se_to_constant(SE);

    unsigned char* d_in = nullptr;
    unsigned char* d_out = nullptr;
    unsigned char* d_temp = nullptr;
    size_t size = w * h * sizeof(unsigned char);

    CUDA_CHECK(cudaMalloc((void**)&d_in, size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));
    CUDA_CHECK(cudaMalloc((void**)&d_temp, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));
    CUDA_CHECK(cudaMemset(d_temp, 0, size));

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize((w + blockSize.x - 1) / blockSize.x, (h + blockSize.y - 1) / blockSize.y);

    erosionNaiveKernel<<<gridSize, blockSize>>>(d_in, d_temp, w, h, numOffsets, top, bottom, left, right);

    dilationNaiveKernel<<<gridSize, blockSize>>>(d_temp, d_out, w, h, numOffsets);

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    // copio il risultato dalla memoria della gpu alla ram
    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));
    CUDA_CHECK(cudaFree(d_temp));

    return 0;
}

int closingNaive(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi");
        return -1;
    }

    int w = image->width;
    int h = image->height;
    int numOffsets = SE->numOffsets;

    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    copy_se_to_constant(SE);

    unsigned char* d_in = nullptr;
    unsigned char* d_out = nullptr;
    unsigned char* d_temp = nullptr;
    size_t size = w * h * sizeof(unsigned char);

    CUDA_CHECK(cudaMalloc((void**)&d_in, size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));
    CUDA_CHECK(cudaMalloc((void**)&d_temp, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));
    CUDA_CHECK(cudaMemset(d_temp, 0, size));

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize((w + blockSize.x - 1) / blockSize.x, (h + blockSize.y - 1) / blockSize.y);

    dilationNaiveKernel<<<gridSize, blockSize>>>(d_in, d_temp, w, h, numOffsets);
    
    erosionNaiveKernel<<<gridSize, blockSize>>>(d_temp, d_out, w, h, numOffsets, top, bottom, left, right);

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    // copio il risultato dalla memoria della gpu alla ram
    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));
    CUDA_CHECK(cudaFree(d_temp));

    return 0;
}

int erosionCuda(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi");
        return -1;
    }

    int w = image->width;
    int h = image->height;
    int numOffsets = SE->numOffsets;

    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    copy_se_to_constant(SE);

    unsigned char* d_in = nullptr;
    unsigned char* d_out = nullptr;
    size_t size = w * h * sizeof(unsigned char);

    CUDA_CHECK(cudaMalloc((void**)&d_in, size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize((w + blockSize.x - 1) / blockSize.x, (h + blockSize.y - 1) / blockSize.y);

    int tile_w = left + right + block_dim_x;
    int tile_h = top + bottom + block_dim_y;
    int shared_bytes = tile_w * tile_h * sizeof(unsigned char);

    erosionKernel<<<gridSize, blockSize, shared_bytes>>>(d_in, d_out, w, h, numOffsets, top, bottom, left, right);

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));

    return 0;
}

int dilationCuda(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi");
        return -1;
    }

    int w = image->width;
    int h = image->height;
    int numOffsets = SE->numOffsets;

    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    copy_se_to_constant(SE);

    unsigned char* d_in = nullptr;
    unsigned char* d_out = nullptr;
    size_t size = w * h * sizeof(unsigned char);

    CUDA_CHECK(cudaMalloc((void**)&d_in, size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize((w + blockSize.x - 1) / blockSize.x, (h + blockSize.y - 1) / blockSize.y);

    int tile_w = left + right + block_dim_x;
    int tile_h = top + bottom + block_dim_y;
    int shared_bytes = tile_w * tile_h * sizeof(unsigned char);

    dilationKernel<<<gridSize, blockSize, shared_bytes>>>(d_in, d_out, w, h, numOffsets, top, bottom, left, right);

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));

    return 0;
}

int openingCuda(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi");
        return -1;
    }

    int w = image->width;
    int h = image->height;
    int numOffsets = SE->numOffsets;

    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    copy_se_to_constant(SE);

    unsigned char* d_in = nullptr;
    unsigned char* d_out = nullptr;
    unsigned char* d_temp = nullptr;
    size_t size = w * h * sizeof(unsigned char);

    CUDA_CHECK(cudaMalloc((void**)&d_in, size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));
    CUDA_CHECK(cudaMalloc((void**)&d_temp, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));
    CUDA_CHECK(cudaMemset(d_temp, 0, size));

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize((w + blockSize.x - 1) / blockSize.x, (h + blockSize.y - 1) / blockSize.y);

    int tile_w = left + right + block_dim_x;
    int tile_h = top + bottom + block_dim_y;
    int shared_bytes = tile_w * tile_h * sizeof(unsigned char);

    erosionKernel<<<gridSize, blockSize, shared_bytes>>>(d_in, d_temp, w, h, numOffsets, top, bottom, left, right);

    dilationKernel<<<gridSize, blockSize, shared_bytes>>>(d_temp, d_out, w, h, numOffsets, top, bottom, left, right);

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    // copio il risultato dalla memoria della gpu alla ram
    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));
    CUDA_CHECK(cudaFree(d_temp));

    return 0;
}

int closingCuda(Image* image, StructuringElementWithOffsets* SE, Image* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi");
        return -1;
    }

    int w = image->width;
    int h = image->height;
    int numOffsets = SE->numOffsets;

    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    copy_se_to_constant(SE);

    unsigned char* d_in = nullptr;
    unsigned char* d_out = nullptr;
    unsigned char* d_temp = nullptr;
    size_t size = w * h * sizeof(unsigned char);

    CUDA_CHECK(cudaMalloc((void**)&d_in, size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));
    CUDA_CHECK(cudaMalloc((void**)&d_temp, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));
    CUDA_CHECK(cudaMemset(d_temp, 0, size));

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize((w + blockSize.x - 1) / blockSize.x, (h + blockSize.y - 1) / blockSize.y);

    int tile_w = left + right + block_dim_x;
    int tile_h = top + bottom + block_dim_y;
    int shared_bytes = tile_w * tile_h * sizeof(unsigned char);

    dilationKernel<<<gridSize, blockSize, shared_bytes>>>(d_in, d_temp, w, h, numOffsets, top, bottom, left, right);

    erosionKernel<<<gridSize, blockSize, shared_bytes>>>(d_temp, d_out, w, h, numOffsets, top, bottom, left, right);

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    // copio il risultato dalla memoria della gpu alla ram
    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));
    CUDA_CHECK(cudaFree(d_temp));

    return 0;
}