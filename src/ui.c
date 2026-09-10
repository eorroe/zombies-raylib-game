#include "ui.h"
#include "game.h"
#include "raymath.h"
#include <string.h>
#include <stdio.h>

static void UIHandleUploadGallery(MenuState *menu, InputState *input, Game *game) {
    if (!menu->showUploadPrompt) return;
    
    if (input->menuPressed || input->escapePressed) {
        menu->showUploadPrompt = false;
        menu->lastUploadError[0] = '\0';
        return;
    }
    
    if (IsFileDropped()) {
        FilePathList files = LoadDroppedFiles();
        menu->lastUploadError[0] = '\0';
        for (int i = 0; i < files.count && menu->uploadedImageCount < MAX_UPLOADED_IMAGES; i++) {
            if (IsFileExtension(files.paths[i], ".png") || IsFileExtension(files.paths[i], ".jpg") || IsFileExtension(files.paths[i], ".bmp")) {
                UIAddUploadedImage(menu, files.paths[i]);
                DebugLogf(&game->debug, DEBUG_SUCCESS, "Uploaded: %s", files.paths[i]);
            }
        }
        UnloadDroppedFiles(files);
    }
    
    if (input->mouseLeftPressed) {
        Vector2 mouse = GetMousePosition();
        int cols = menu->screenWidth / (UPLOAD_THUMB_SIZE + UPLOAD_GALLERY_PADDING);
        if (cols < 1) cols = 1;
        
        for (int i = 0; i < menu->uploadedImageCount; i++) {
            int col = i % cols;
            int row = i / cols;
            float x = UPLOAD_GALLERY_PADDING + col * (UPLOAD_THUMB_SIZE + UPLOAD_GALLERY_PADDING);
            float y = 80 + UPLOAD_GALLERY_PADDING + row * (UPLOAD_THUMB_SIZE + UPLOAD_GALLERY_PADDING + 30);
            
            Rectangle thumbRect = { x, y, (float)UPLOAD_THUMB_SIZE, (float)UPLOAD_THUMB_SIZE };
            if (CheckCollisionPointRec(mouse, thumbRect)) {
                DebugLog(&game->debug, "Image clicked (no action yet)", DEBUG_INFO);
                break;
            }
            
            Rectangle deleteBtn = { x + UPLOAD_THUMB_SIZE - 24, y, 24, 24 };
            if (CheckCollisionPointRec(mouse, deleteBtn)) {
                UnloadTexture(menu->uploadedImages[i]);
                for (int j = i; j < menu->uploadedImageCount - 1; j++) {
                    menu->uploadedImages[j] = menu->uploadedImages[j + 1];
                    memcpy(menu->imagePaths[j], menu->imagePaths[j + 1], MAX_IMAGE_PATH);
                }
                menu->uploadedImageCount--;
                DebugLog(&game->debug, "Image deleted", DEBUG_WARN);
                break;
            }
        }
    }
}

void UIInit(MenuState *menu, int screenWidth, int screenHeight) {
    menu->active = true;
    menu->selectedItem = 0;
    menu->mode = GAME_MODE_ROUNDS;
    menu->zombieMode = ZOMBIE_MODE_MIXED;
    menu->uploadedImageCount = 0;
    menu->showUploadPrompt = false;
    menu->fadeAlpha = 0.0f;
    menu->screenWidth = screenWidth;
    menu->screenHeight = screenHeight;
    menu->lastUploadError[0] = '\0';
    memset(menu->uploadedImages, 0, sizeof(menu->uploadedImages));
    memset(menu->imagePaths, 0, sizeof(menu->imagePaths));
}

void UIUpdate(MenuState *menu, InputState *input, Game *game) {
    if (menu->fadeAlpha < 1.0f) menu->fadeAlpha += 0.05f;
    
    if (menu->showUploadPrompt) {
        UIHandleUploadGallery(menu, input, game);
        return;
    }
    
    if (input->menuPressed && !menu->active) {
        menu->active = true;
        menu->selectedItem = 0;
    }
    
    if (menu->active) {
        if (input->upPressed) {
            menu->selectedItem = (menu->selectedItem - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
        }
        
        if (input->downPressed) {
            menu->selectedItem = (menu->selectedItem + 1) % MENU_ITEM_COUNT;
        }
        
        if (input->enterPressed) {
            switch (menu->selectedItem) {
                case MENU_ITEM_START:
                    menu->active = false;
                    GameInit(game, menu->screenWidth, menu->screenHeight);
                    break;
                case MENU_ITEM_MODE:
                    menu->mode = (menu->mode == GAME_MODE_ROUNDS) ? GAME_MODE_ENDLESS : GAME_MODE_ROUNDS;
                    break;
                case MENU_ITEM_ZOMBIE_MODE:
                    menu->zombieMode = (menu->zombieMode == ZOMBIE_MODE_MIXED) ? ZOMBIE_MODE_ALL_IMAGES : ZOMBIE_MODE_MIXED;
                    break;
                case MENU_ITEM_UPLOAD:
                    menu->showUploadPrompt = true;
                    menu->lastUploadError[0] = '\0';
                    break;
                case MENU_ITEM_QUIT:
                    game->state = GAME_STATE_GAMEOVER;
                    break;
            }
        }
        
        if (input->mouseLeftPressed) {
            Vector2 mouse = GetMousePosition();
            int itemH = 50;
            int startY = menu->screenHeight / 2 - 100;
            for (int i = 0; i < MENU_ITEM_COUNT; i++) {
                Rectangle btn = { menu->screenWidth / 2 - 150, startY + i * itemH, 300, 40 };
                if (CheckCollisionPointRec(mouse, btn)) {
                    menu->selectedItem = i;
                    switch (i) {
                        case MENU_ITEM_START:
                            menu->active = false;
                            GameInit(game, menu->screenWidth, menu->screenHeight);
                            break;
                        case MENU_ITEM_MODE:
                            menu->mode = (menu->mode == GAME_MODE_ROUNDS) ? GAME_MODE_ENDLESS : GAME_MODE_ROUNDS;
                            break;
                        case MENU_ITEM_ZOMBIE_MODE:
                            menu->zombieMode = (menu->zombieMode == ZOMBIE_MODE_MIXED) ? ZOMBIE_MODE_ALL_IMAGES : ZOMBIE_MODE_MIXED;
                            break;
                        case MENU_ITEM_UPLOAD:
                            menu->showUploadPrompt = true;
                            break;
                        case MENU_ITEM_QUIT:
                            game->state = GAME_STATE_GAMEOVER;
                            break;
                    }
                    break;
                }
            }
        }
    }
}

void UIRender(const MenuState *menu) {
    if (!menu->active) return;
    
    DrawRectangle(0, 0, menu->screenWidth, menu->screenHeight, (Color){ 35, 35, 40, 200 });
    
    if (menu->showUploadPrompt) {
        DrawRectangle(0, 0, menu->screenWidth, menu->screenHeight, (Color){ 35, 35, 40, 230 });
        DrawText("UPLOAD GALLERY", menu->screenWidth / 2 - MeasureText("UPLOAD GALLERY", 40) / 2, 20, 40, (Color){ 220, 210, 190, 255 });
        DrawText("Drag and drop images here to upload", menu->screenWidth / 2 - MeasureText("Drag and drop images here to upload", 20) / 2, 60, 20, (Color){ 160, 155, 145, 255 });
        DrawText("Press ESC to close", menu->screenWidth / 2 - MeasureText("Press ESC to close", 20) / 2, menu->screenHeight - 40, 20, (Color){ 160, 155, 145, 255 });
        
        if (menu->lastUploadError[0] != '\0') {
            DrawText(menu->lastUploadError, menu->screenWidth / 2 - MeasureText(menu->lastUploadError, 20) / 2, 75, 20, (Color){ 140, 50, 50, 255 });
        }
        
        int cols = menu->screenWidth / (UPLOAD_THUMB_SIZE + UPLOAD_GALLERY_PADDING);
        if (cols < 1) cols = 1;
        
        for (int i = 0; i < menu->uploadedImageCount; i++) {
            int col = i % cols;
            int row = i / cols;
            float x = UPLOAD_GALLERY_PADDING + col * (UPLOAD_THUMB_SIZE + UPLOAD_GALLERY_PADDING);
            float y = 80 + UPLOAD_GALLERY_PADDING + row * (UPLOAD_THUMB_SIZE + UPLOAD_GALLERY_PADDING + 30);
            
            DrawTexturePro(menu->uploadedImages[i],
                (Rectangle){ 0, 0, (float)menu->uploadedImages[i].width, (float)menu->uploadedImages[i].height },
                (Rectangle){ x, y, (float)UPLOAD_THUMB_SIZE, (float)UPLOAD_THUMB_SIZE },
                (Vector2){ 0, 0 }, 0.0f, WHITE);
            DrawRectangleLines(x, y, UPLOAD_THUMB_SIZE, UPLOAD_THUMB_SIZE, (Color){ 40, 40, 45, 255 });
            
            DrawRectangle(x + UPLOAD_THUMB_SIZE - 24, y, 24, 24, (Color){ 120, 50, 50, 255 });
            DrawText("X", x + UPLOAD_THUMB_SIZE - 18, y + 4, 16, (Color){ 220, 210, 190, 255 });
        }
        
        if (menu->uploadedImageCount == 0) {
            DrawText("No images uploaded yet. Drag and drop images onto this window.", menu->screenWidth / 2 - 300, menu->screenHeight / 2 - 20, 20, (Color){ 160, 155, 145, 255 });
        }
        
        return;
    }
    
    int itemH = 50;
    int startY = menu->screenHeight / 2 - 100;
    const char *modeText = (menu->mode == GAME_MODE_ROUNDS) ? "ROUNDS" : "ENDLESS";
    const char *zombieModeText = (menu->zombieMode == ZOMBIE_MODE_MIXED) ? "MIXED" : "ALL IMAGES";
    const char *items[] = { "START GAME", TextFormat("MODE: %s", modeText), TextFormat("ZOMBIE MODE: %s", zombieModeText), "UPLOAD IMAGES", "QUIT" };
    
    DrawText("Each zombie kill: +100 score | Kills counter tracks total dead zombies", menu->screenWidth / 2 - MeasureText("Each zombie kill: +100 score | Kills counter tracks total dead zombies", 20) / 2, 20, 20, (Color){ 160, 155, 145, 255 });
    
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        Rectangle btn = { menu->screenWidth / 2 - 150, startY + i * itemH, 300, 40 };
        Color col = (i == menu->selectedItem) ? (Color){ 120, 50, 50, 255 } : (Color){ 180, 175, 165, 255 };
        DrawRectangleRec(btn, (Color){ col.r, col.g, col.b, 50 });
        DrawRectangleLinesEx(btn, 2, (Color){ col.r, col.g, col.b, 200 });
        int textW = MeasureText(items[i], 20);
        DrawText(items[i], menu->screenWidth / 2 - textW / 2, startY + i * itemH + 10, 20, (Color){ 220, 210, 190, 255 });
    }
}

void UIShutdown(MenuState *menu) {
    (void)menu;
}

bool UIIsImageUploadRequested(const MenuState *menu) {
    return menu->showUploadPrompt;
}

void UIAddUploadedImage(MenuState *menu, const char *path) {
    if (menu->uploadedImageCount >= MAX_UPLOADED_IMAGES) {
        snprintf(menu->lastUploadError, sizeof(menu->lastUploadError), "Maximum %d images allowed", MAX_UPLOADED_IMAGES);
        return;
    }
    
    Texture2D tex = LoadTexture(path);
    if (tex.id == 0) {
        snprintf(menu->lastUploadError, sizeof(menu->lastUploadError), "Failed to load image: %s", path);
        return;
    }
    
    if (tex.width > MAX_IMAGE_DIMENSION || tex.height > MAX_IMAGE_DIMENSION) {
        UnloadTexture(tex);
        snprintf(menu->lastUploadError, sizeof(menu->lastUploadError), "Image too large: %dx%d (max %dx%d)", tex.width, tex.height, MAX_IMAGE_DIMENSION, MAX_IMAGE_DIMENSION);
        return;
    }
    
    menu->uploadedImages[menu->uploadedImageCount] = tex;
    strncpy(menu->imagePaths[menu->uploadedImageCount], path, MAX_IMAGE_PATH - 1);
    menu->uploadedImageCount++;
    menu->lastUploadError[0] = '\0';
}

void UIClearUploadedImages(MenuState *menu) {
    for (int i = 0; i < menu->uploadedImageCount; i++) {
        UnloadTexture(menu->uploadedImages[i]);
    }
    menu->uploadedImageCount = 0;
    memset(menu->imagePaths, 0, sizeof(menu->imagePaths));
}
