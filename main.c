
#define _POSIX_C_SOURCE 200809L

#include "sequential/image_utils.h"
#include "sequential/morpho.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

static const int kSeSizes[] = {3, 5, 7, 9, 11, 15, 21, 37};
static const int kNumSeSizes = (int)(sizeof(kSeSizes) / sizeof(kSeSizes[0]));

typedef enum {
    SE_SHAPE_BOX = 0,
    SE_SHAPE_DISK = 1
} SeShape;

static double timeDiffMs(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1e6;
}

static void printUsage(const char* prog) {
    fprintf(stderr, "Uso: %s -i <input.pbm> [-o <output_dir>] [-r <num_runs>] [-s <save_se_size>]\n", prog);
    fprintf(stderr, "  -i   immagine PBM di input (obbligatorio)\n");
    fprintf(stderr, "  -o   directory di output (opzionale)\n");
    fprintf(stderr, "       se omessa, viene creata output_images/output_N\n");
    fprintf(stderr, "  -r   numero di run per media (default: 10)\n");
    fprintf(stderr, "  -s   dimensione box SE per salvare le 4 immagini finali (default: 5)\n");
}

static int ensureDirectory(const char* path) {
    if (mkdir(path, 0777) == 0) {
        return 0;
    }
    if (errno == EEXIST) {
        return 0;
    }
    return -1;
}

static int chooseDefaultOutputRoot(char* rootDir, size_t rootDirSize) {
    struct stat st;

    if (stat("../output_images", &st) == 0 && S_ISDIR(st.st_mode)) {
        int written = snprintf(rootDir, rootDirSize, "%s", "../output_images");
        return (written >= 0 && (size_t)written < rootDirSize) ? 0 : -1;
    }

    if (stat("output_images", &st) == 0 && S_ISDIR(st.st_mode)) {
        int written = snprintf(rootDir, rootDirSize, "%s", "output_images");
        return (written >= 0 && (size_t)written < rootDirSize) ? 0 : -1;
    }

    int written = snprintf(rootDir, rootDirSize, "%s", "output_images");
    return (written >= 0 && (size_t)written < rootDirSize) ? 0 : -1;
}

static int makeProgressiveOutputDirectory(const char* rootDir, char* outDir, size_t outDirSize) {
    if (ensureDirectory(rootDir) != 0) {
        return -1;
    }

    for (int idx = 0; idx < 1000000; idx++) {
        int written = snprintf(outDir, outDirSize, "%s/output_%d", rootDir, idx);
        if (written < 0 || (size_t)written >= outDirSize) {
            return -1;
        }

        if (mkdir(outDir, 0777) == 0) {
            return 0;
        }

        if (errno != EEXIST) {
            return -1;
        }
    }

    return -1;
}

typedef Image* (*BaseOp)(Image*, StructuringElement*);
typedef Image* (*OffsetOp)(Image*, StructuringElementWithOffsets*);
typedef Image* (*SeparableOp)(Image*, int, int);
typedef ByteImage* (*ByteOffsetOp)(ByteImage*, StructuringElementWithOffsets*);
typedef Uint64Image* (*Uint64OffsetOp)(Uint64Image*, StructuringElementWithOffsets*);

typedef struct {
    const char* name;
    BaseOp base;
    OffsetOp offset;
    SeparableOp separable;
    ByteOffsetOp byteOffset;
    Uint64OffsetOp uint64Offset;
    const char* outputFile;
} OperationSpec;

static double benchmarkBase(Image* image, StructuringElement* se, BaseOp op, int runs) {
    struct timespec start, end;
    double total = 0.0;

    for (int r = 0; r < runs; r++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        Image* out = op(image, se);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (!out) {
            return -1.0;
        }
        total += timeDiffMs(start, end);
        freeImage(out);
    }

    return total / runs;
}

static double benchmarkOffset(Image* image, StructuringElementWithOffsets* se, OffsetOp op, int runs) {
    struct timespec start, end;
    double total = 0.0;

    for (int r = 0; r < runs; r++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        Image* out = op(image, se);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (!out) {
            return -1.0;
        }
        total += timeDiffMs(start, end);
        freeImage(out);
    }

    return total / runs;
}

static double benchmarkSeparable(Image* image, int size, SeparableOp op, int runs) {
    struct timespec start, end;
    double total = 0.0;

    for (int r = 0; r < runs; r++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        Image* out = op(image, size, size);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (!out) {
            return -1.0;
        }
        total += timeDiffMs(start, end);
        freeImage(out);
    }

    return total / runs;
}

static double benchmarkByteOffset(ByteImage* image, StructuringElementWithOffsets* se, ByteOffsetOp op, int runs) {
    struct timespec start, end;
    double total = 0.0;

    if (!image || !op) {
        return -1.0;
    }

    for (int r = 0; r < runs; r++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        ByteImage* out = op(image, se);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (!out) {
            return -1.0;
        }
        total += timeDiffMs(start, end);
        freeByteImage(out);
    }

    return total / runs;
}

static double benchmarkUint64Offset(Uint64Image* image, StructuringElementWithOffsets* se, Uint64OffsetOp op, int runs) {
    struct timespec start, end;
    double total = 0.0;

    if (!image || !op) {
        return -1.0;
    }

    for (int r = 0; r < runs; r++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        Uint64Image* out = op(image, se);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (!out) {
            return -1.0;
        }
        total += timeDiffMs(start, end);
        freeUint64Image(out);
    }

    return total / runs;
}

static void formatMetric(double value, char* out, size_t outSize) {
    if (value < 0.0) {
        snprintf(out, outSize, "NA");
    } else {
        snprintf(out, outSize, "%.6f", value);
    }
}

static int saveOperationImages(Image* image, StructuringElement* se, const char* outputDir, const OperationSpec* ops, int numOps) {
    for (int i = 0; i < numOps; i++) {
        char path[1024];
        int written = snprintf(path, sizeof(path), "%s/%s", outputDir, ops[i].outputFile);
        if (written < 0 || written >= (int)sizeof(path)) {
            fprintf(stderr, "Errore: path troppo lungo per il file output\n");
            return -1;
        }

        Image* out = ops[i].base(image, se);
        if (!out) {
            fprintf(stderr, "Errore: impossibile calcolare l'operazione %s per il salvataggio\n", ops[i].name);
            return -1;
        }

        if (saveImage(path, out) != 0) {
            fprintf(stderr, "Errore: impossibile salvare il file %s\n", path);
            freeImage(out);
            return -1;
        }

        freeImage(out);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    const int defaultRuns = 10;
    const int defaultImageSaveSeSize = 5;

    const char* inputPath = NULL;
    const char* requestedOutputDir = NULL;
    int numRuns = defaultRuns;
    int imageSaveSeSize = defaultImageSaveSeSize;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            inputPath = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            requestedOutputDir = argv[++i];
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            numRuns = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            imageSaveSeSize = atoi(argv[++i]);
        } else {
            printUsage(argv[0]);
            return 1;
        }
    }

    if (!inputPath) {
        printUsage(argv[0]);
        return 1;
    }
    if (numRuns <= 0) {
        fprintf(stderr, "Errore: il numero di run deve essere > 0\n");
        return 1;
    }
    if (imageSaveSeSize <= 0 || (imageSaveSeSize % 2) == 0) {
        fprintf(stderr, "Errore: -s deve essere un intero dispari > 0\n");
        return 1;
    }

    char outputDir[1024];
    if (requestedOutputDir) {
        int written = snprintf(outputDir, sizeof(outputDir), "%s", requestedOutputDir);
        if (written < 0 || written >= (int)sizeof(outputDir)) {
            fprintf(stderr, "Errore: output directory troppo lunga\n");
            return 1;
        }
        if (ensureDirectory(outputDir) != 0) {
            fprintf(stderr, "Errore: impossibile creare/aprire la directory %s\n", outputDir);
            return 1;
        }
    } else {
        char outputRoot[1024];
        if (chooseDefaultOutputRoot(outputRoot, sizeof(outputRoot)) != 0) {
            fprintf(stderr, "Errore: impossibile determinare la root di output\n");
            return 1;
        }
        if (makeProgressiveOutputDirectory(outputRoot, outputDir, sizeof(outputDir)) != 0) {
            fprintf(stderr, "Errore: impossibile creare la directory di output progressiva\n");
            return 1;
        }
    }

    Image* image = loadImage(inputPath);
    if (!image) {
        fprintf(stderr, "Errore: impossibile caricare l'immagine %s\n", inputPath);
        return 1;
    }

    ByteImage* byteImage = loadByteImage(inputPath);
    if (!byteImage) {
        fprintf(stderr, "Avviso: impossibile caricare ByteImage, benchmark byte disabilitato\n");
    }

    Uint64Image* uint64Image = loadUint64Image(inputPath);
    if (!uint64Image) {
        fprintf(stderr, "Avviso: impossibile caricare Uint64Image, benchmark uint64 disabilitato\n");
    }

    OperationSpec ops[] = {
        {"erosion",  erosion,  erosionWithOffsets,  erosionSeparable,  erosionByteImage,  erosionUint64Image,  "erosion.pbm"},
        {"dilation", dilation, dilationWithOffsets, dilationSeparable, dilationByteImage, dilationUint64Image, "dilation.pbm"},
        {"opening",  opening,  openingWithOffsets,  openingSeparable,  openingByteImage,  openingUint64Image,  "opening.pbm"},
        {"closing",  closing,  closingWithOffsets,  closingSeparable,  closingByteImage,  closingUint64Image,  "closing.pbm"}
    };
    int numOps = (int)(sizeof(ops) / sizeof(ops[0]));

    char csvPath[1024];
    int csvWritten = snprintf(csvPath, sizeof(csvPath), "%s/benchmark.csv", outputDir);
    if (csvWritten < 0 || csvWritten >= (int)sizeof(csvPath)) {
        fprintf(stderr, "Errore: path CSV troppo lungo\n");
        freeUint64Image(uint64Image);
        freeByteImage(byteImage);
        freeImage(image);
        return 1;
    }

    FILE* csv = fopen(csvPath, "w");
    if (!csv) {
        fprintf(stderr, "Errore: impossibile aprire il CSV %s\n", csvPath);
        freeUint64Image(uint64Image);
        freeByteImage(byteImage);
        freeImage(image);
        return 1;
    }

    fprintf(csv, "se_shape,se_size,se_radius,operation,base_ms,offset_ms,separable_ms,byte_ms,uint64_ms,speedup_offset,speedup_separable,speedup_byte,speedup_uint64,runs,input\n");

    printf("Input: %s (%dx%d)\n", inputPath, image->width, image->height);
    printf("Run mediati: %d\n", numRuns);
    printf("SE per immagini finali: %d\n", imageSaveSeSize);
    printf("Output dir: %s\n", outputDir);

    for (int shapeIdx = 0; shapeIdx < 2; shapeIdx++) {
        SeShape shape = (shapeIdx == 0) ? SE_SHAPE_BOX : SE_SHAPE_DISK;
        const char* shapeName = (shape == SE_SHAPE_BOX) ? "box" : "disk";

        for (int s = 0; s < kNumSeSizes; s++) {
            int seSize = kSeSizes[s];
            int seRadius = (seSize - 1) / 2;

            StructuringElement* se = NULL;
            StructuringElementWithOffsets* seOff = NULL;

            if (shape == SE_SHAPE_BOX) {
                se = createBoxSE(seSize);
                seOff = createBoxSEWithOffsets(seSize);
            } else {
                se = createDiskSE(seRadius);
                seOff = createDiskSEWithOffsets(seRadius);
            }

            if (!se || !seOff) {
                fprintf(stderr, "Errore: impossibile creare l'elemento strutturante %s (size=%d)\n", shapeName, seSize);
                freeSE(se);
                freeSEWithOffsets(seOff);
                fclose(csv);
                    freeUint64Image(uint64Image);
                freeByteImage(byteImage);
                freeImage(image);
                return 1;
            }

            if (shape == SE_SHAPE_BOX) {
                printf("\nSE BOX %dx%d\n", seSize, seSize);
            } else {
                printf("\nSE DISK r=%d (diametro %dx%d)\n", seRadius, seSize, seSize);
            }

            for (int i = 0; i < numOps; i++) {
                double baseMs = benchmarkBase(image, se, ops[i].base, numRuns);
                double offsetMs = benchmarkOffset(image, seOff, ops[i].offset, numRuns);
                double separableMs = -1.0;
                if (shape == SE_SHAPE_BOX) {
                    separableMs = benchmarkSeparable(image, seSize, ops[i].separable, numRuns);
                }
                double byteMs = benchmarkByteOffset(byteImage, seOff, ops[i].byteOffset, numRuns);
                double uint64Ms = benchmarkUint64Offset(uint64Image, seOff, ops[i].uint64Offset, numRuns);

                if (baseMs < 0 || offsetMs < 0 || (shape == SE_SHAPE_BOX && separableMs < 0)) {
                    fprintf(stderr, "Errore: benchmark fallito per %s (%s, size=%d)\n", ops[i].name, shapeName, seSize);
                    freeSE(se);
                    freeSEWithOffsets(seOff);
                    fclose(csv);
                    freeUint64Image(uint64Image);
                    freeByteImage(byteImage);
                    freeImage(image);
                    return 1;
                }

                double speedupOffset = baseMs / offsetMs;
                double speedupSeparable = (separableMs > 0.0) ? (baseMs / separableMs) : -1.0;
                double speedupByte = (byteMs > 0.0) ? (baseMs / byteMs) : -1.0;
                double speedupUint64 = (uint64Ms > 0.0) ? (baseMs / uint64Ms) : -1.0;

                char separableMsField[32];
                char speedupSeparableField[32];
                char byteMsField[32];
                char speedupByteField[32];
                char uint64MsField[32];
                char speedupUint64Field[32];
                formatMetric(separableMs, separableMsField, sizeof(separableMsField));
                formatMetric(speedupSeparable, speedupSeparableField, sizeof(speedupSeparableField));
                formatMetric(byteMs, byteMsField, sizeof(byteMsField));
                formatMetric(speedupByte, speedupByteField, sizeof(speedupByteField));
                formatMetric(uint64Ms, uint64MsField, sizeof(uint64MsField));
                formatMetric(speedupUint64, speedupUint64Field, sizeof(speedupUint64Field));

                fprintf(csv, "%s,%d,%d,%s,%.6f,%.6f,%s,%s,%s,%.6f,%s,%s,%s,%d,%s\n",
                    shapeName,
                    seSize,
                    seRadius,
                    ops[i].name,
                    baseMs,
                    offsetMs,
                    separableMsField,
                    byteMsField,
                    uint64MsField,
                    speedupOffset,
                    speedupSeparableField,
                    speedupByteField,
                    speedupUint64Field,
                    numRuns,
                    inputPath);

                if (shape == SE_SHAPE_BOX) {
                    printf("%s -> base: %.3f ms | offset: %.3f ms | separabile: %s ms | byte: %s ms | uint64: %s ms\n",
                        ops[i].name,
                        baseMs,
                        offsetMs,
                        separableMsField,
                        byteMsField,
                        uint64MsField);
                } else {
                    printf("%s -> base: %.3f ms | offset: %.3f ms | separabile: NA | byte: %s ms | uint64: %s ms\n",
                        ops[i].name,
                        baseMs,
                        offsetMs,
                        byteMsField,
                        uint64MsField);
                }
            }

            freeSE(se);
            freeSEWithOffsets(seOff);
        }
    }

    fclose(csv);

    StructuringElement* imageSe = createBoxSE(imageSaveSeSize);
    if (!imageSe) {
        fprintf(stderr, "Errore durante la creazione dell'SE per il salvataggio immagini\n");
        freeUint64Image(uint64Image);
        freeByteImage(byteImage);
        freeImage(image);
        return 1;
    }

    if (saveOperationImages(image, imageSe, outputDir, ops, numOps) != 0) {
        fprintf(stderr, "Errore durante il salvataggio delle immagini di output\n");
        freeSE(imageSe);
        freeUint64Image(uint64Image);
        freeByteImage(byteImage);
        freeImage(image);
        return 1;
    }

    freeSE(imageSe);

    printf("CSV salvato in: %s\n", csvPath);
    printf("Immagini salvate in: %s/{erosion,dilation,opening,closing}.pbm\n", outputDir);

    freeUint64Image(uint64Image);
    freeByteImage(byteImage);
    freeImage(image);
    return 0;
}
