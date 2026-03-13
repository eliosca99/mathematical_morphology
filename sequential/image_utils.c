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
        fscanf(file, "%s", mn);
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
