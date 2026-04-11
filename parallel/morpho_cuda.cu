#include "morpho_cuda.h"
#include "kernels.cuh"

Image* erosion(Image* image, StructuringElementWithOffsets* SE) {
    // funzione wrapper lanciata dall'host che alloca e inizializza la memoria sia su host
    // che su device e lancia il kernel
    
    int w = image->width;
    int h = image->height;

    Image* imageEroded = (Image*)malloc(sizeof(Image));
    if (!imageEroded) {
        fprintf(stderr, "Impossibile allocare memoria per l'immagine erosa");
        return NULL;
    }

    unsigned char* dataEroded = (unsigned char*)malloc(w * h * sizeof(unsigned char));
    if (!dataEroded) {
        fprintf(stderr, "Impossibile allocare memoria per i dati dell'immagine erosa");
        free(imageEroded);
        return NULL;
    }

    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;

    char* linearOffsets = (char*)malloc(SE->numOffsets * sizeof(char));
    if(!linearOffsets) {
        fprintf(stderr, "Errore: impossibile allocare memoria per gli offset linearizzati\n");
        free(dataEroded);
        free(imageEroded);
        return NULL;
    }
    for(int i = 0; i < SE->numOffsets; i++) {
        linearOffsets[i] = SE->offsets[i].dy * w + SE->offsets[i].dx;
    }

    // il SE è comune per tutti i blocchi, quindi lo alloco nella constant memory.
    // ogni blocco invece avrà la sua porzione di immagine da erodere, e quindi le immagini di input e output
    // le alloco nella global memory. della definizione del kernel poi, sfrutterò la shared memory

    copy_se_to_constant(linearOffsets, SE->numOffsets);

    
}
