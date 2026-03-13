
#define _POSIX_C_SOURCE 200809L
#include "sequential/image_utils.h"
#include "sequential/morpho.h"
#include <string.h>
#include <time.h>

#define NUM_RUNS 10

static double time_diff_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1e6;
}

// Macro per il benchmark di una singola operazione (base + offset + separabile)
#define BENCH_OP_3(label, baseCall, offCall, sepCall) \
    do { \
        double avgBase = 0, avgOff = 0, avgSep = 0; \
        for (int r = 0; r < numRuns; r++) { \
            clock_gettime(CLOCK_MONOTONIC, &start); \
            Image* res = baseCall; \
            clock_gettime(CLOCK_MONOTONIC, &end); \
            avgBase += time_diff_ms(start, end); \
            freeImage(res); \
        } \
        avgBase /= numRuns; \
        for (int r = 0; r < numRuns; r++) { \
            clock_gettime(CLOCK_MONOTONIC, &start); \
            Image* res = offCall; \
            clock_gettime(CLOCK_MONOTONIC, &end); \
            avgOff += time_diff_ms(start, end); \
            freeImage(res); \
        } \
        avgOff /= numRuns; \
        for (int r = 0; r < numRuns; r++) { \
            clock_gettime(CLOCK_MONOTONIC, &start); \
            Image* res = sepCall; \
            clock_gettime(CLOCK_MONOTONIC, &end); \
            avgSep += time_diff_ms(start, end); \
            freeImage(res); \
        } \
        avgSep /= numRuns; \
        printf("%-12s | %10.3f | %11.3f | %10.3f | %7.2fx | %7.2fx\n", \
               label, avgBase, avgOff, avgSep, \
               avgBase / avgOff, avgBase / avgSep); \
    } while(0)

// Macro per il benchmark di una singola operazione (base + offset, senza separabile)
#define BENCH_OP_2(label, baseCall, offCall) \
    do { \
        double avgBase = 0, avgOff = 0; \
        for (int r = 0; r < numRuns; r++) { \
            clock_gettime(CLOCK_MONOTONIC, &start); \
            Image* res = baseCall; \
            clock_gettime(CLOCK_MONOTONIC, &end); \
            avgBase += time_diff_ms(start, end); \
            freeImage(res); \
        } \
        avgBase /= numRuns; \
        for (int r = 0; r < numRuns; r++) { \
            clock_gettime(CLOCK_MONOTONIC, &start); \
            Image* res = offCall; \
            clock_gettime(CLOCK_MONOTONIC, &end); \
            avgOff += time_diff_ms(start, end); \
            freeImage(res); \
        } \
        avgOff /= numRuns; \
        printf("%-12s | %10.3f | %11.3f | %7.2fx\n", \
               label, avgBase, avgOff, avgBase / avgOff); \
    } while(0)

static void printUsage(const char* prog) {
    fprintf(stderr, "Uso: %s -i <input> [-r <num_runs>] [-o <output_dir>]\n", prog);
    fprintf(stderr, "  -i   percorso immagine di input\n");
    fprintf(stderr, "  -r   numero di run per la media (default: %d)\n", NUM_RUNS);
    fprintf(stderr, "  -o   cartella di output: salva 1 immagine per operazione\n");
}

int main(int argc, char* argv[]) {
    const char* inputPath = NULL;
    const char* outputDir = NULL;
    int numRuns = NUM_RUNS;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            inputPath = argv[++i];
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            numRuns = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outputDir = argv[++i];
        } else {
            printUsage(argv[0]);
            return 1;
        }
    }

    if (!inputPath) {
        printUsage(argv[0]);
        return 1;
    }

    Image* image = loadImage(inputPath);
    if (image == NULL) return 1;

    printf("Immagine: %s (%dx%d)\n", inputPath, image->width, image->height);
    printf("Numero di run per media: %d\n", numRuns);

    // =============================================
    //  SALVATAGGIO OUTPUT (se -o specificato)
    // =============================================
    if (outputDir) {
        StructuringElement* SE5 = createBoxSE(5);
        if (!SE5) { fprintf(stderr, "Errore creazione SE\n"); freeImage(image); return 1; }

        char path[1024];
        Image* res;

        res = erosion(image, SE5);
        if (res) {
            snprintf(path, sizeof(path), "%s/erosione_box5.pbm", outputDir);
            saveImage(path, res);
            printf("Salvata: %s\n", path);
            freeImage(res);
        }

        res = dilation(image, SE5);
        if (res) {
            snprintf(path, sizeof(path), "%s/dilatazione_box5.pbm", outputDir);
            saveImage(path, res);
            printf("Salvata: %s\n", path);
            freeImage(res);
        }

        res = opening(image, SE5);
        if (res) {
            snprintf(path, sizeof(path), "%s/opening_box5.pbm", outputDir);
            saveImage(path, res);
            printf("Salvata: %s\n", path);
            freeImage(res);
        }

        res = closing(image, SE5);
        if (res) {
            snprintf(path, sizeof(path), "%s/closing_box5.pbm", outputDir);
            saveImage(path, res);
            printf("Salvata: %s\n", path);
            freeImage(res);
        }

        freeSE(SE5);
    }

    struct timespec start, end;

    // =============================================
    //  TEST CON BOX SE (base + offset + separabile)
    // =============================================
    int boxSizes[] = {3, 5, 11, 21, 41, 61};
    int numBoxSizes = sizeof(boxSizes) / sizeof(boxSizes[0]);

    printf("\n##############################################\n");
    printf("#             BENCHMARK BOX SE               #\n");
    printf("##############################################\n");

    for (int s = 0; s < numBoxSizes; s++) {
        int size = boxSizes[s];

        StructuringElement* SE = createBoxSE(size);
        StructuringElementWithOffsets* SEO = createBoxSEWithOffsets(size);
        if (!SE || !SEO) {
            fprintf(stderr, "Errore: impossibile creare SE di dimensione %d\n", size);
            if (SE) freeSE(SE);
            if (SEO) freeSEWithOffsets(SEO);
            continue;
        }

        printf("\n========================================\n");
        printf("  SE Box %dx%d\n", size, size);
        printf("========================================\n");
        printf("%-12s | %-10s | %-11s | %-10s | %-8s | %-8s\n",
               "Operazione", "Base (ms)", "Offset (ms)", "Separ (ms)", "Sp. Off", "Sp. Sep");
        printf("-------------|------------|-------------|------------|----------|--------\n");

        BENCH_OP_3("Erosione",
                   erosion(image, SE),
                   erosionWithOffsets(image, SEO),
                   erosionSeparable(image, size, size));

        BENCH_OP_3("Dilatazione",
                   dilation(image, SE),
                   dilationWithOffsets(image, SEO),
                   dilationSeparable(image, size, size));

        BENCH_OP_3("Opening",
                   opening(image, SE),
                   openingWithOffsets(image, SEO),
                   openingSeparable(image, size, size));

        BENCH_OP_3("Closing",
                   closing(image, SE),
                   closingWithOffsets(image, SEO),
                   closingSeparable(image, size, size));

        freeSE(SE);
        freeSEWithOffsets(SEO);
    }

    // =============================================
    //  TEST CON DISK SE (base + offset, no separabile)
    // =============================================
    int diskRadii[] = {1, 2, 5, 10, 20, 30};
    int numDiskRadii = sizeof(diskRadii) / sizeof(diskRadii[0]);

    printf("\n##############################################\n");
    printf("#            BENCHMARK DISK SE               #\n");
    printf("##############################################\n");

    for (int s = 0; s < numDiskRadii; s++) {
        int radius = diskRadii[s];
        int diameter = 2 * radius + 1;

        StructuringElement* SE = createDiskSE(radius);
        StructuringElementWithOffsets* SEO = createDiskSEWithOffsets(radius);
        if (!SE || !SEO) {
            fprintf(stderr, "Errore: impossibile creare Disk SE di raggio %d\n", radius);
            if (SE) freeSE(SE);
            if (SEO) freeSEWithOffsets(SEO);
            continue;
        }

        printf("\n========================================\n");
        printf("  SE Disk raggio=%d (diametro %dx%d, %d pixel attivi)\n",
               radius, diameter, diameter, SEO->numOffsets);
        printf("========================================\n");
        printf("%-12s | %-10s | %-11s | %-8s\n",
               "Operazione", "Base (ms)", "Offset (ms)", "Sp. Off");
        printf("-------------|------------|-------------|--------\n");

        BENCH_OP_2("Erosione",
                   erosion(image, SE),
                   erosionWithOffsets(image, SEO));

        BENCH_OP_2("Dilatazione",
                   dilation(image, SE),
                   dilationWithOffsets(image, SEO));

        BENCH_OP_2("Opening",
                   opening(image, SE),
                   openingWithOffsets(image, SEO));

        BENCH_OP_2("Closing",
                   closing(image, SE),
                   closingWithOffsets(image, SEO));

        freeSE(SE);
        freeSEWithOffsets(SEO);
    }

    freeImage(image);
    return 0;
}
