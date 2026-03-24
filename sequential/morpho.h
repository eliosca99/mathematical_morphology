#ifndef MORPHO_H
#define MORPHO_H

#include "image_utils.h"

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

ByteImage* erosionByteImage(ByteImage* image, StructuringElementWithOffsets* SE);
ByteImage* dilationByteImage(ByteImage* image, StructuringElementWithOffsets* SE);
ByteImage* openingByteImage(ByteImage* image, StructuringElementWithOffsets* SE);
ByteImage* closingByteImage(ByteImage* image, StructuringElementWithOffsets* SE);


#endif