#ifndef AUDIO_H
#define AUDIO_H

#include "raylib.h"

typedef struct {
    Sound gunshot;
    Sound zombieGrowl;
    Sound zombieAttack;
    Sound playerHit;
    Sound reload;
    bool initialized;
} AudioManager;

void AudioInit(AudioManager *audio);
void AudioShutdown(AudioManager *audio);
void AudioPlayGunshot(AudioManager *audio);
void AudioPlayZombieGrowl(AudioManager *audio);
void AudioPlayZombieAttack(AudioManager *audio);
void AudioPlayPlayerHit(AudioManager *audio);
void AudioPlayReload(AudioManager *audio);
void AudioUpdate(AudioManager *audio, float dt);

#endif
