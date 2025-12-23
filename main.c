#include "sequential/image_utils.h"

int main() {
    Image * image = loadImage("img_test.pbm");
    if (image == NULL) {
        return 1;
    }
    freeImage(image);
    return 0;
}