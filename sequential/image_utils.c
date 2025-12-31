#include "image_utils.h"
#include <errno.h>

Image* createImage(int width, int height, char *mn) {
    Image *img = (Image*)malloc(sizeof(Image));
    if (!img) {
        printf("Impossible to allocate memory for the image");
        return NULL;
    }

    strcpy(img->magicNumber, mn);
    img->width = width;
    img->height = height;

    img->data = (unsigned char*)calloc(width * height, sizeof(unsigned char));

    if (!img->data) {
        printf("Impossible to allocate memory for the pixels");
        free(img);
        return NULL;
    }

    printf("Image allocated\n");
    return img;
}// createImage

void freeImage(Image *img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
        printf("Image deallocated\n");
    }
}// freeImage

Image* loadImage(const char* filename) {
    FILE * file = fopen(filename, "r");

    if (file == NULL) {
        printf("Can't open the file %s\n", filename);
        perror("");
        return NULL;
    } else {
        printf("File opened\n");
        int width, height, res;
        char mn[3];
        fscanf(file, "%s", mn);
        if (mn[0] != 'P' || (mn[1] != '1' && mn[1] != '2' && mn[1] != 3 && mn[1] != '4' && mn[1] != '5' && mn[1] != '6')) {
            printf("Incompatible file type");
            fclose(file);
            return NULL;
        } 
        if (fscanf(file, "%d %d", &width, &height) != 2) {
            printf("Impossible to read dimensions\n");
            fclose(file);
            return NULL;
        } else {
            Image * image = createImage(width, height, mn);
            int val, res;
            for (int i = 0; i < width*height; i++) {
                res = fscanf(file, "%d", &val);
                if (res != 1) {
                    printf("File terminated before expected pixels\n");
                    fclose(file);
                    freeImage(image);
                    return NULL;
                } else {
                    if (val != 0 && val != 1) {
                        printf("Found anomalous value\n");
                        fclose(file);
                        freeImage(image);
                        return NULL;
                    } else {
                        image->data[i] = (unsigned char) val;
                    }
                }
            }
            fclose(file);
            printf("File closed\n");
            return image;
        }
    }
}//loadImage

int saveImage(const char* filename, Image* image) {
    char *mn = image->magicNumber;
    int w = image->width;
    int h = image->height;
    unsigned char * data = image->data;
    char * ext;
    int header_len = snprintf(NULL, 0, "%s\n%d %d\n", mn, w, h);
    char * header = (char *)malloc(header_len + 1);
    strcpy(header, mn);
    sprintf(header, "%s\n%d %d\n", mn, w, h);
    
    switch (mn[1])
    {
        case '1': {
            ext = ".pbm";
        }
        case '2': {
            ext = ".pgm";
        }
        case '3': {
            ext = ".ppm";
        }
    }

    FILE *fptr;
    char full_filename[50];
    sprintf(full_filename, "%s%s%s", "../output_images/", filename, ext);
    fptr = fopen(full_filename, "w");
    if (fptr == NULL) {
        printf("Impossible to open or read the file %s\n", filename);
        return 1;
    }
    fprintf(fptr, "%s", header);
    char *raw = (char *)malloc(w * 2 + 1);

    for (int i = 0; i < h; i++) {
        int pos = 0;
        for (int j = 0; j < w; j++) {
            if(data[(i*w)+j] == 1) {
                raw[pos++] = '1';
                printf("HEllo\n");
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
    printf("Image saved\n");
    return 0;
}
