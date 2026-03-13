#include "morpho.h"

StructuringElement* createSE(int width, int height, int originX, int originY, unsigned char* data) {
    StructuringElement* se = (StructuringElement*)malloc(sizeof(StructuringElement));
    if (!se) {
        fprintf(stderr, "Errore: impossibile allocare memoria per l'elemento strutturante\n");
        return NULL;
    }

    se->width = width;
    se->height = height;
    se->originX = originX;
    se->originY = originY;

    se->data = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    if (!se->data) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati dell'elemento strutturante\n");
        free(se);
        return NULL;
    }

    memcpy(se->data, data, width * height * sizeof(unsigned char));

    return se;
}// createSE

StructuringElement* createBoxSE(int size) {
    unsigned char* data = (unsigned char*)malloc(size * size * sizeof(unsigned char));
    if (!data) {
        fprintf(stderr, "Errore: impossibile allocare memoria per SE box\n");
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
        fprintf(stderr, "Errore: impossibile allocare memoria per SE cross\n");
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
        fprintf(stderr, "Errore: impossibile allocare memoria per SE disk\n");
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
        fprintf(stderr, "Errore: impossibile allocare memoria per SE linea orizzontale\n");
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
        fprintf(stderr, "Errore: impossibile allocare memoria per SE linea verticale\n");
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        data[i] = 1;
    }

    StructuringElement* se = createSE(1, length, 0, length / 2, data);
    free(data);
    return se;
} // createVerticalLineSE

StructuringElementWithOffsets* createSEWithOffsets(int width, int height, int originX, int originY, unsigned char* data) {
    StructuringElementWithOffsets* se = (StructuringElementWithOffsets*)malloc(sizeof(StructuringElementWithOffsets));
    if (!se) {
        fprintf(stderr, "Errore: impossibile allocare memoria per l'elemento strutturante con offset\n");
        return NULL;
    }

    se->width = width;
    se->height = height;
    se->originX = originX;
    se->originY = originY;

    se->data = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    if (!se->data) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati dell'elemento strutturante con offset\n");
        free(se);
        return NULL;
    }

    memcpy(se->data, data, width * height * sizeof(unsigned char));

    // Conta il numero di pixel attivi e crea l'array di offset
    se->numOffsets = 0;
    for (int i = 0; i < width * height; i++) {
        if (data[i] == 1) {
            se->numOffsets++;
        }
    }

    se->offsets = (SEOffset*)malloc(se->numOffsets * sizeof(SEOffset));
    if (!se->offsets) {
        fprintf(stderr, "Errore: impossibile allocare memoria per gli offset dell'elemento strutturante\n");
        free(se->data);
        free(se);
        return NULL;
    }

    int idx = 0;
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            if (data[r * width + c] == 1) {
                se->offsets[idx].dx = c - originX;
                se->offsets[idx].dy = r - originY;
                idx++;
            }
        }
    }

    return se;
} // createSEWithOffsets

StructuringElementWithOffsets* createBoxSEWithOffsets(int size) {
    unsigned char* data = (unsigned char*)malloc(size * size * sizeof(unsigned char));
    if (!data) {
        fprintf(stderr, "Errore: impossibile allocare memoria per SE box con offset\n");
        return NULL;
    }

    for (int i = 0; i < size * size; i++) {
        data[i] = 1;
    }

    StructuringElementWithOffsets* se = createSEWithOffsets(size, size, size / 2, size / 2, data);
    free(data);
    return se;
} // createBoxSEWithOffsets

StructuringElementWithOffsets* createCrossSEWithOffsets(int size) {
    unsigned char* data = (unsigned char*)malloc(size * size * sizeof(unsigned char));
    if (!data) {
        fprintf(stderr, "Errore: impossibile allocare memoria per SE cross con offset\n");
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

    StructuringElementWithOffsets* se = createSEWithOffsets(size, size, size / 2, size / 2, data);
    free(data);
    return se;
} // createCrossSEWithOffsets

StructuringElementWithOffsets* createDiskSEWithOffsets(int radius) {
    int diameter = 2 * radius + 1;
    unsigned char* data = (unsigned char*)malloc(diameter * diameter * sizeof(unsigned char));
    if (!data) {
        fprintf(stderr, "Errore: impossibile allocare memoria per SE disk con offset\n");
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

    StructuringElementWithOffsets* se = createSEWithOffsets(diameter, diameter, radius, radius, data);
    free(data);
    return se;
} // createDiskSEWithOffsets

StructuringElementWithOffsets* createOrizontalLineSEWithOffsets(int length) {
    unsigned char* data = (unsigned char*)malloc(length * sizeof(unsigned char));
    if (!data) {
        fprintf(stderr, "Errore: impossibile allocare memoria per SE linea orizzontale con offset\n");
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        data[i] = 1;
    }

    StructuringElementWithOffsets* se = createSEWithOffsets(length, 1, length / 2, 0, data);
    free(data);
    return se;
} // createOrizontalLineSEWithOffsets

StructuringElementWithOffsets* createVerticalLineSEWithOffsets(int length) {
    unsigned char* data = (unsigned char*)malloc(length * sizeof(unsigned char));
    if (!data) {
        fprintf(stderr, "Errore: impossibile allocare memoria per SE linea verticale con offset\n");
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        data[i] = 1;
    }

    StructuringElementWithOffsets* se = createSEWithOffsets(1, length, 0, length / 2, data);
    free(data);
    return se;
} // createVerticalLineSEWithOffsets

void freeSE(StructuringElement* se) {
    if (se) {
        if (se->data) {
            free(se->data);
        }
        free(se);
    }
} // freeSE

void freeSEWithOffsets(StructuringElementWithOffsets* se) {
    if (se) {
        if (se->data) {
            free(se->data);
        }
        if (se->offsets) {
            free(se->offsets);
        }
        free(se);
    }
} // freeSEWithOffsets

Image* erosion(Image* image, StructuringElement* SE) {
    int h = image->height;
    int w = image->width;
    
    Image* imageEroded = createImage(w, h, image->magicNumber);
    if(imageEroded == NULL) {
        fprintf(stderr, "Errore: impossibile allocare la nuova immagine\n");
        return NULL;
    }
    unsigned char* dataEroded = (unsigned char*)calloc(w * h, sizeof(unsigned char));
    if(!dataEroded) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati della nuova immagine\n");
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

Image* dilation(Image* image, StructuringElement* SE) {
    int h = image->height;
    int w = image->width;

    Image* imageDilated = createImage(w, h, image->magicNumber);
    if(imageDilated == NULL) {
        fprintf(stderr, "Errore: impossibile allocare la nuova immagine\n");
        return NULL;
    }
    unsigned char* dataDilated = (unsigned char*)calloc(w * h, sizeof(unsigned char));
    if(!dataDilated) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati della nuova immagine\n");
        free(imageDilated);
        return NULL;
    }

    // per ogni pixel dell'immagine che è 1, "stampo" l'SE centrato su quel pixel
    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j++) {
            if(image->data[(w * i) + j] == 1) {
                for(int r = 0; r < SE->height; r++) {
                    for(int c = 0; c < SE->width; c++) {
                        if(SE->data[(SE->width * r) + c] == 1) {
                            int X = j + (c - SE->originX);
                            int Y = i + (r - SE->originY);
                            if(X >= 0 && X < w && Y >= 0 && Y < h) {
                                dataDilated[(w * Y) + X] = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    imageDilated->data = dataDilated;

    return imageDilated;
}

Image* opening(Image* image, StructuringElement* SE) {
    Image* eroded = erosion(image, SE);
    if(eroded == NULL) return NULL;
    Image* result = dilation(eroded, SE);
    freeImage(eroded);
    return result;
}

Image* closing(Image* image, StructuringElement* SE) {
    Image* dilated = dilation(image, SE);
    if(dilated == NULL) return NULL;
    Image* result = erosion(dilated, SE);
    freeImage(dilated);
    return result;
}

Image* erosionWithOffsets (Image* image, StructuringElementWithOffsets* SE) {
    // in questa versione, utilizzo gli offset del kernel. In questo modo non devo iterare su ogni pixel del kernel, ma solo su quelli pari a 1. 
    // Inoltre, linearizzo gli indici in modo che ho 3 for invece di 4, e nel for interno in cui valuto la corrispondenza tra i pixel dell'immagine e del kernel
    // devo eseguire una semplice somma e non moltiplicazione e somma.

    int h = image->height;
    int w = image->width;
    
    Image* imageEroded = createImage(w, h, image->magicNumber);
    if(imageEroded == NULL) {
        fprintf(stderr, "Errore: impossibile allocare la nuova immagine\n");
        return NULL;
    }
    unsigned char* dataEroded = (unsigned char*)calloc(w * h, sizeof(unsigned char));
    if(!dataEroded) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati della nuova immagine\n");
        free(imageEroded);
        return NULL;
    }
        
    // qui mi definisco i bordi dell'immagine, perchè l'SE deve essere totalmente contenuto nell'immagine, non può sforare -> avrò sicuro un bordo di zeri
    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    // calcolo gli offset lineari ora in modo da calcolarli una volta sola
    int* linearOffsets = (int*)malloc(SE->numOffsets * sizeof(int));
    if(!linearOffsets) {
        fprintf(stderr, "Errore: impossibile allocare memoria per gli offset linearizzati\n");
        free(dataEroded);
        free(imageEroded);
        return NULL;
    }
    for(int i = 0; i < SE->numOffsets; i++) {
        linearOffsets[i] = SE->offsets[i].dy * w + SE->offsets[i].dx;
    }


    // itero sui pixel dell'immagine
    for(int i = top; i < h - bottom; i++)
        for(int j = left; j < w - right; j++) {
            int base = i * w + j;
            int match = 1;
            // itero sugli offsets del SE. Ogni offset è già linearizzato, quindi basta una somma
            for(int o = 0; o < SE->numOffsets; o++) {
                if(image->data[base + linearOffsets[o]] == 0) {
                    match = 0;
                    break;
                }
            }
            if(match == 1) {
                dataEroded[base] = 1;
            }
        }

    imageEroded->data = dataEroded;

    return imageEroded;
} //erosionWithOffsets

Image* dilationWithOffsets(Image* image, StructuringElementWithOffsets* SE) {
    int h = image->height;
    int w = image->width;

    Image* imageDilated = createImage(w, h, image->magicNumber);
    if(imageDilated == NULL) {
        fprintf(stderr, "Errore: impossibile allocare la nuova immagine\n");
        return NULL;
    }
    unsigned char* dataDilated = (unsigned char*)calloc(w * h, sizeof(unsigned char));
    if(!dataDilated) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati della nuova immagine\n");
        freeImage(imageDilated);
        return NULL;
    }

    int* linearOffsets = (int*)malloc(SE->numOffsets * sizeof(int));
    if(!linearOffsets) {
        fprintf(stderr, "Errore: impossibile allocare memoria per gli offset linearizzati\n");
        free(dataDilated);
        freeImage(imageDilated);
        return NULL;
    }
    for(int i = 0; i < SE->numOffsets; i++) {
        linearOffsets[i] = SE->offsets[i].dy * w + SE->offsets[i].dx;
    }

    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    // per ogni pixel dell'immagine che è 1, "stampo" gli offset del SE
    for(int i = top; i < h - bottom; i++) {
        for(int j = left; j < w - right; j++) {
            int base = i * w + j;
            if(image->data[base] == 1) {
                for(int o = 0; o < SE->numOffsets; o++) {
                    dataDilated[base + linearOffsets[o]] = 1;
                }
            }
        }
    }

    // gestisco i bordi separatamente con bounds checking
    // riga top
    for(int i = 0; i < top; i++) {
        for(int j = 0; j < w; j++) {
            if(image->data[i * w + j] == 1) {
                for(int o = 0; o < SE->numOffsets; o++) {
                    int Y = i + SE->offsets[o].dy;
                    int X = j + SE->offsets[o].dx;
                    if(X >= 0 && X < w && Y >= 0 && Y < h)
                        dataDilated[Y * w + X] = 1;
                }
            }
        }
    }
    // riga bottom
    for(int i = h - bottom; i < h; i++) {
        for(int j = 0; j < w; j++) {
            if(image->data[i * w + j] == 1) {
                for(int o = 0; o < SE->numOffsets; o++) {
                    int Y = i + SE->offsets[o].dy;
                    int X = j + SE->offsets[o].dx;
                    if(X >= 0 && X < w && Y >= 0 && Y < h)
                        dataDilated[Y * w + X] = 1;
                }
            }
        }
    }
    // colonne left e right (solo righe interne, i bordi top/bottom già gestiti)
    for(int i = top; i < h - bottom; i++) {
        for(int j = 0; j < left; j++) {
            if(image->data[i * w + j] == 1) {
                for(int o = 0; o < SE->numOffsets; o++) {
                    int Y = i + SE->offsets[o].dy;
                    int X = j + SE->offsets[o].dx;
                    if(X >= 0 && X < w && Y >= 0 && Y < h)
                        dataDilated[Y * w + X] = 1;
                }
            }
        }
        for(int j = w - right; j < w; j++) {
            if(image->data[i * w + j] == 1) {
                for(int o = 0; o < SE->numOffsets; o++) {
                    int Y = i + SE->offsets[o].dy;
                    int X = j + SE->offsets[o].dx;
                    if(X >= 0 && X < w && Y >= 0 && Y < h)
                        dataDilated[Y * w + X] = 1;
                }
            }
        }
    }

    free(linearOffsets);
    imageDilated->data = dataDilated;

    return imageDilated;
} // dilationWithOffsets

Image* openingWithOffsets(Image* image, StructuringElementWithOffsets* SE) {
    Image* eroded = erosionWithOffsets(image, SE);
    if(eroded == NULL) return NULL;
    Image* result = dilationWithOffsets(eroded, SE);
    freeImage(eroded);
    return result;
} // openingWithOffsets

Image* closingWithOffsets(Image* image, StructuringElementWithOffsets* SE) {
    Image* dilated = dilationWithOffsets(image, SE);
    if(dilated == NULL) return NULL;
    Image* result = erosionWithOffsets(dilated, SE);
    freeImage(dilated);
    return result;
} // closingWithOffsets

Image* erosionSeparable(Image* image, int hSize, int vSize) {
    // Decomposizione separabile: un'erosione con box hSize x vSize equivale a
    // un'erosione orizzontale (linea 1 x hSize) seguita da un'erosione verticale (linea vSize x 1).
    // Complessità: O(n * m * (hSize + vSize)) invece di O(n * m * hSize * vSize).

    int h = image->height;
    int w = image->width;

    // --- Passo 1: erosione orizzontale ---
    unsigned char* temp = (unsigned char*)calloc(w * h, sizeof(unsigned char));
    if (!temp) {
        fprintf(stderr, "Errore: impossibile allocare memoria per il buffer temporaneo\n");
        return NULL;
    }

    int hLeft = hSize / 2;
    int hRight = hSize - hLeft - 1;

    for (int i = 0; i < h; i++) {
        for (int j = hLeft; j < w - hRight; j++) {
            int base = i * w + j;
            int match = 1;
            for (int k = -hLeft; k <= hRight; k++) {
                if (image->data[base + k] == 0) {
                    match = 0;
                    break;
                }
            }
            if (match)
                temp[base] = 1;
        }
    }

    // --- Passo 2: erosione verticale sul risultato intermedio ---
    Image* imageEroded = createImage(w, h, image->magicNumber);
    if (!imageEroded) {
        fprintf(stderr, "Errore: impossibile allocare la nuova immagine\n");
        free(temp);
        return NULL;
    }
    unsigned char* dataEroded = (unsigned char*)calloc(w * h, sizeof(unsigned char));
    if (!dataEroded) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati della nuova immagine\n");
        freeImage(imageEroded);
        free(temp);
        return NULL;
    }

    int vTop = vSize / 2;
    int vBottom = vSize - vTop - 1;

    for (int i = vTop; i < h - vBottom; i++) {
        for (int j = hLeft; j < w - hRight; j++) {
            int base = i * w + j;
            int match = 1;
            for (int k = -vTop; k <= vBottom; k++) {
                if (temp[base + k * w] == 0) {
                    match = 0;
                    break;
                }
            }
            if (match)
                dataEroded[base] = 1;
        }
    }

    free(temp);
    imageEroded->data = dataEroded;

    return imageEroded;
} // erosionSeparable

Image* dilationSeparable(Image* image, int hSize, int vSize) {
    // Decomposizione separabile: una dilatazione con box hSize x vSize equivale a
    // una dilatazione orizzontale (linea 1 x hSize) seguita da una dilatazione verticale (linea vSize x 1).

    int h = image->height;
    int w = image->width;

    // --- Passo 1: dilatazione orizzontale ---
    unsigned char* temp = (unsigned char*)calloc(w * h, sizeof(unsigned char));
    if (!temp) {
        fprintf(stderr, "Errore: impossibile allocare memoria per il buffer temporaneo\n");
        return NULL;
    }

    int hLeft = hSize / 2;
    int hRight = hSize - hLeft - 1;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (image->data[i * w + j] == 1) {
                // "stampo" la linea orizzontale centrata su questo pixel
                int jStart = j - hLeft;
                int jEnd = j + hRight;
                if (jStart < 0) jStart = 0;
                if (jEnd >= w) jEnd = w - 1;
                for (int k = jStart; k <= jEnd; k++) {
                    temp[i * w + k] = 1;
                }
            }
        }
    }

    // --- Passo 2: dilatazione verticale sul risultato intermedio ---
    Image* imageDilated = createImage(w, h, image->magicNumber);
    if (!imageDilated) {
        fprintf(stderr, "Errore: impossibile allocare la nuova immagine\n");
        free(temp);
        return NULL;
    }
    unsigned char* dataDilated = (unsigned char*)calloc(w * h, sizeof(unsigned char));
    if (!dataDilated) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati della nuova immagine\n");
        freeImage(imageDilated);
        free(temp);
        return NULL;
    }

    int vTop = vSize / 2;
    int vBottom = vSize - vTop - 1;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (temp[i * w + j] == 1) {
                int iStart = i - vTop;
                int iEnd = i + vBottom;
                if (iStart < 0) iStart = 0;
                if (iEnd >= h) iEnd = h - 1;
                for (int k = iStart; k <= iEnd; k++) {
                    dataDilated[k * w + j] = 1;
                }
            }
        }
    }

    free(temp);
    imageDilated->data = dataDilated;

    return imageDilated;
} // dilationSeparable

Image* openingSeparable(Image* image, int hSize, int vSize) {
    Image* eroded = erosionSeparable(image, hSize, vSize);
    if (eroded == NULL) return NULL;
    Image* result = dilationSeparable(eroded, hSize, vSize);
    freeImage(eroded);
    return result;
} // openingSeparable

Image* closingSeparable(Image* image, int hSize, int vSize) {
    Image* dilated = dilationSeparable(image, hSize, vSize);
    if (dilated == NULL) return NULL;
    Image* result = erosionSeparable(dilated, hSize, vSize);
    freeImage(dilated);
    return result;
} // closingSeparable