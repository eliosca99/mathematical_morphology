#include "morpho.h"

StructuringElement* createSE(int width, int height, int originX, int originY, unsigned char* data) {
    StructuringElement* se = (StructuringElement*)malloc(sizeof(StructuringElement));
    if (!se) {
        printf("Impossible to allocate memory for the structuring element\n");
        return NULL;
    }

    se->width = width;
    se->height = height;
    se->originX = originX;
    se->originY = originY;

    se->data = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    if (!se->data) {
        printf("Impossible to allocate memory for the structuring element data\n");
        free(se);
        return NULL;
    }

    memcpy(se->data, data, width * height * sizeof(unsigned char));

    return se;
}// createSE

StructuringElement* createBoxSE(int size) {
    unsigned char* data = (unsigned char*)malloc(size * size * sizeof(unsigned char));
    if (!data) {
        printf("Impossible to allocate memory for box SE data\n");
        return NULL;
    }

    for (int i = 0; i < size * size; i++) {
        data[i] = 1;
    }

    StructuringElement* se = createSE(size, size, size / 2, size / 2, data);
    free(data);
    return se;
} // createBoxSE

StructuringElement* createCrossSE(int size) {
    unsigned char* data = (unsigned char*)malloc(size * size * sizeof(unsigned char));
    if (!data) {
        printf("Impossible to allocate memory for cross SE data\n");
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i == size / 2 || j == size / 2) {
                data[i * size + j] = 1;
            } else {
                data[i * size + j] = 0;
            }
        }
    }

    StructuringElement* se = createSE(size, size, size / 2, size / 2, data);
    free(data);
    return se;
} // createCrossSE

StructuringElement* createDiskSE(int radius) {
    int diameter = 2 * radius + 1;
    unsigned char* data = (unsigned char*)malloc(diameter * diameter * sizeof(unsigned char));
    if (!data) {
        printf("Impossible to allocate memory for disk SE data\n");
        return NULL;
    }

    for (int i = 0; i < diameter; i++) {
        for (int j = 0; j < diameter; j++) {
            int dx = i - radius;
            int dy = j - radius;
            if (dx * dx + dy * dy <= radius * radius) {
                data[i * diameter + j] = 1;
            } else {
                data[i * diameter + j] = 0;
            }
        }
    }

    StructuringElement* se = createSE(diameter, diameter, radius, radius, data);
    free(data);
    return se;
} // createDiskSE

StructuringElement* createOrizontalLineSE(int length) {
    unsigned char* data = (unsigned char*)malloc(length * sizeof(unsigned char));
    if (!data) {
        printf("Impossible to allocate memory for orizontal line SE data\n");
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        data[i] = 1;
    }

    StructuringElement* se = createSE(length, 1, length / 2, 0, data);
    free(data);
    return se;
} // createOrizontalLineSE

StructuringElement* createVerticalLineSE(int length) {
    unsigned char* data = (unsigned char*)malloc(length * sizeof(unsigned char));
    if (!data) {
        printf("Impossible to allocate memory for vertical line SE data\n");
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        data[i] = 1;
    }

    StructuringElement* se = createSE(1, length, 0, length / 2, data);
    free(data);
    return se;
} // createVerticalLineSE

void freeSE(StructuringElement* se) {
    if (se) {
        if (se->data) {
            free(se->data);
        }
        free(se);
    }
} // freeSE

Image* erosion(Image* image, StructuringElement* SE) {
    int h = image->height;
    int w = image->width;
    
    Image* imageEroded = createImage(w, h, image->magicNumber);
    if(imageEroded == NULL) {
        printf("Impossibile to allocate the new image\n");
        return NULL;
    }
    unsigned char* dataEroded = (unsigned char*)calloc(w * h, sizeof(unsigned char));
    if(!dataEroded) {
        printf("Impossible to allocate memory for the data of the new image\n");
        free(imageEroded);
        return NULL;
    }
        
    //qui mi definisco i bordi dell'immagine, perchè l'SE deve essere totalmente contenuto nell'immagine, non può sforare -> avrò sicuro un bordo di zeri
    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    //con i primi due for itero sui pixel dell'immagine. considero che mi trovo sul pixel (i, j) con l'origine del SE
    for(int i = top; i < h - bottom; i++) {
        for(int j = left; j < w - right; j++) {
            int match = 1;
            //qui itero sui pixel del SE; se tutti i pixel 1 corrispondono a un pixel 1 dell'immagine allora quel pixel rimane 1 nell'immagine erosa
            for(int r = 0; r < SE->height; r++) {
                for(int c = 0; c < SE->width; c++) {
                    if(SE->data[(SE->width * r) + c] == 1) {
                        // se il pixel (r, c) del SE è 1, anche il pixel dell'immagine deve essere 1 altrimenti match = 0
                        int X = j - (SE->originX - c);
                        int Y = i - (SE->originY - r);
                        if(image->data[(w * Y) + X] == 0) {
                            match = 0;
                            break;
                        }
                    }
                }
                if(match == 0)
                    break;
            }
            if(match == 1) {
                dataEroded[(w * i) + j] = 1;
            }
        }
    }
    imageEroded->data = dataEroded;

    return imageEroded;
}
