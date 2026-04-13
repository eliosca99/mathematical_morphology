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

int erosionByteImageCuda(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi\n");
        return -1;
    }

    int width     = image->width;
    int height    = image->height;
    int rowStride = image->rowStride;
    int numOffsets = SE->numOffsets;

    int top    = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left   = SE->originX;
    int right  = SE->width - SE->originX - 1;

    copy_se_to_constant(SE);

    unsigned char* d_in  = nullptr;
    unsigned char* d_out = nullptr;
    size_t size = (size_t)rowStride * height * sizeof(unsigned char);

    CUDA_CHECK(cudaMalloc((void**)&d_in,  size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize(
        (rowStride + block_dim_x - 1) / block_dim_x,
        (height    + block_dim_y - 1) / block_dim_y
    );

    int leftHaloBytes  = (left + 7) / 8;
    int rightHaloBytes = right / 8;
    int tile_byte_w    = leftHaloBytes + block_dim_x + rightHaloBytes + 1;
    int tile_h         = top + block_dim_y + bottom;
    int shared_bytes   = tile_byte_w * tile_h * sizeof(unsigned char);

    erosionByteImageKernel<<<gridSize, blockSize, shared_bytes>>>(
        d_in, d_out,
        width, height, rowStride,
        numOffsets,
        top, bottom, left, right
    );

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));

    return 0;
}

int dilationByteImageCuda(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi\n");
        return -1;
    }

    int width      = image->width;
    int height     = image->height;
    int rowStride  = image->rowStride;
    int numOffsets = SE->numOffsets;

    int top    = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left   = SE->originX;
    int right  = SE->width - SE->originX - 1;

    copy_se_to_constant(SE);

    unsigned char* d_in  = nullptr;
    unsigned char* d_out = nullptr;
    size_t size = (size_t)rowStride * height * sizeof(unsigned char);

    CUDA_CHECK(cudaMalloc((void**)&d_in,  size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize(
        (rowStride + block_dim_x - 1) / block_dim_x,
        (height    + block_dim_y - 1) / block_dim_y
    );

    // halo invertito rispetto all'erosione
    int leftHaloBytes  = right / 8 + 1;
    int rightHaloBytes = (left + 7) / 8;
    int tile_byte_w    = leftHaloBytes + block_dim_x + rightHaloBytes;
    int tile_h         = bottom + block_dim_y + top;
    int shared_bytes   = tile_byte_w * tile_h * sizeof(unsigned char);

    dilationByteImageKernel<<<gridSize, blockSize, shared_bytes>>>(
        d_in, d_out,
        width, height, rowStride,
        numOffsets,
        top, bottom, left, right
    );

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));

    return 0;
}

int openingByteImageCuda(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi\n");
        return -1;
    }

    // alloco un ByteImage temporaneo con la stessa struttura dell'input
    ByteImage* temp = createByteImage(image->width, image->height, image->magicNumber);
    if (!temp) return -1;
    temp->data = (unsigned char*)calloc((size_t)image->rowStride * image->height, sizeof(unsigned char));
    if (!temp->data) {
        freeByteImage(temp);
        return -1;
    }

    // erosione → dilatazione
    if (erosionByteImageCuda(image, SE, temp, block_dim_x, block_dim_y) != 0) {
        freeByteImage(temp);
        return -1;
    }
    if (dilationByteImageCuda(temp, SE, output, block_dim_x, block_dim_y) != 0) {
        freeByteImage(temp);
        return -1;
    }

    freeByteImage(temp);
    return 0;
}

int closingByteImageCuda(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi\n");
        return -1;
    }

    ByteImage* temp = createByteImage(image->width, image->height, image->magicNumber);
    if (!temp) return -1;
    temp->data = (unsigned char*)calloc((size_t)image->rowStride * image->height, sizeof(unsigned char));
    if (!temp->data) {
        freeByteImage(temp);
        return -1;
    }

    // dilatazione → erosione
    if (dilationByteImageCuda(image, SE, temp, block_dim_x, block_dim_y) != 0) {
        freeByteImage(temp);
        return -1;
    }
    if (erosionByteImageCuda(temp, SE, output, block_dim_x, block_dim_y) != 0) {
        freeByteImage(temp);
        return -1;
    }

    freeByteImage(temp);
    return 0;
}

int erosionUint64ImageCuda(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi\n");
        return -1;
    }

    int width      = image->width;
    int height     = image->height;
    int rowStride  = image->rowStride;
    int numOffsets = SE->numOffsets;

    int top    = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left   = SE->originX;
    int right  = SE->width - SE->originX - 1;

    copy_se_to_constant(SE);

    uint64_t* d_in  = nullptr;
    uint64_t* d_out = nullptr;
    size_t size = (size_t)rowStride * height * sizeof(uint64_t);

    CUDA_CHECK(cudaMalloc((void**)&d_in,  size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize(
        (rowStride + block_dim_x - 1) / block_dim_x,
        (height    + block_dim_y - 1) / block_dim_y
    );

    int leftHaloWords  = (left + 63) / 64;
    int rightHaloWords = right / 64;
    int tile_word_w    = leftHaloWords + block_dim_x + rightHaloWords + 1;
    int tile_h         = top + block_dim_y + bottom;
    int shared_bytes   = tile_word_w * tile_h * sizeof(uint64_t);

    erosionUint64ImageKernel<<<gridSize, blockSize, shared_bytes>>>(
        d_in, d_out,
        width, height, rowStride,
        numOffsets,
        top, bottom, left, right
    );

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));

    return 0;
}

int dilationUint64ImageCuda(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi\n");
        return -1;
    }

    int width      = image->width;
    int height     = image->height;
    int rowStride  = image->rowStride;
    int numOffsets = SE->numOffsets;

    int top    = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left   = SE->originX;
    int right  = SE->width - SE->originX - 1;

    copy_se_to_constant(SE);

    uint64_t* d_in  = nullptr;
    uint64_t* d_out = nullptr;
    size_t size = (size_t)rowStride * height * sizeof(uint64_t);

    CUDA_CHECK(cudaMalloc((void**)&d_in,  size));
    CUDA_CHECK(cudaMalloc((void**)&d_out, size));

    CUDA_CHECK(cudaMemcpy(d_in, image->data, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_out, 0, size));

    dim3 blockSize(block_dim_x, block_dim_y);
    dim3 gridSize(
        (rowStride + block_dim_x - 1) / block_dim_x,
        (height    + block_dim_y - 1) / block_dim_y
    );

    int leftHaloWords  = right / 64 + 1;
    int rightHaloWords = (left + 63) / 64;
    int tile_word_w    = leftHaloWords + block_dim_x + rightHaloWords;
    int tile_h         = bottom + block_dim_y + top;
    int shared_bytes   = tile_word_w * tile_h * sizeof(uint64_t);

    dilationUint64ImageKernel<<<gridSize, blockSize, shared_bytes>>>(
        d_in, d_out,
        width, height, rowStride,
        numOffsets,
        top, bottom, left, right
    );

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaGetLastError());

    CUDA_CHECK(cudaMemcpy(output->data, d_out, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));

    return 0;
}

int openingUint64ImageCuda(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi\n");
        return -1;
    }

    Uint64Image* temp = createUint64Image(image->width, image->height, image->magicNumber);
    if (!temp) return -1;
    temp->data = (uint64_t*)calloc((size_t)image->rowStride * image->height, sizeof(uint64_t));
    if (!temp->data) { freeUint64Image(temp); return -1; }

    if (erosionUint64ImageCuda(image, SE, temp, block_dim_x, block_dim_y) != 0) {
        freeUint64Image(temp); return -1;
    }
    if (dilationUint64ImageCuda(temp, SE, output, block_dim_x, block_dim_y) != 0) {
        freeUint64Image(temp); return -1;
    }

    freeUint64Image(temp);
    return 0;
}

int closingUint64ImageCuda(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, int block_dim_x, int block_dim_y) {
    if (!image || !image->data || !SE || !output || !output->data) {
        fprintf(stderr, "Parametri non validi\n");
        return -1;
    }

    Uint64Image* temp = createUint64Image(image->width, image->height, image->magicNumber);
    if (!temp) return -1;
    temp->data = (uint64_t*)calloc((size_t)image->rowStride * image->height, sizeof(uint64_t));
    if (!temp->data) { freeUint64Image(temp); return -1; }

    if (dilationUint64ImageCuda(image, SE, temp, block_dim_x, block_dim_y) != 0) {
        freeUint64Image(temp); return -1;
    }
    if (erosionUint64ImageCuda(temp, SE, output, block_dim_x, block_dim_y) != 0) {
        freeUint64Image(temp); return -1;
    }

    freeUint64Image(temp);
    return 0;
}