#ifndef DEBUG_H
#define DEBUG_H

#include "raylib.h"

#define DEBUG_MAX_ENTRIES 256
#define DEBUG_MAX_MESSAGE 256

typedef enum {
    DEBUG_INFO,
    DEBUG_WARN,
    DEBUG_ERROR,
    DEBUG_SUCCESS
} DebugLevel;

typedef struct {
    char message[DEBUG_MAX_MESSAGE];
    float time;
    DebugLevel level;
} DebugLogEntry;

typedef struct {
    bool enabled;
    DebugLogEntry entries[DEBUG_MAX_ENTRIES];
    int entryCount;
    int scrollOffset;
    bool autoScroll;
    float messageLifetime;
} DebugState;

void DebugInit(DebugState *debug, float messageLifetime);
void DebugShutdown(DebugState *debug);
void DebugLog(DebugState *debug, const char *message, DebugLevel level);
void DebugLogf(DebugState *debug, DebugLevel level, const char *fmt, ...);
void DebugUpdate(DebugState *debug, float dt);
void DebugRender(DebugState *debug, int screenWidth, int screenHeight);
void DebugClear(DebugState *debug);
void DebugToggle(DebugState *debug);

#endif
