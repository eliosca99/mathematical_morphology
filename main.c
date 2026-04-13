
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

typedef int (*BaseOp)(Image*, StructuringElement*, Image*, Image*);
typedef int (*OffsetOp)(Image*, StructuringElementWithOffsets*, Image*, Image*);
typedef int (*SeparableOp)(Image*, int, int, Image*, Image*);
typedef int (*ByteOffsetOp)(ByteImage*, StructuringElementWithOffsets*, ByteImage*, ByteImage*);
typedef int (*Uint64OffsetOp)(Uint64Image*, StructuringElementWithOffsets*, Uint64Image*, Uint64Image*);

typedef struct {
    const char* name;
    BaseOp base;
    OffsetOp offset;
    SeparableOp separable;
    ByteOffsetOp byteOffset;
    Uint64OffsetOp uint64Offset;
    OffsetOp parallelOffset;
    OffsetOp parallelNaive;
    ByteOffsetOp parallelByteOffset;
    Uint64OffsetOp parallelUint64Offset;
    const char* outputFile;
} OperationSpec;

static Image* allocateImageBufferLike(const Image* reference) {
    Image* out = createImage(reference->width, reference->height, (char*)reference->magicNumber);
    if (!out) {
        return NULL;
    }

    out->data = (unsigned char*)calloc((size_t)reference->width * (size_t)reference->height, sizeof(unsigned char));
    if (!out->data) {
        freeImage(out);
        return NULL;
    }

    return out;
}

static ByteImage* allocateByteImageBufferLike(const ByteImage* reference) {
    ByteImage* out = createByteImage(reference->width, reference->height, (char*)reference->magicNumber);
    if (!out) {
        return NULL;
    }

    out->data = (unsigned char*)calloc((size_t)reference->rowStride * (size_t)reference->height, sizeof(unsigned char));
    if (!out->data) {
        freeByteImage(out);
        return NULL;
    }

    return out;
}

static Uint64Image* allocateUint64ImageBufferLike(const Uint64Image* reference) {
    Uint64Image* out = createUint64Image(reference->width, reference->height, (char*)reference->magicNumber);
    if (!out) {
        return NULL;
    }

    out->data = (uint64_t*)calloc((size_t)reference->rowStride * (size_t)reference->height, sizeof(uint64_t));
    if (!out->data) {
        freeUint64Image(out);
        return NULL;
    }

    return out;
}

static double benchmarkBase(Image* image, StructuringElement* se, BaseOp op, int runs) {
    struct timespec start, end;
    double total = 0.0;

    Image* out = allocateImageBufferLike(image);
    Image* tmp = allocateImageBufferLike(image);
    if (!out || !tmp) {
        freeImage(out);
        freeImage(tmp);
        return -1.0;
    }

    for (int r = 0; r < runs; r++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        int rc = op(image, se, out, tmp);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (rc != 0) {
            freeImage(tmp);
            freeImage(out);
            return -1.0;
        }
        total += timeDiffMs(start, end);
    }

    freeImage(tmp);
    freeImage(out);

    return total / runs;
}

static double benchmarkOffset(Image* image, StructuringElementWithOffsets* se, OffsetOp op, int runs) {
    struct timespec start, end;
    double total = 0.0;

    Image* out = allocateImageBufferLike(image);
    Image* tmp = allocateImageBufferLike(image);
    if (!out || !tmp) {
        freeImage(out);
        freeImage(tmp);
        return -1.0;
    }

    for (int r = 0; r < runs; r++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        int rc = op(image, se, out, tmp);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (rc != 0) {
            freeImage(tmp);
            freeImage(out);
            return -1.0;
        }
        total += timeDiffMs(start, end);
    }

    freeImage(tmp);
    freeImage(out);

    return total / runs;
}

static double benchmarkSeparable(Image* image, int size, SeparableOp op, int runs) {
    struct timespec start, end;
    double total = 0.0;

    Image* out = allocateImageBufferLike(image);
    Image* tmp = allocateImageBufferLike(image);
    if (!out || !tmp) {
        freeImage(out);
        freeImage(tmp);
        return -1.0;
    }

    for (int r = 0; r < runs; r++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        int rc = op(image, size, size, out, tmp);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (rc != 0) {
            freeImage(tmp);
            freeImage(out);
            return -1.0;
        }
        total += timeDiffMs(start, end);
    }

    freeImage(tmp);
    freeImage(out);

    return total / runs;
}

static double benchmarkByteOffset(ByteImage* image, StructuringElementWithOffsets* se, ByteOffsetOp op, int runs) {
    struct timespec start, end;
    double total = 0.0;

    if (!image || !op) {
        return -1.0;
    }

    ByteImage* out = allocateByteImageBufferLike(image);
    ByteImage* tmp = allocateByteImageBufferLike(image);
    if (!out || !tmp) {
        freeByteImage(out);
        freeByteImage(tmp);
        return -1.0;
    }

    for (int r = 0; r < runs; r++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        int rc = op(image, se, out, tmp);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (rc != 0) {
            freeByteImage(tmp);
            freeByteImage(out);
            return -1.0;
        }
        total += timeDiffMs(start, end);
    }

    freeByteImage(tmp);
    freeByteImage(out);

    return total / runs;
}

static double benchmarkUint64Offset(Uint64Image* image, StructuringElementWithOffsets* se, Uint64OffsetOp op, int runs) {
    struct timespec start, end;
    double total = 0.0;

    if (!image || !op) {
        return -1.0;
    }

    Uint64Image* out = allocateUint64ImageBufferLike(image);
    Uint64Image* tmp = allocateUint64ImageBufferLike(image);
    if (!out || !tmp) {
        freeUint64Image(out);
        freeUint64Image(tmp);
        return -1.0;
    }

    for (int r = 0; r < runs; r++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        int rc = op(image, se, out, tmp);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (rc != 0) {
            freeUint64Image(tmp);
            freeUint64Image(out);
            return -1.0;
        }
        total += timeDiffMs(start, end);
    }

    freeUint64Image(tmp);
    freeUint64Image(out);

    return total / runs;
}

static void formatMetric(double value, char* out, size_t outSize) {
    if (value < 0.0) {
        snprintf(out, outSize, "NA");
    } else {
        snprintf(out, outSize, "%.6f", value);
    }
}

// Placeholder hooks: finche' le versioni parallele non sono implementate,
// il benchmark le esegue comunque e riporta NA nel CSV.
static int parallelNotImplementedOffset(Image* image, StructuringElementWithOffsets* se, Image* output, Image* temp) {
    (void)image;
    (void)se;
    (void)output;
    (void)temp;
    return -1;
}

static int parallelNotImplementedByte(ByteImage* image, StructuringElementWithOffsets* se, ByteImage* output, ByteImage* temp) {
    (void)image;
    (void)se;
    (void)output;
    (void)temp;
    return -1;
}

static int parallelNotImplementedUint64(Uint64Image* image, StructuringElementWithOffsets* se, Uint64Image* output, Uint64Image* temp) {
    (void)image;
    (void)se;
    (void)output;
    (void)temp;
    return -1;
}

#include "parallel/morpho_cuda.h"

static int runParallelErosion(Image* image, StructuringElementWithOffsets* se, Image* output, Image* temp) {
    (void)temp;
    return erosionCuda(image, se, output, 32, 32);
}
static int runParallelDilation(Image* image, StructuringElementWithOffsets* se, Image* output, Image* temp) {
    (void)temp;
    return dilationCuda(image, se, output, 32, 32);
}
static int runParallelOpening(Image* image, StructuringElementWithOffsets* se, Image* output, Image* temp) {
    (void)temp;
    return openingCuda(image, se, output, 32, 32);
}
static int runParallelClosing(Image* image, StructuringElementWithOffsets* se, Image* output, Image* temp) {
    (void)temp;
    return closingCuda(image, se, output, 32, 32);
}

static int runParallelErosionNaive(Image* image, StructuringElementWithOffsets* se, Image* output, Image* temp) {
    (void)temp;
    return erosionNaive(image, se, output, 32, 32);
}
static int runParallelDilationNaive(Image* image, StructuringElementWithOffsets* se, Image* output, Image* temp) {
    (void)temp;
    return dilationNaive(image, se, output, 32, 32);
}
static int runParallelOpeningNaive(Image* image, StructuringElementWithOffsets* se, Image* output, Image* temp) {
    (void)temp;
    return openingNaive(image, se, output, 32, 32);
}
static int runParallelClosingNaive(Image* image, StructuringElementWithOffsets* se, Image* output, Image* temp) {
    (void)temp;
    return closingNaive(image, se, output, 32, 32);
}

static int saveOperationImages(Image* image, StructuringElement* se, const char* outputDir, const OperationSpec* ops, int numOps) {
    Image* out = allocateImageBufferLike(image);
    Image* tmp = allocateImageBufferLike(image);
    if (!out || !tmp) {
        freeImage(out);
        freeImage(tmp);
        fprintf(stderr, "Errore: impossibile allocare i buffer per il salvataggio immagini\n");
        return -1;
    }

    for (int i = 0; i < numOps; i++) {
        char path[1024];
        int written = snprintf(path, sizeof(path), "%s/%s", outputDir, ops[i].outputFile);
        if (written < 0 || written >= (int)sizeof(path)) {
            fprintf(stderr, "Errore: path troppo lungo per il file output\n");
            freeImage(tmp);
            freeImage(out);
            return -1;
        }

        int rc = ops[i].base(image, se, out, tmp);
        if (rc != 0) {
            fprintf(stderr, "Errore: impossibile calcolare l'operazione %s per il salvataggio\n", ops[i].name);
            freeImage(tmp);
            freeImage(out);
            return -1;
        }

        if (saveImage(path, out) != 0) {
            fprintf(stderr, "Errore: impossibile salvare il file %s\n", path);
            freeImage(tmp);
            freeImage(out);
            return -1;
        }
    }

    freeImage(tmp);
    freeImage(out);

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
        {"erosion",  erosion,  erosionWithOffsets,  erosionSeparable,  erosionByteImage,  erosionUint64Image,  runParallelErosion, runParallelErosionNaive, parallelNotImplementedByte, parallelNotImplementedUint64, "erosion.pbm"},
        {"dilation", dilation, dilationWithOffsets, dilationSeparable, dilationByteImage, dilationUint64Image, runParallelDilation, runParallelDilationNaive, parallelNotImplementedByte, parallelNotImplementedUint64, "dilation.pbm"},
        {"opening",  opening,  openingWithOffsets,  openingSeparable,  openingByteImage,  openingUint64Image,  runParallelOpening, runParallelOpeningNaive, parallelNotImplementedByte, parallelNotImplementedUint64, "opening.pbm"},
        {"closing",  closing,  closingWithOffsets,  closingSeparable,  closingByteImage,  closingUint64Image,  runParallelClosing, runParallelClosingNaive, parallelNotImplementedByte, parallelNotImplementedUint64, "closing.pbm"}
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

    fprintf(csv, "se_shape,se_size,se_radius,operation,base_ms,offset_ms,separable_ms,byte_ms,uint64_ms,parallel_offset_ms,parallel_naive_ms,parallel_byte_ms,parallel_uint64_ms,speedup_offset,speedup_separable,speedup_byte,speedup_uint64,speedup_parallel_offset,speedup_parallel_naive,speedup_parallel_byte,speedup_parallel_uint64,runs,input\n");

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
                double parallelOffsetMs = benchmarkOffset(image, seOff, ops[i].parallelOffset, numRuns);
                double parallelNaiveMs = benchmarkOffset(image, seOff, ops[i].parallelNaive, numRuns);
                double parallelByteMs = benchmarkByteOffset(byteImage, seOff, ops[i].parallelByteOffset, numRuns);
                double parallelUint64Ms = benchmarkUint64Offset(uint64Image, seOff, ops[i].parallelUint64Offset, numRuns);

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
                double speedupParallelOffset = (parallelOffsetMs > 0.0) ? (baseMs / parallelOffsetMs) : -1.0;
                double speedupParallelNaive = (parallelNaiveMs > 0.0) ? (baseMs / parallelNaiveMs) : -1.0;
                double speedupParallelByte = (parallelByteMs > 0.0) ? (baseMs / parallelByteMs) : -1.0;
                double speedupParallelUint64 = (parallelUint64Ms > 0.0) ? (baseMs / parallelUint64Ms) : -1.0;

                char separableMsField[32];
                char speedupSeparableField[32];
                char byteMsField[32];
                char speedupByteField[32];
                char uint64MsField[32];
                char speedupUint64Field[32];
                char parallelOffsetMsField[32];
                char parallelNaiveMsField[32];
                char parallelByteMsField[32];
                char parallelUint64MsField[32];
                char speedupParallelOffsetField[32];
                char speedupParallelNaiveField[32];
                char speedupParallelByteField[32];
                char speedupParallelUint64Field[32];
                formatMetric(separableMs, separableMsField, sizeof(separableMsField));
                formatMetric(speedupSeparable, speedupSeparableField, sizeof(speedupSeparableField));
                formatMetric(byteMs, byteMsField, sizeof(byteMsField));
                formatMetric(speedupByte, speedupByteField, sizeof(speedupByteField));
                formatMetric(uint64Ms, uint64MsField, sizeof(uint64MsField));
                formatMetric(speedupUint64, speedupUint64Field, sizeof(speedupUint64Field));
                formatMetric(parallelOffsetMs, parallelOffsetMsField, sizeof(parallelOffsetMsField));
                formatMetric(parallelNaiveMs, parallelNaiveMsField, sizeof(parallelNaiveMsField));
                formatMetric(parallelByteMs, parallelByteMsField, sizeof(parallelByteMsField));
                formatMetric(parallelUint64Ms, parallelUint64MsField, sizeof(parallelUint64MsField));
                formatMetric(speedupParallelOffset, speedupParallelOffsetField, sizeof(speedupParallelOffsetField));
                formatMetric(speedupParallelNaive, speedupParallelNaiveField, sizeof(speedupParallelNaiveField));
                formatMetric(speedupParallelByte, speedupParallelByteField, sizeof(speedupParallelByteField));
                formatMetric(speedupParallelUint64, speedupParallelUint64Field, sizeof(speedupParallelUint64Field));

                fprintf(csv, "%s,%d,%d,%s,%.6f,%.6f,%s,%s,%s,%s,%s,%s,%s,%.6f,%s,%s,%s,%s,%s,%s,%s,%d,%s\n",
                    shapeName,
                    seSize,
                    seRadius,
                    ops[i].name,
                    baseMs,
                    offsetMs,
                    separableMsField,
                    byteMsField,
                    uint64MsField,
                    parallelOffsetMsField,
                    parallelNaiveMsField,
                    parallelByteMsField,
                    parallelUint64MsField,
                    speedupOffset,
                    speedupSeparableField,
                    speedupByteField,
                    speedupUint64Field,
                    speedupParallelOffsetField,
                    speedupParallelNaiveField,
                    speedupParallelByteField,
                    speedupParallelUint64Field,
                    numRuns,
                    inputPath);

                if (shape == SE_SHAPE_BOX) {
                    printf("%s -> base: %.3f ms | offset: %.3f ms | separabile: %s ms | byte: %s ms | uint64: %s ms | cuda_offset: %s ms | cuda_naive: %s ms\n",
                        ops[i].name,
                        baseMs,
                        offsetMs,
                        separableMsField,
                        byteMsField,
                        uint64MsField,
                        parallelOffsetMsField,
                        parallelNaiveMsField);
                } else {
                    printf("%s -> base: %.3f ms | offset: %.3f ms | separabile: NA | byte: %s ms | uint64: %s ms | cuda_offset: %s ms | cuda_naive: %s ms\n",
                        ops[i].name,
                        baseMs,
                        offsetMs,
                        byteMsField,
                        uint64MsField,
                        parallelOffsetMsField,
                        parallelNaiveMsField);
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
