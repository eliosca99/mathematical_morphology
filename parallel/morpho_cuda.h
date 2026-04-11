#ifndef MORPHO_CUDA_H
#define MORPHO_CUDA_H

#include "sequential/image_utils.h"

Image* erosion(Image* image, StructuringElementWithOffsets* SE);
Image* dilation(Image* image, StructuringElementWithOffsets* SE);
Image* opening(Image* image, StructuringElementWithOffsets* SE);
Image* closing(Image* image, StructuringElementWithOffsets* SE);

ByteImage* erosionByteImage(ByteImage* image, StructuringElementWithOffsets* SE);
ByteImage* dilationByteImage(ByteImage* image, StructuringElementWithOffsets* SE);
ByteImage* openingByteImage(ByteImage* image, StructuringElementWithOffsets* SE);
ByteImage* closingByteImage(ByteImage* image, StructuringElementWithOffsets* SE);

Uint64Image* erosionUint64Image(Uint64Image* image, StructuringElementWithOffsets* SE);
Uint64Image* dilationUint64Image(Uint64Image* image, StructuringElementWithOffsets* SE);
Uint64Image* openingUint64Image(Uint64Image* image, StructuringElementWithOffsets* SE);
Uint64Image* closingUint64Image(Uint64Image* image, StructuringElementWithOffsets* SE);

#endif
