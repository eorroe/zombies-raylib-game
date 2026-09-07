#ifndef INPUT_H
#define INPUT_H

#include "raylib.h"

typedef struct {
    Vector2 mouseDelta;
    bool mouseLeftDown;
    bool mouseLeftPressed;
    bool mouseRightDown;
    bool mouseRightPressed;
    bool scopePressed;
    bool reloadPressed;
    bool interactPressed;
    bool menuPressed;
    bool escapePressed;
    float scrollDelta;
} InputState;

void InputUpdate(InputState *input);
bool InputIsKeyPressed(InputState *input, int key);
bool InputIsMouseButtonPressed(InputState *input, int button);
Vector2 InputGetMouseDelta(InputState *input);

#endif
