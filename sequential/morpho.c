#include "morpho.h"

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

ByteImage* erosionByteImage(ByteImage* image, StructuringElementWithOffsets* SE) {
    // Implementazione dell'erosione per immagini in formato byte (1 bit per pixel, 8 bit per byte).
    // La logica è simile all'erosione standard, ma bisogna gestire i bit all'interno dei byte.

    int h = image->height;
    int w = image->width;
    int rowStride = image->rowStride;

    ByteImage* imageEroded = createByteImage(w, h, image->magicNumber);
    if (imageEroded == NULL) {
        fprintf(stderr, "Errore: impossibile allocare la nuova immagine\n");
        return NULL;
    }
    unsigned char* dataEroded = (unsigned char*)calloc(rowStride * h, sizeof(unsigned char));
    if (!dataEroded) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati della nuova immagine\n");
        free(imageEroded);
        return NULL;
    }

    int top = SE->originY;
    int bottom = SE->height - SE->originY - 1;
    int left = SE->originX;
    int right = SE->width - SE->originX - 1;
    int rs = image->rowStride;

    // calcolo gli offset linearizzati. in questo caso un offset è formato da dy, il numero di righe in verticale, 
    // e dx, con quest'ultimo si aggiunge ByteOffset e bitOffset, che indica in quale byte andare e per quanti bit spostarsi
    // all'interno di tale byte. Nota: dy ci permette di prendere il byte nella riga giusta, dx serve invece per capire quale byte
    // oltre a quello corrente prendere per eseguire lo shift. infatti, se dx è positivo, allora devo fare uno shift a sinistra di dx bit, 
    // e quindi devo prendere il byte a sinistra del byte corrente per ottenere dx bit da tale byte. Altrimenti, i bit entranti nel byte a 
    // causa dello shift sarebbero tutti 0 e quindi non sarebbe corretto.
    int n = SE->numOffsets;
    int* dyRows = (int*)malloc(n * sizeof(int));
    int* dxBytes = (int*)malloc(n * sizeof(int));
    int* dxBits = (int*)malloc(n * sizeof(int));
    if (!dyRows || !dxBytes || !dxBits) {
        fprintf(stderr, "Errore: impossibile allocare memoria per gli offset byte\n");
        free(dyRows);
        free(dxBytes);
        free(dxBits);
        free(dataEroded);
        freeByteImage(imageEroded);
        return NULL;
    }

    for(int k = 0; k < n; k++) {
        int dy = SE->offsets[k].dy;
        int dx = SE->offsets[k].dx;

        dyRows[k] = rs * dy; // offset in byte per spostarsi di dy righe

        if (dx >= 0) { // shift a sinistra, prendo i bit da dx bit a sinistra del byte corrente
            dxBytes[k] = dx / 8;
            dxBits[k] = dx % 8;
        } else { // shift a destra, prendo i bit da dx bit a destra del byte corrente, quindi non devo prendere byte aggiuntivi
            dxBytes[k] = (dx - 7) / 8;
            dxBits[k] = dx - dxBytes[k] * 8;
        }
    }
    

    for(int i = top; i < h - bottom; i++) {
        int rowBase = i * rs; // indice della riga corrente
        int jByteStart = (left + 7) / 8;
        int jByteEnd = (w - right - 1) / 8;

        for(int j = jByteStart; j <= jByteEnd; j++) { // j itera sui byte della riga
            unsigned char acc = 0xFF; // inizializzo tutti i bit del byte a 1. acc sarà il byte di output

            for(int k = 0; k < n; k++) { 
                // itero per ogni bit attivo del SE. Per ogni offset, mi sposto sulla riga seguendo dyRows
                int srcRow = rowBase + dyRows[k];   // a partire da rowBase, ovvero dalla riga che sto processando,
                                                    // mi sposto di dy righe per arrivare alla riga da cui prendere i bit per questo offset

                int srcByte = j + dxBytes[k];       // a partire dal byte j, mi sposto di dxBytes per arrivare al byte da cui prendere i bit per questo offset
                                                    // da notare che nella riga srcRow, il byte di partenza è sempre j

                int bits = dxBits[k];               // bits indica di quanti bit devo fare lo shift del byte srcByte. questo ovviamente perchè l'immagine è in byte, lo SE 
                                                    // è in pixel e gli offset sono in pixel, quindi ogni bit del byte, che corrisponde a un pixel, deve essere shiftato.
                                                    

                unsigned char val;  // val conterrà un byte formato dai bit che servono per questo offset, presi dal byte srcByte (originale nell'immagine) e, 
                                    // se necessario, dal byte adiacente a srcByte (a seconda di dxBits, ovvero di quanti bit devo fare lo shift)
                                    // ottenuto il byte val, eseguo l'AND con acc, che è il byte di output, in modo da mantenere solo i bit
                                    // che corrispondono a pixel che sono 1 nell'immagine. Eseguendo questa operazione per ogni offset, alla fine acc conterrà solo i bit 
                                    // che corrispondono a pixel che sono 1 in tutti gli offset, ovvero in tutto il SE, e quindi sarà il byte eroso da scrivere nell'immagine di output.
                
                if (bits == 0) {    // nessuno shift necessario 
                    if (srcByte >= 0 && srcByte < rs) {
                        val = image->data[srcRow + srcByte];
                    } else {
                        val = 0;
                    }
                } else {            // shift necessario
                    // quando eseguo uno shift, che sia a destra o a sinistra, ho bisogno oltre che dal byte corrente anche quello adiacente,
                    // a sinistra se faccio shift a destra, a destra se faccio shift a sinistra, in modo da ottenere i bit che entrano nel byte a causa dello shift.
                    // Definisco quindi byteA e byteB: uno dei due sarà il byte srcByte, l'altro sarà il byte adiacente a srcByte, a seconda di dxBits.
                    // Esempio: dxBits = 3, shift a sinistra di 3 bit
                    //     0          1          2
                    // [10101011] [11010101] [10101010]
                    // mi trovo su scrByte = 1, dxBits = 3, shift a sinistra di 3 bit -> i bit che entrano nel byte 1 a causa dello shift sono 
                    // i primi 3 bit del byte 2 a destra di srcByte (110). quindi per ottenere il byte val, faccio uno shift a sinistra di 3 bit del byte srcByte, byteA (10101011 << 3) 
                    // e uno shift a destra di 5 bit del byte adiacente a srcByte, byteB (11010101 >> 5), in modo da ottenere i bit che entrano nel byte a causa dello shift (00000110)
                    // e poi faccio l'OR tra i due risultati per ottenere il byte val (10101100).
                    // Se invece dxBits fosse -3, shift a destra di 3 bit, i bit che entrano nel byte 1 a causa dello shift sarebbero i 3 bit più a destra del byte 0 (011).
                    // ByteA sarebbe quindi il byte 0 e byteB sarebbe il byte 1, e per ottenere val farei uno shift a destra di 3 bit del byteB e uno shift a sinistra di 5 bit del byteA, e poi l'OR tra i due risultati.
                    
                    unsigned char byteA = 0;
                    unsigned char byteB = 0;
                    if (srcByte >= 0 && srcByte < rs) {
                        byteA = image->data[srcRow + srcByte];
                    }
                    if (srcByte + 1 >= 0 && srcByte + 1 < rs) {
                        byteB = image->data[srcRow + srcByte + 1];
                    }
                    val = ((byteA << bits) | byteB >> (8 - bits));
                }
                // Fatto ciò, faccio l'AND tra acc e val. Se val ha un bit a 0, allora anche acc avrà quel bit a 0, e quindi quel pixel sarà 0 nell'immagine erosa, altrimenti rimarrà 1.
                acc &= val;
                if(acc == 0) break; // vuole dire che non ci sono più bit a 1 in acc, quindi non serve continuare ad iterare sugli offset, tanto acc rimarrà 0
            }
            dataEroded[rowBase + j] = acc;
        }
    }

    free(dyRows);
    free(dxBytes);
    free(dxBits);

    imageEroded->data = dataEroded;
    return imageEroded;
}

ByteImage* dilationByteImage(ByteImage* image, StructuringElementWithOffsets* SE) {
    int h = image->height;
    int w = image->width;
    int rs = image->rowStride;

    // definisco il padding per l'immagine di output
    int topPadding = SE->originY;    
    int bottomPadding = SE->height - SE->originY - 1;
    int leftPaddingBytes = (SE->originX + 7) / 8;
    int rightPaddingBytes = ((SE->width - SE->originX - 1) / 8) + 1;

    int newH = h + topPadding + bottomPadding;
    int newRs = rs + leftPaddingBytes + rightPaddingBytes;

    // L'immagine di output deve mantenere la stessa dimensione dell'input;
    // il padding resta solo nei buffer di lavoro interni.
    ByteImage* imageDilated = createByteImage(w, h, image->magicNumber);
    if (imageDilated == NULL) {
        fprintf(stderr, "Errore: impossibile allocare la nuova immagine\n");
        return NULL;
    }

    unsigned char* dataDilated = (unsigned char*)calloc(newRs * newH, sizeof(unsigned char));
    if (!dataDilated) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati della nuova immagine\n");
        free(imageDilated);
        return NULL;
    }

    // alloco l'immagine di input con padding 
    unsigned char* dataPadded = (unsigned char*)calloc(newRs * newH, sizeof(unsigned char));
     if (!dataPadded) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati della nuova immagine\n");
        free(imageDilated);
        free(dataDilated);
        return NULL;
    }

    for(int i = topPadding; i < newH - bottomPadding; i++) {
        const unsigned char* src = image->data + (i - topPadding) * rs;
        unsigned char* dst = dataPadded + (i * newRs) + leftPaddingBytes;
        memcpy(dst, src, (size_t)rs);
    }



    // calcolo gli offset linearizzati
    int n = SE->numOffsets;
    int* dyRows = (int*)malloc(n * sizeof(int));
    int* dxBytes = (int*)malloc(n * sizeof(int));
    int* dxBits = (int*)malloc(n * sizeof(int));
    if (!dyRows || !dxBytes || !dxBits) {
        fprintf(stderr, "Errore: impossibile allocare memoria per gli offset byte\n");
        free(dyRows);
        free(dxBytes);
        free(dxBits);
        free(dataPadded);
        free(dataDilated);
        freeByteImage(imageDilated);
        return NULL;
    }

    for (int k = 0; k < n; k++) {
        int dy = SE->offsets[k].dy;
        int dx = SE->offsets[k].dx;

        dyRows[k] = dy * newRs;
        
        if (dx >= 0) {
            dxBytes[k] = dx / 8;
            dxBits[k] = dx % 8;
        } else {
            dxBytes[k] = (dx - 7) / 8;
            dxBits[k] = dx - dxBytes[k] * 8;
        }
    }

    

    for (int i = topPadding; i < newH - bottomPadding; i++) {
        for (int j = leftPaddingBytes; j < newRs - rightPaddingBytes; j++) {
            unsigned char srcByte = dataPadded[i * newRs + j];
            int dstIdx = i * newRs + j;
            if (srcByte == 0)
                continue;

            for (int k = 0; k < n; k++) {

                dataDilated[dstIdx + dyRows[k] + dxBytes[k]] |= srcByte >> dxBits[k];
                dataDilated[dstIdx + dyRows[k] + dxBytes[k] + !!dxBits[k]] |= srcByte << ((8 - dxBits[k]) & 7);
            }
        }
    }

    free(dyRows);
    free(dxBytes);
    free(dxBits);

    unsigned char* dataOutput = (unsigned char*)calloc(rs * h, sizeof(unsigned char));
    if (!dataOutput) {
        fprintf(stderr, "Errore: impossibile allocare memoria per i dati della nuova immagine\n");
        free(dataPadded);
        free(dataDilated);
        freeByteImage(imageDilated);
        return NULL;
    }

    // Ritaglio della regione centrale (senza padding) per ottenere un output h x w.
    for (int i = 0; i < h; i++) {
        const unsigned char* src = dataDilated + (i + topPadding) * newRs + leftPaddingBytes;
        unsigned char* dst = dataOutput + i * rs;
        memcpy(dst, src, (size_t)rs);
    }

    free(dataPadded);
    free(dataDilated);
    imageDilated->data = dataOutput;
    return imageDilated;
}

ByteImage* openingByteImage(ByteImage* image, StructuringElementWithOffsets* SE) {
    ByteImage* eroded = erosionByteImage(image, SE);
    if (eroded == NULL) return NULL;
    ByteImage* result = dilationByteImage(eroded, SE);
    freeByteImage(eroded);
    return result;
}

ByteImage* closingByteImage(ByteImage* image, StructuringElementWithOffsets* SE) {
    ByteImage* dilated = dilationByteImage(image, SE);
    if (dilated == NULL) return NULL;
    ByteImage* result = erosionByteImage(dilated, SE);
    freeByteImage(dilated);
    return result;
}

