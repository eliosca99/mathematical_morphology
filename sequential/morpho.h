#ifndef MORPHO_H
#define MORPHO_H

#include "image_utils.h"

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

Image* erosion(Image* image, StructuringElement* SE);
Image* dilation(Image* image, StructuringElement* SE);
Image* opening(Image* image, StructuringElement* SE);
Image* closing(Image* image, StructuringElement* SE);

Image* erosionWithOffsets(Image* image, StructuringElementWithOffsets* SE);
Image* dilationWithOffsets(Image* image, StructuringElementWithOffsets* SE);
Image* openingWithOffsets(Image* image, StructuringElementWithOffsets* SE);
Image* closingWithOffsets(Image* image, StructuringElementWithOffsets* SE);

Image* erosionSeparable(Image* image, int hSize, int vSize);
Image* dilationSeparable(Image* image, int hSize, int vSize);
Image* openingSeparable(Image* image, int hSize, int vSize);
Image* closingSeparable(Image* image, int hSize, int vSize);


#endif