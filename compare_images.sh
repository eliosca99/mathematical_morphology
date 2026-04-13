#!/bin/bash

# Abilita exit on error
set -e

if [ -z "$1" ]; then
    echo "Uso: $0 <immagine_di_input.pbm>"
    echo "Esempio: ./compare_images.sh input_images/test.pbm"
    exit 1
fi

INPUT_IMG="$1"
OUT_DIR="output_images/test_correctness"

echo "Compilazione del progetto..."
cd build
make
cd ..

echo "Pulizia cartella di test..."
rm -rf "$OUT_DIR"

echo "Esecuzione applicazione per generare le immagini..."
./build/main -i "$INPUT_IMG" -o "$OUT_DIR" -r 1

echo ""
echo "=================================================="
echo "Risultati del confronto rispetto alla Versione Base"
echo "=================================================="

ALL_OK=true

for op in erosion dilation opening closing; do
    echo "--- OPERAZIONE: $op ---"
    BASE_FILE="$OUT_DIR/${op}_base.pbm"
    
    if [ ! -f "$BASE_FILE" ]; then
        echo "[ERRORE] Immagine base ${BASE_FILE} non generata."
        ALL_OK=false
        continue
    fi

    for method in offset separable byte uint64 cuda_offset cuda_naive cuda_byte; do
        COMPARE_FILE="$OUT_DIR/${op}_${method}.pbm"
        
        if [ ! -f "$COMPARE_FILE" ]; then
            continue
        fi
        
        if cmp -s "$BASE_FILE" "$COMPARE_FILE"; then
            echo "  [OK] $method: IDENTICA alla base"
        else
            echo "  [FAIL] $method: DIFFERENTE dalla base"
            ALL_OK=false
        fi
    done
    echo ""
done

if $ALL_OK; then
    echo ">>>> TUTTI I CONTROLLI SONO PASSATI! <<<<"
    exit 0
else
    echo ">>>> ATTENZIONE: ALCUNE IMMAGINI DIFFERISCONO! <<<<"
    exit 1
fi
