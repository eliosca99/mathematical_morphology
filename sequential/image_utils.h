#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char magicNumber[3];
    int width;
    int height;
    unsigned char *data;
} Image;

Image* createImage(int width, int height, char *mn);
void freeImage(Image* image);

Image* loadImage(const char* filename);
int saveImage(const char* filename, Image* image);

#endif