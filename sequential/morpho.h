#ifndef MORPHO_H
#define MORPHO_H

#include "image_utils.h"

int erosion(Image* image, StructuringElement* SE, Image* output, Image* temp);
int dilation(Image* image, StructuringElement* SE, Image* output, Image* temp);
int opening(Image* image, StructuringElement* SE, Image* output, Image* temp);
int closing(Image* image, StructuringElement* SE, Image* output, Image* temp);

int erosionWithOffsets(Image* image, StructuringElementWithOffsets* SE, Image* output, Image* temp);
int dilationWithOffsets(Image* image, StructuringElementWithOffsets* SE, Image* output, Image* temp);
int openingWithOffsets(Image* image, StructuringElementWithOffsets* SE, Image* output, Image* temp);
int closingWithOffsets(Image* image, StructuringElementWithOffsets* SE, Image* output, Image* temp);

int erosionSeparable(Image* image, int hSize, int vSize, Image* output, Image* temp);
int dilationSeparable(Image* image, int hSize, int vSize, Image* output, Image* temp);
int openingSeparable(Image* image, int hSize, int vSize, Image* output, Image* temp);
int closingSeparable(Image* image, int hSize, int vSize, Image* output, Image* temp);

int erosionByteImage(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, ByteImage* temp);
int dilationByteImage(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, ByteImage* temp);
int openingByteImage(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, ByteImage* temp);
int closingByteImage(ByteImage* image, StructuringElementWithOffsets* SE, ByteImage* output, ByteImage* temp);

int erosionUint64Image(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, Uint64Image* temp);
int dilationUint64Image(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, Uint64Image* temp);
int openingUint64Image(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, Uint64Image* temp);
int closingUint64Image(Uint64Image* image, StructuringElementWithOffsets* SE, Uint64Image* output, Uint64Image* temp);

#endif