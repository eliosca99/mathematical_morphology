#include "image_utils.h"

Image* createImage(int width, int height) {
    Image *img = (Image*)malloc(sizeof(Image));
    if (!img) {
        printf("Impossible to allocate memory for the image");
        return NULL;
    }

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
        return NULL;
    } else {
        printf("File opened\n");
        int width, height, res;
        char tmp[3];
        fscanf(file, "%s", tmp);
        if (fscanf(file, "%d %d", &width, &height) != 2) {
            printf("Impossible to read dimensions\n");
            return NULL;
        } else {
            Image * image = createImage(width, height);
            int val, res;
            for (int i = 0; i < width*height; i++) {
                res = fscanf(file, "%d", &val);
                if (res != 1) {
                    printf("File terminated before expected pixels\n");
                    freeImage(image);
                    return NULL;
                } else {
                    if (val != 0 && val != 1) {
                        printf("Found anomalous value\n");
                        freeImage(image);
                        return NULL;
                    } else {
                        image->data[i] = (unsigned char) val;
                    }
                }
            }
            return image;

        }
    }



}//loadImage