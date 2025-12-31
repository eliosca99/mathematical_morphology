#include "sequential/image_utils.h"

int main() {
    Image * image = loadImage("../input_images/img_test.pbm");
    if (image == NULL) {
        return 1;
    }
    saveImage("img_test", image);
    freeImage(image);
    return 0;
}