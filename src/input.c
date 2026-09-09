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
    input->scrollDelta = GetMouseWheelMove();
    input->upPressed = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
    input->downPressed = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S);
    input->leftPressed = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A);
    input->rightPressed = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D);
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