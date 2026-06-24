#include <string.h>
#include <stdio.h>

#include "EmiBase.h"

typedef enum {
    AUDIO_SOUND,
    AUDIO_MUSIC
} AudioType;

struct AudioAsset {
    char* path;

    Sound sound;
    Music music;

    int isMusicLoaded;
    int isSoundLoaded;
};

typedef struct {
    AudioAsset* items;
    int count;
    int capacity;
} AudioCache;

typedef struct {
    AudioAsset* asset;
    float volume;
    int active;
    int loop;

    int playing;        // NEW
} AudioChannel;

static AudioCache cache = {0};
static AudioChannel channels[MAX_CHANNELS] = {0};

static AudioAsset* Find(const char* path)
{
    for (int i = 0; i < cache.count; i++)
    {
        if (strcmp(cache.items[i].path, path) == 0)
            return &cache.items[i];
    }
    return NULL;
}


//------------------------------------------------------------
// Core API
//------------------------------------------------------------

int AudioManager_Init(void)
{
    InitAudioDevice();
    if(!IsAudioDeviceReady())
    {
        eprintf("[AudioManager] Failed to init AudioDevice\n");
        return 0;
    }

    for (int i = 0; i < MAX_CHANNELS; i++)
        channels[i].volume = 1.0f;
    eprintf("[AudioManager] Ready!\n");
    return 1;
}

void AudioManager_Shutdown(void)
{
    for (int i = 0; i < cache.count; i++)
    {
        if (cache.items[i].isSoundLoaded)
            UnloadSound(cache.items[i].sound);

        if (cache.items[i].isMusicLoaded)
            UnloadMusicStream(cache.items[i].music);

        MemFree(cache.items[i].path);
    }

    MemFree(cache.items);
    cache = (AudioCache){0};

    CloseAudioDevice();
}


//------------------------------------------------------------
// Asset loading
//------------------------------------------------------------

AudioAsset* AudioManager_Get(const char* path)
{
    AudioAsset* found = Find(path);
    if (found)
        return found;

    if (cache.count >= cache.capacity)
    {
        cache.capacity = cache.capacity ? cache.capacity * 2 : 8;
        cache.items = MemRealloc(cache.items, sizeof(AudioAsset) * cache.capacity);
    }

    AudioAsset* a = &cache.items[cache.count++];
    a->path = _strdup(path);

    // Don't load anything yet
    a->isMusicLoaded = 0;
    a->isSoundLoaded = 0;

    return a;
}


//------------------------------------------------------------
// Playback
//------------------------------------------------------------

void AudioManager_PlayChannel(int channel, const char* finalPath, int loop)
{
    if (channel < 0 || channel >= MAX_CHANNELS) return;

    AudioAsset* a = AudioManager_Get(finalPath);

    AudioManager_StopChannel(channel);

    channels[channel].asset = a;
    channels[channel].active = 1;
    channels[channel].loop = loop;
    channels[channel].playing = 1;

    if (loop)
    {
        // MUSIC path
        if (!a->isMusicLoaded)
        {
            a->music = ContentManager_LoadMusic(finalPath);
            a->isMusicLoaded = 1;
        }

        SetMusicVolume(a->music, channels[channel].volume);
        PlayMusicStream(a->music);
    }
    else
    {
        // SOUND path
        if (!a->isSoundLoaded)
        {
            a->sound = ContentManager_LoadSound(finalPath);
            a->isSoundLoaded = 1;
        }

        SetSoundVolume(a->sound, channels[channel].volume);
        PlaySound(a->sound);
    }
}


//------------------------------------------------------------
// Control
//------------------------------------------------------------

void AudioManager_StopChannel(int channel)
{
    if (channel < 0 || channel >= MAX_CHANNELS) return;

    if (!channels[channel].active || !channels[channel].asset) return;

    AudioAsset* a = channels[channel].asset;

    if (channels[channel].loop)
        StopMusicStream(a->music);
    else
        StopSound(a->sound);

    channels[channel].active = 0;
    channels[channel].asset = NULL;
}

void AudioManager_StopAll(void)
{
    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        if (!channels[i].active || !channels[i].asset) continue;

        AudioAsset* a = channels[i].asset;

        if (channels[i].loop)
            StopMusicStream(a->music);
        else
            StopSound(a->sound);

        channels[i].active = 0;
        channels[i].asset = NULL;
    }
}

void AudioManager_SetChannelVolume(int channel, float volume)
{
    if (channel < 0 || channel >= MAX_CHANNELS) return;

    channels[channel].volume = volume;

    if (!channels[channel].active || !channels[channel].asset) return;

    AudioAsset* a = channels[channel].asset;

    if (channels[channel].loop)
        SetMusicVolume(a->music, volume);
    else
        SetSoundVolume(a->sound, volume);
}

int AudioManager_IsChannelPlaying(int channel)
{
    if (channel < 0 || channel >= MAX_CHANNELS) return 0;

    if (!channels[channel].active || !channels[channel].asset) return 0;

    AudioAsset* a = channels[channel].asset;

    if (channels[channel].loop)
        return IsMusicStreamPlaying(a->music);
    else
        return IsSoundPlaying(a->sound);
}


//------------------------------------------------------------
// Update (must be called every frame)
//------------------------------------------------------------

void AudioManager_Update(void)
{
    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        if (!channels[i].active || !channels[i].asset) continue;

        AudioAsset* a = channels[i].asset;

        if (channels[i].loop)
        {
            UpdateMusicStream(a->music);

            // only music exists in loop mode now
            if (!IsMusicStreamPlaying(a->music))
                PlayMusicStream(a->music);
        }
        else
        {
            if (!IsSoundPlaying(a->sound))
            {
                channels[i].active = 0;
                channels[i].playing = 0;
            }
        }
    }
}


//------------------------------------------------------------
// Preload
//------------------------------------------------------------

void AudioManager_Preload(const char** paths, int count)
{
    for (int i = 0; i < count; i++)
    {
        AudioManager_Get(paths[i]);
    }
}