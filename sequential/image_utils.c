#include "image_utils.h"
#include <errno.h>

Image* createImage(int width, int height, char *mn) {
    Image *img = (Image*)malloc(sizeof(Image));
    if (!img) {
        fprintf(stderr, "Errore: impossibile allocare memoria per l'immagine\n");
        return NULL;
    }

    strcpy(img->magicNumber, mn);
    img->width = width;
    img->height = height;

    img->data = NULL;

    return img;
}// createImage

void freeImage(Image *img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
    }
}// freeImage

Image* loadImage(const char* filename) {
    FILE * file = fopen(filename, "r");

    if (file == NULL) {
        fprintf(stderr, "Errore: impossibile aprire il file %s\n", filename);
        perror("");
        return NULL;
    } else {
        int width, height, res;
        char mn[3];
        int r = fscanf(file, "%s", mn);
        if (mn[0] != 'P' || (mn[1] != '1' && mn[1] != '2' && mn[1] != '3' && mn[1] != '4' && mn[1] != '5' && mn[1] != '6')) {
            fprintf(stderr, "Errore: tipo di file non compatibile\n");
            fclose(file);
            return NULL;
        } 
        if (fscanf(file, "%d %d", &width, &height) != 2) {
            fprintf(stderr, "Errore: impossibile leggere le dimensioni\n");
            fclose(file);
            return NULL;
        } else {
            Image * image = createImage(width, height, mn);
            int val, res;
            unsigned char * data = (unsigned char*)malloc(width * height);
            if(data == NULL) {
                fprintf(stderr, "Errore: impossibile allocare memoria per i pixel dell'immagine\n");
                free(image);
                fclose(file);
                return NULL;
            }
            for (int i = 0; i < width*height; i++) {
                res = fscanf(file, "%d", &val);
                if (res != 1) {
                    fprintf(stderr, "Errore: file terminato prima dei pixel attesi\n");
                    fclose(file);
                    freeImage(image);
                    return NULL;
                } else {
                    if (val != 0 && val != 1) {
                        fprintf(stderr, "Errore: valore anomalo trovato\n");
                        fclose(file);
                        freeImage(image);
                        return NULL;
                    } else {
                        data[i] = (unsigned char) val;
                    }
                }
            }
            image->data = data;
            fclose(file);
            return image;
        }
    }
}//loadImage

int saveImage(const char* filename, Image* image) {
    char *mn = image->magicNumber;
    int w = image->width;
    int h = image->height;
    unsigned char * data = image->data;
    int header_len = snprintf(NULL, 0, "%s\n%d %d\n", mn, w, h);
    char * header = (char *)malloc(header_len + 1);
    sprintf(header, "%s\n%d %d\n", mn, w, h);

    FILE *fptr;
    fptr = fopen(filename, "w");
    if (fptr == NULL) {
        fprintf(stderr, "Errore: impossibile aprire il file %s per la scrittura\n", filename);
        return 1;
    }
    fprintf(fptr, "%s", header);
    char *raw = (char *)malloc(w * 2 + 1);

    for (int i = 0; i < h; i++) {
        int pos = 0;
        for (int j = 0; j < w; j++) {
            if(data[(i*w)+j] == 1) {
                raw[pos++] = '1';
            } else {
                raw[pos++] = '0';
            }
            raw[pos++] = ' ';
        }
        raw[pos++] = '\n';
        fwrite(raw, 1, pos, fptr);
    }
    free(header);
    free(raw);
    return 0;
}

ByteImage* createByteImage(int width, int height, char *mn) {
    ByteImage* img = (ByteImage*)malloc(sizeof(ByteImage));
    if (!img) {
        fprintf(stderr, "Errore: impossibile allocare memoria per l'immagine\n");
        return NULL;
    }

    strcpy(img->magicNumber, mn);
    img->width = width;
    img->height = height;
    img->rowStride = (width + 7) / 8;

    img->data = NULL;

    return img;
} // createByteImage

void freeByteImage(ByteImage* byteImage) {
    if (byteImage) {
        if (byteImage->data) {
            free(byteImage->data);
        }
        free(byteImage);
    }
} // freeByteImage

ByteImage* loadByteImage(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Errore: impossibile aprire il file %s\n", filename);
        perror("");
        return NULL;
    } else {
        int width, height, res;
        char mn[3];
        int r = fscanf(file, "%s", mn);
        if (mn[0] != 'P' || (mn[1] != '1' && mn[1] != '2' && mn[1] != '3' && mn[1] != '4' && mn[1] != '5' && mn[1] != '6')) {
            fprintf(stderr, "Errore: tipo di file non compatibile\n");
            fclose(file);
            return NULL;
        }
        if (fscanf(file, "%d %d", &width, &height) != 2) {
            fprintf(stderr, "Errore: impossibile leggere le dimensioni\n");
            fclose(file);
            return NULL;
        } else {
            ByteImage* image = createByteImage(width, height, mn);
            int val, res;
            int rowStride = image->rowStride;
            unsigned char* data = (unsigned char*)malloc(rowStride * height);
            if (data == NULL) {
                fprintf(stderr, "Errore: impossibile allocare memoria per i pixel dell'immagine\n");
                free(image);
                fclose(file);
                return NULL;
            }
            memset(data, 0, rowStride * height); // Inizializza a 0
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    res = fscanf(file, "%d", &val);
                    if (res != 1) {
                        fprintf(stderr, "Errore: file terminato prima dei pixel attesi\n");
                        fclose(file);
                        freeByteImage(image);
                        return NULL;
                    } else {
                        if (val != 0 && val != 1) {
                            fprintf(stderr, "Errore: valore anomalo trovato\n");
                            fclose(file);
                            freeByteImage(image);
                            return NULL;
                        } else {
                            if (val == 1) {
                                data[i * rowStride + j / 8] |= (1 << (7 - (j % 8)));
                            }
                        }
                    }
                }
            }
            image->data = data;
            fclose(file);
            return image;
        }
    }
} // loadByteImage

int saveByteImage(const char* filename, ByteImage* byteImage) {
    char* mn = byteImage->magicNumber;
    int w = byteImage->width;
    int h = byteImage->height;
    int rowStride = byteImage->rowStride;
    unsigned char* data = byteImage->data;
    int header_len = snprintf(NULL, 0, "%s\n%d %d\n", mn, w, h);
    char* header = (char*)malloc(header_len + 1);
    sprintf(header, "%s\n%d %d\n", mn, w, h);

    FILE* fptr;
    fptr = fopen(filename, "w");
    if (fptr == NULL) {
        fprintf(stderr, "Errore: impossibile aprire il file %s per la scrittura\n", filename);
        return 1;
    }
    fprintf(fptr, "%s", header);
    char* raw = (char*)malloc(w * 2 + 1);

    for (int i = 0; i < h; i++) {
        int pos = 0;
        for (int j = 0; j < w; j++) {
            unsigned char byte = data[i * rowStride + j / 8];
            int bit = (byte >> (7 - (j % 8))) & 1;
            if (bit == 1) {
                raw[pos++] = '1';
            } else {
                raw[pos++] = '0';
            }
            raw[pos++] = ' ';
        }
        raw[pos++] = '\n';
        fwrite(raw, 1, pos, fptr);
    }
    free(header);
    free(raw);
    fclose(fptr);
    return 0;
} // saveByteImage

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