#pragma once

#include "EmiBase.h"

#ifndef AUDIO_MAX_CHANNELS
    #define AUDIO_MAX_CHANNELS 32
#endif

enum ChannelType {
    ChannelType_Empty,
    ChannelType_Sound,
    ChannelType_Music
};

enum AudioFlags {
    AudioFlag_NONE = 0,
    AudioFlag_Uninterruptible = 1,
    AudioFlag_External = 2
};

typedef struct {
    uint8_t type;          // Sound type
    bool interruptible;    // Can this audio be interrupted to play another sound?
    uint8_t state;         // 0 = Not playing | 1 = Playing | 2 = Paused
    bool managed;          // Sounds played through manual channel selection, won't be overwritten by the engine's channel picker
    bool external;         // Is the audio hash part of an external asset or from ContentManager
    int lastWriteId;       // Tracking for the engine to find the best overwrite target
    float volume;          // Tracking for the engine to know when to run SetSoundVolume
    void* soundObject;     // Allocated block of memory containing either a Music or Sound struct
    uint64_t soundHash;    // Hash for the sound path we're playing
} AudioChannel;

extern uint64_t fnv1a_hash(const char* filePath, size_t pathLen);

int AudioManager_Init();

void AudioManager_GC(); // Free memory of unnecessary audio instances

void AudioManager_Update();

void AudioManager_PlaySound(const char* path, float volume, enum AudioFlags flags);
void AudioManager_PlaySoundCh(const char* path, int channel, float volume, enum AudioFlags flags);

void AudioManager_PlayMusic(const char* path, float volume, enum AudioFlags flags);
void AudioManager_PlayMusicCh(const char* path, int channel, float volume, enum AudioFlags flags);