#include "sequential/image_utils.h"
#include "sequential/morpho.h"

int main() {
    Image * image = loadImage("../input_images/test_bridge.pbm");
    if (image == NULL) {
        return 1;
    }
    StructuringElement* SE = createBoxSE(60);
    if (SE == NULL) {
        return 1;
    }
    Image* imageEroded = erosion(image, SE);
    saveImage("img_test", imageEroded);

    freeImage(image);
    freeImage(imageEroded);
    freeSE(SE);

    return 0;
}