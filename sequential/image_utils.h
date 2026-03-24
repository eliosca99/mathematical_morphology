#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    char magicNumber[3];
    int width;
    int height;
    unsigned char *data;
} Image;

typedef struct {
    char magicNumber[3];
    int width;
    int height;
    int rowStride; // il numero di byte per riga = (width + 7) / 8
    unsigned char *data; // ogni char (1 byte) contiene ora 8 pixel e non solo 1
} ByteImage;

typedef struct {
    char magicNumber[3];
    int width;
    int height;
    int rowStride;
    uint64_t *data; // uint64_t occupa in memoria 64 bit, quindi 64 pixel
    
} Uint64Image;

Image* createImage(int width, int height, char *mn);
void freeImage(Image* image);
Image* loadImage(const char* filename);
int saveImage(const char* filename, Image* image);

ByteImage* createByteImage(int width, int height, char *mn);
void freeByteImage(ByteImage* byteImage);
ByteImage* loadByteImage(const char* filename);
int saveByteImage(const char* filename, ByteImage* byteImage);

Uint64Image* createUint64Image(int width, int height, char *mn);
void freeUint64Image(Uint64Image* uint64Image);
Uint64Image* loadUint64Image(const char* filename);
int saveUint64Image(const char* filename, Uint64Image* uint64Image);

typedef struct {
    int width;
    int height;
    int originX;
    int originY;
    unsigned char *data;
} StructuringElement;

typedef struct {
    int dx;
    int dy;
} SEOffset;

typedef struct {
    int width, height;
    int originX, originY;
    unsigned char *data;
    SEOffset *offsets; // Array di offset per i pixel attivi
    int numOffsets;    // Numero di offset attivi
} StructuringElementWithOffsets;

StructuringElement* createSE(int width, int height, int originX, int originY, unsigned char* data);
StructuringElement* createBoxSE(int size);
StructuringElement* createCrossSE(int size);
StructuringElement* createDiskSE(int radius);
StructuringElement* createOrizontalLineSE(int length);
StructuringElement* createVerticalLineSE(int length);

StructuringElementWithOffsets* createSEWithOffsets(int width, int height, int originX, int originY, unsigned char* data);
StructuringElementWithOffsets* createBoxSEWithOffsets(int size);
StructuringElementWithOffsets* createCrossSEWithOffsets(int size);
StructuringElementWithOffsets* createDiskSEWithOffsets(int radius);
StructuringElementWithOffsets* createOrizontalLineSEWithOffsets(int length);
StructuringElementWithOffsets* createVerticalLineSEWithOffsets(int length);

void freeSE(StructuringElement* se);
void freeSEWithOffsets(StructuringElementWithOffsets* se);

#endif