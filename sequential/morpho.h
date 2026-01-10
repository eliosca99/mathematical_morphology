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

StructuringElement* createSE(int width, int height, int originX, int originY, unsigned char* data);
StructuringElement* createBoxSE(int size);
StructuringElement* createCrossSE(int size);
StructuringElement* createDiskSE(int radius);
StructuringElement* createOrizontalLineSE(int length);
StructuringElement* createVerticalLineSE(int length);
void freeSE(StructuringElement* se);

Image* erosion(Image* image, StructuringElement* SE);


#endif