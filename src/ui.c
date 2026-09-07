#include "ui.h"
#include "game.h"
#include "raymath.h"
#include <string.h>

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
    memset(menu->uploadedImages, 0, sizeof(menu->uploadedImages));
}

void UIUpdate(MenuState *menu, InputState *input, Game *game) {
    if (menu->fadeAlpha < 1.0f) menu->fadeAlpha += 0.05f;
    
    if (input->menuPressed && !menu->active) {
        menu->active = true;
        menu->selectedItem = 0;
    }
    
    if (menu->active) {
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
                            GameInit(game);
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
        
        if (menu->showUploadPrompt) {
            if (input->interactPressed) {
                menu->showUploadPrompt = false;
            }
        }
    }
}

void UIRender(const MenuState *menu) {
    if (!menu->active) return;
    
    DrawRectangle(0, 0, menu->screenWidth, menu->screenHeight, ColorAlpha(BLACK, 0.7f * menu->fadeAlpha));
    
    int itemH = 50;
    int startY = menu->screenHeight / 2 - 100;
    const char *modeText = (menu->mode == GAME_MODE_ROUNDS) ? "ROUNDS" : "ENDLESS";
    const char *zombieModeText = (menu->zombieMode == ZOMBIE_MODE_MIXED) ? "MIXED" : "ALL IMAGES";
    const char *items[] = { "START GAME", TextFormat("MODE: %s", modeText), TextFormat("ZOMBIE MODE: %s", zombieModeText), "UPLOAD IMAGES", "QUIT" };
    
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        Rectangle btn = { menu->screenWidth / 2 - 150, startY + i * itemH, 300, 40 };
        Color col = (i == menu->selectedItem) ? RED : LIGHTGRAY;
        DrawRectangleRec(btn, ColorAlpha(col, 0.3f * menu->fadeAlpha));
        DrawRectangleLinesEx(btn, 2, ColorAlpha(col, 0.8f * menu->fadeAlpha));
        int textW = MeasureText(items[i], 20);
        DrawText(items[i], menu->screenWidth / 2 - textW / 2, startY + i * itemH + 10, 20, WHITE);
    }
    
    if (menu->showUploadPrompt) {
        DrawText("Drag and drop images here or press I to open file picker", menu->screenWidth / 2 - 300, menu->screenHeight - 100, 20, YELLOW);
    }
}

void UIShutdown(MenuState *menu) {
    (void)menu;
}

bool UIIsImageUploadRequested(const MenuState *menu) {
    return menu->showUploadPrompt;
}

void UIAddUploadedImage(MenuState *menu, const char *path) {
    if (menu->uploadedImageCount >= MAX_UPLOADED_IMAGES) return;
    strncpy(menu->imagePaths[menu->uploadedImageCount], path, MAX_IMAGE_PATH - 1);
    menu->uploadedImages[menu->uploadedImageCount] = LoadTexture(path);
    if (menu->uploadedImages[menu->uploadedImageCount].id != 0) {
        menu->uploadedImageCount++;
    }
}

void UIClearUploadedImages(MenuState *menu) {
    for (int i = 0; i < menu->uploadedImageCount; i++) {
        UnloadTexture(menu->uploadedImages[i]);
    }
    menu->uploadedImageCount = 0;
    memset(menu->imagePaths, 0, sizeof(menu->imagePaths));
}