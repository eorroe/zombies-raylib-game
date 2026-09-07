#include "debug.h"
#include <stdio.h>
#include "raymath.h"
#include <stdarg.h>
#include <string.h>

static const char *DebugLevelToString(DebugLevel level) {
    switch (level) {
        case DEBUG_INFO: return "INFO";
        case DEBUG_WARN: return "WARN";
        case DEBUG_ERROR: return "ERROR";
        case DEBUG_SUCCESS: return "OK";
        default: return "UNKNOWN";
    }
}

static Color DebugLevelToColor(DebugLevel level) {
    switch (level) {
        case DEBUG_INFO: return SKYBLUE;
        case DEBUG_WARN: return YELLOW;
        case DEBUG_ERROR: return RED;
        case DEBUG_SUCCESS: return LIME;
        default: return WHITE;
    }
}

void DebugInit(DebugState *debug, float messageLifetime) {
    debug->enabled = false;
    debug->entryCount = 0;
    debug->scrollOffset = 0;
    debug->autoScroll = true;
    debug->messageLifetime = messageLifetime;
    memset(debug->entries, 0, sizeof(debug->entries));
}

void DebugShutdown(DebugState *debug) {
    (void)debug;
}

void DebugLog(DebugState *debug, const char *message, DebugLevel level) {
    if (debug->entryCount >= DEBUG_MAX_ENTRIES) {
        memmove(&debug->entries[0], &debug->entries[1], sizeof(DebugLogEntry) * (DEBUG_MAX_ENTRIES - 1));
        debug->entryCount--;
    }
    
    DebugLogEntry *entry = &debug->entries[debug->entryCount++];
    strncpy(entry->message, message, DEBUG_MAX_MESSAGE - 1);
    entry->message[DEBUG_MAX_MESSAGE - 1] = '\0';
    entry->time = GetTime();
    entry->level = level;
}

void DebugLogf(DebugState *debug, DebugLevel level, const char *fmt, ...) {
    char buffer[DEBUG_MAX_MESSAGE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, DEBUG_MAX_MESSAGE, fmt, args);
    va_end(args);
    DebugLog(debug, buffer, level);
}

void DebugUpdate(DebugState *debug, float dt) {
    (void)dt;
    if (!debug->enabled) return;
    
    float currentTime = GetTime();
    while (debug->entryCount > 0 && currentTime - debug->entries[0].time > debug->messageLifetime) {
        memmove(&debug->entries[0], &debug->entries[1], sizeof(DebugLogEntry) * (debug->entryCount - 1));
        debug->entryCount--;
        if (debug->scrollOffset > 0) debug->scrollOffset--;
    }
}

void DebugRender(DebugState *debug, int screenWidth, int screenHeight) {
    if (!debug->enabled) return;
    
    int lineHeight = 16;
    int padding = 10;
    int maxVisibleLines = (screenHeight - padding * 2) / lineHeight;
    int visibleStart = debug->scrollOffset;
    int visibleEnd = visibleStart + maxVisibleLines;
    if (visibleEnd > debug->entryCount) visibleEnd = debug->entryCount;
    
    DrawRectangle(padding, padding, screenWidth - padding * 2, screenHeight - padding * 2, Fade(BLACK, 0.8f));
    DrawRectangleLines(padding, padding, screenWidth - padding * 2, screenHeight - padding * 2, GREEN);
    
    DrawText("DEBUG CONSOLE (F1 toggle, F2 clear)", padding + 5, padding + 5, 14, GREEN);
    DrawText(TextFormat("Logs: %d/%d", debug->entryCount, DEBUG_MAX_ENTRIES), padding + 5, padding + 22, 12, GRAY);
    
    int textY = padding + 40;
    for (int i = visibleStart; i < visibleEnd && i < DEBUG_MAX_ENTRIES; i++) {
        DebugLogEntry *entry = &debug->entries[i];
        Color levelColor = DebugLevelToColor(entry->level);
        float age = GetTime() - entry->time;
        float alpha = 1.0f;
        if (age > debug->messageLifetime * 0.7f) {
            alpha = 1.0f - (age - debug->messageLifetime * 0.7f) / (debug->messageLifetime * 0.3f);
            alpha = Clamp(alpha, 0.0f, 1.0f);
        }
        
        DrawText(TextFormat("[%.1f] [%s] %s", entry->time, DebugLevelToString(entry->level), entry->message), padding + 5, textY, 12, Fade(levelColor, alpha));
        textY += lineHeight;
    }
    
    if (debug->entryCount == 0) {
        DrawText("No logs yet...", padding + 5, textY, 12, GRAY);
    }
}

void DebugClear(DebugState *debug) {
    debug->entryCount = 0;
    debug->scrollOffset = 0;
    memset(debug->entries, 0, sizeof(debug->entries));
}

void DebugToggle(DebugState *debug) {
    debug->enabled = !debug->enabled;
}
