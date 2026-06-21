#pragma once
#include "raylib.h"

#define MAX_CHANNELS 16

typedef struct AudioAsset AudioAsset;

int AudioManager_Init(void);
void AudioManager_Shutdown(void);

AudioAsset* AudioManager_Get(const char* path);

void AudioManager_PlayChannel(int channel, const char* path, int loop);

void AudioManager_StopChannel(int channel);
void AudioManager_StopAll(void);

void AudioManager_SetChannelVolume(int channel, float volume);
int  AudioManager_IsChannelPlaying(int channel);

void AudioManager_Update(void);

void AudioManager_Preload(const char** paths, int count);