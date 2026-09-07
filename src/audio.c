#include "audio.h"
#include <math.h>
#include <stdlib.h>

static Wave GenerateGunshotWave() {
    Wave wave = { 0 };
    wave.frameCount = 44100 * 1;
    wave.sampleRate = 44100;
    wave.sampleSize = 16;
    wave.channels = 1;
    short *samples = (short *)MemAlloc(wave.frameCount * sizeof(short));
    for (int i = 0; i < (int)wave.frameCount; i++) {
        float t = (float)i / wave.sampleRate;
        float env = expf(-t * 20.0f);
        float noise = ((float)(rand() % 2000 - 1000) / 1000.0f);
        float tone = sinf(2.0f * PI * 150.0f * t) * 0.5f;
        samples[i] = (short)((noise * 0.8f + tone * 0.2f) * env * 32767.0f * 0.5f);
    }
    wave.data = samples;
    return wave;
}

static Wave GenerateGrowlWave() {
    Wave wave = { 0 };
    wave.frameCount = 44100 * 2;
    wave.sampleRate = 44100;
    wave.sampleSize = 16;
    wave.channels = 1;
    short *samples = (short *)MemAlloc(wave.frameCount * sizeof(short));
    for (int i = 0; i < (int)wave.frameCount; i++) {
        float t = (float)i / wave.sampleRate;
        float env = (t < 0.1f) ? t / 0.1f : ((t > 1.5f) ? (2.0f - t) / 0.5f : 1.0f);
        float freq = 80.0f + sinf(t * 3.0f) * 20.0f;
        float tone = sinf(2.0f * PI * freq * t) * 0.6f;
        float noise = ((float)(rand() % 2000 - 1000) / 1000.0f) * 0.4f;
        samples[i] = (short)((tone + noise) * env * 32767.0f * 0.3f);
    }
    wave.data = samples;
    return wave;
}

void AudioInit(AudioManager *audio) {
    InitAudioDevice();
    Wave gunshotWave = GenerateGunshotWave();
    audio->gunshot = LoadSoundFromWave(gunshotWave);
    UnloadWave(gunshotWave);
    Wave growlWave = GenerateGrowlWave();
    audio->zombieGrowl = LoadSoundFromWave(growlWave);
    UnloadWave(growlWave);
    audio->zombieAttack = audio->gunshot;
    audio->playerHit = audio->zombieGrowl;
    audio->reload = audio->gunshot;
    audio->initialized = true;
}

void AudioShutdown(AudioManager *audio) {
    if (audio->initialized) {
        UnloadSound(audio->gunshot);
        UnloadSound(audio->zombieGrowl);
        CloseAudioDevice();
        audio->initialized = false;
    }
}

void AudioPlayGunshot(AudioManager *audio) { if (audio->initialized) PlaySound(audio->gunshot); }
void AudioPlayZombieGrowl(AudioManager *audio) { if (audio->initialized) PlaySound(audio->zombieGrowl); }
void AudioPlayZombieAttack(AudioManager *audio) { if (audio->initialized) PlaySound(audio->zombieAttack); }
void AudioPlayPlayerHit(AudioManager *audio) { if (audio->initialized) PlaySound(audio->playerHit); }
void AudioPlayReload(AudioManager *audio) { if (audio->initialized) PlaySound(audio->reload); }
void AudioUpdate(AudioManager *audio, float dt) { (void)audio; (void)dt; }