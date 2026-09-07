#include "image_upload.h"
#include "raymath.h"
#include <string.h>
#include <stdlib.h>

void ImageUploadInit(ImageUpload *upload) {
    upload->count = 0;
    memset(upload->paths, 0, sizeof(upload->paths));
    upload->dropRequested = false;
    upload->pickRequested = false;
}

void ImageUploadShutdown(ImageUpload *upload) {
    for (int i = 0; i < upload->count; i++) {
        UnloadTexture(upload->images[i]);
    }
    upload->count = 0;
}

void ImageUploadUpdate(ImageUpload *upload) {
    if (upload->dropRequested) {
        upload->dropRequested = false;
    }
    if (upload->pickRequested) {
        upload->pickRequested = false;
        if (IsFileDropped()) {
            FilePathList files = LoadDroppedFiles();
            for (int i = 0; i < files.count && upload->count < MAX_UPLOADED_IMAGES; i++) {
                if (IsFileExtension(files.paths[i], ".png") || IsFileExtension(files.paths[i], ".jpg") || IsFileExtension(files.paths[i], ".bmp")) {
                    strncpy(upload->paths[upload->count], files.paths[i], MAX_IMAGE_PATH - 1);
                    upload->count++;
                }
            }
            UnloadDroppedFiles(files);
        }
    }
}

void ImageUploadAddPath(ImageUpload *upload, const char *path) {
    if (upload->count >= MAX_UPLOADED_IMAGES) return;
    strncpy(upload->paths[upload->count], path, MAX_IMAGE_PATH - 1);
    upload->count++;
}

int ImageUploadLoadImages(ImageUpload *upload) {
    int loaded = 0;
    for (int i = 0; i < upload->count; i++) {
        if (upload->images[i].id == 0) {
            upload->images[i] = LoadTexture(upload->paths[i]);
            if (upload->images[i].id != 0) loaded++;
        }
    }
    return loaded;
}

void ImageUploadClear(ImageUpload *upload) {
    for (int i = 0; i < upload->count; i++) {
        if (upload->images[i].id != 0) UnloadTexture(upload->images[i]);
    }
    upload->count = 0;
    memset(upload->paths, 0, sizeof(upload->paths));
}

Texture2D ImageUploadGetRandom(const ImageUpload *upload) {
    if (upload->count == 0) return (Texture2D){ 0 };
    int idx = rand() % upload->count;
    return upload->images[idx];
}
