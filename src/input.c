#include "input.h"

void InputUpdate(InputState *input) {
    input->mouseDelta = GetMouseDelta();
    input->mouseLeftDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    input->mouseLeftPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input->mouseRightDown = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    input->mouseRightPressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    input->scopePressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    input->reloadPressed = IsKeyPressed(KEY_R);
    input->interactPressed = IsKeyPressed(KEY_E);
    input->menuPressed = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P);
    input->escapePressed = IsKeyPressed(KEY_ESCAPE);
    input->scrollDelta = GetMouseWheelMove();
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