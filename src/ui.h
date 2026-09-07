#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "texture.h"
#include "input.h"

#define MAX_UPLOADED_IMAGES 16
#define MAX_IMAGE_PATH 256
#define UPLOAD_GALLERY_PADDING 20
#define UPLOAD_THUMB_SIZE 128

typedef struct Game Game;

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAMEOVER
} GameState;

typedef enum {
    GAME_MODE_ROUNDS,
    GAME_MODE_ENDLESS
} GameMode;

typedef enum {
    ZOMBIE_MODE_MIXED,
    ZOMBIE_MODE_ALL_IMAGES
} ZombieMode;

typedef enum {
    MENU_ITEM_START,
    MENU_ITEM_MODE,
    MENU_ITEM_ZOMBIE_MODE,
    MENU_ITEM_UPLOAD,
    MENU_ITEM_QUIT,
    MENU_ITEM_COUNT
} MenuItem;

typedef struct {
    bool active;
    int selectedItem;
    GameMode mode;
    ZombieMode zombieMode;
    Texture2D uploadedImages[MAX_UPLOADED_IMAGES];
    int uploadedImageCount;
    char imagePaths[MAX_UPLOADED_IMAGES][MAX_IMAGE_PATH];
    bool showUploadPrompt;
    float fadeAlpha;
    int screenWidth;
    int screenHeight;
} MenuState;

void UIInit(MenuState *menu, int screenWidth, int screenHeight);
void UIUpdate(MenuState *menu, InputState *input, Game *game);
void UIRender(const MenuState *menu);
void UIShutdown(MenuState *menu);
bool UIIsImageUploadRequested(const MenuState *menu);
void UIAddUploadedImage(MenuState *menu, const char *path);
void UIClearUploadedImages(MenuState *menu);

#endif
