#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int width;
    int height;
    unsigned char *data;
} Image;

Image* createImage(int width, int height);
void freeImage(Image* image);

Image* loadImage(const char* filename);
void saveImage(const char* filename, Image* image);

#endif