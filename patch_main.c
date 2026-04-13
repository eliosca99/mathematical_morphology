    Image* out = allocateImageBufferLike(image);
    Image* tmp = allocateImageBufferLike(image);
    if (tmp && tmp->data) {
        free(tmp->data);
        tmp->data = (unsigned char*)calloc(2 * (size_t)image->width * (size_t)image->height, sizeof(unsigned char));
    }
