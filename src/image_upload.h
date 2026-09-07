#ifndef IMAGE_UPLOAD_H
#define IMAGE_UPLOAD_H

#include "raylib.h"

#define MAX_UPLOADED_IMAGES 16
#define MAX_IMAGE_PATH 512

typedef struct {
    Texture2D images[MAX_UPLOADED_IMAGES];
    int count;
    char paths[MAX_UPLOADED_IMAGES][MAX_IMAGE_PATH];
    bool dropRequested;
    bool pickRequested;
} ImageUpload;

void ImageUploadInit(ImageUpload *upload);
void ImageUploadShutdown(ImageUpload *upload);
void ImageUploadUpdate(ImageUpload *upload);
void ImageUploadAddPath(ImageUpload *upload, const char *path);
int ImageUploadLoadImages(ImageUpload *upload);
void ImageUploadClear(ImageUpload *upload);
Texture2D ImageUploadGetRandom(const ImageUpload *upload);

#endif
