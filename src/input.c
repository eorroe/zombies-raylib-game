#include "input.h"

void InputUpdate(InputState *input, bool menuActive) {
    input->mouseDelta = GetMouseDelta();
    input->mouseLeftDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    input->mouseLeftPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input->mouseLeftReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    input->mouseRightDown = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    input->mouseRightPressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    input->scopePressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    input->reloadPressed = IsKeyPressed(KEY_R);
    input->interactPressed = IsKeyPressed(KEY_E);
    input->menuPressed = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P);
    input->escapePressed = IsKeyPressed(KEY_ESCAPE);
    input->cameraTogglePressed = IsKeyPressed(KEY_C);
    input->shiftPressed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    input->ctrlPressed = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    input->spacePressed = IsKeyPressed(KEY_SPACE);
    input->scrollDelta = GetMouseWheelMove();
    input->upPressed = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
    input->downPressed = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
    input->leftPressed = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    input->rightPressed = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
    input->enterPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
}

bool InputIsKeyPressed(InputState *input, int key) {
    (void)input;
    return IsKeyPressed(key);
}

bool InputIsMouseButtonPressed(InputState *input, int button) {
    (void)input;
    return IsMouseButtonPressed(button);
}

Vector2 InputGetMouseDelta(InputState *input) {
    return input->mouseDelta;
}