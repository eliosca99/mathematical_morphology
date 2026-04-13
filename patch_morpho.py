import sys

with open("sequential/morpho.c", "r") as f:
    lines = f.read()

new_opening = r"""int openingSeparable(Image* image, int hSize, int vSize, Image* output, Image* temp) {
    if (temp == NULL || temp->data == NULL) {
        fprintf(stderr, "Errore: buffer temporaneo non valido\n");
        return -1;
    }
    Image tempScratch = *temp;
    tempScratch.data = temp->data + (size_t)temp->width * temp->height;

    if (erosionSeparable(image, hSize, vSize, temp, &tempScratch) != 0) return -1;
    if (dilationSeparable(temp, hSize, vSize, output, &tempScratch) != 0) return -1;
    return 0;
} // openingSeparable"""

new_closing = r"""int closingSeparable(Image* image, int hSize, int vSize, Image* output, Image* temp) {
    if (temp == NULL || temp->data == NULL) {
        fprintf(stderr, "Errore: buffer temporaneo non valido\n");
        return -1;
    }
    Image tempScratch = *temp;
    tempScratch.data = temp->data + (size_t)temp->width * temp->height;

    if (dilationSeparable(image, hSize, vSize, temp, &tempScratch) != 0) return -1;
    if (erosionSeparable(temp, hSize, vSize, output, &tempScratch) != 0) return -1;
    return 0;
} // closingSeparable"""

import re
lines = re.sub(r"int openingSeparable.*?\} // openingSeparable", new_opening, lines, flags=re.DOTALL)
lines = re.sub(r"int closingSeparable.*?\} // closingSeparable", new_closing, lines, flags=re.DOTALL)

with open("sequential/morpho.c", "w") as f:
    f.write(lines)
