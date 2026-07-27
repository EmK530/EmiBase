#include <string.h>

#include "EmiBase/AudioManager.h"

static AudioChannel channels[AUDIO_MAX_CHANNELS];

int AudioManager_Init(void)
{
    InitAudioDevice();
    if(!IsAudioDeviceReady())
    {
        eprintf("[AudioManager] Failed to init AudioDevice\n");
        return 0;
    }
    memset(channels, 0, sizeof(AudioChannel) * AUDIO_MAX_CHANNELS);
    for (int i = 0; i < AUDIO_MAX_CHANNELS; i++)
        channels[i].volume = 1.0f;
    eprintf("[AudioManager] Ready!\n");
    return 1;
}

void AudioManager_GC()
{

}

void AudioManager_Update()
{
    AudioChannel* channel = channels;
    for(int i = 0; i < AUDIO_MAX_CHANNELS; i++)
    {
        if(channel->state == 1) {
            if(channel->type == ChannelType_Sound) {
                Sound* snd = (Sound*)channel->soundObject;
                if(!IsSoundPlaying(*snd))
                    channel->state = 0;
            } else {
                Music* snd = (Music*)channel->soundObject;
                UpdateMusicStream(*snd);
                if(!IsMusicStreamPlaying(*snd))
                {
                    channel->state = 0;
                    UnloadMusicStream(*snd);
                    MemFree(channel->soundObject);
                    memset(channel, 0, sizeof(AudioChannel));
                }
            }
        }
        channel++;
    }
}

static AudioChannel* AudioManager_GetChannel(uint64_t hash, enum ChannelType desired)
{
    int firstFree = -1;
    int bestReplace = -1;
    uint32_t oldestWrite = UINT32_MAX;

    for (int i = 0; i < AUDIO_MAX_CHANNELS; i++)
    {
        AudioChannel* ch = &channels[i];
        if (ch->type == desired && ch->soundHash == hash && ch->state == 0)
            return ch;
        if (firstFree == -1 && ch->type == ChannelType_Empty && !ch->managed)
            firstFree = i;
        if ((!ch->managed||ch->state==0) && ch->interruptible && (ch->type!=ChannelType_Empty||ch->state==0) && ch->lastWriteId < oldestWrite)
        {
            oldestWrite = ch->lastWriteId;
            bestReplace = i;
        }
    }
    if (firstFree != -1)
        return &channels[firstFree];
    if (bestReplace != -1)
        return &channels[bestReplace];
    return NULL;
}

static bool AudioManager_LoadChannel(AudioChannel* ch, enum ChannelType type, uint64_t hash, const char* path, enum AudioFlags flags)
{
    if (ch->soundHash == hash)
        return true;
    if (ch->state)
    {
        switch (ch->type)
        {
            case ChannelType_Sound:
                UnloadSound(*(Sound*)ch->soundObject);
                break;
            case ChannelType_Music:
                UnloadMusicStream(*(Music*)ch->soundObject);
                break;
        }
        MemFree(ch->soundObject);
    }
    memset(ch, 0, sizeof(*ch));
    ch->volume = 1.0f;
    ch->type = type;
    ch->soundHash = hash;
    ch->interruptible = !(flags & AudioFlag_Uninterruptible);
    if (type == ChannelType_Sound)
    {
        Sound snd = ContentManager_LoadSound(path);
        if (!IsSoundValid(snd))
            return false;
        ch->soundObject = emalloc_strict(sizeof(Sound));
        *(Sound*)ch->soundObject = snd;
    } else {
        Music music = ContentManager_LoadMusic(path);
        if (!IsMusicValid(music))
            return false;
        ch->soundObject = emalloc_strict(sizeof(Music));
        *(Music*)ch->soundObject = music;
    }
    return true;
}

static int writeId = 0;

void _AudioManager_PlaySoundInner(AudioChannel* ch, const char* path, uint64_t hash, float volume, enum AudioFlags flags, bool managed)
{
    if (!AudioManager_LoadChannel(ch, ChannelType_Sound, hash, path, flags))
        return;
    Sound* snd = ch->soundObject;
    if (ch->state)
        StopSound(*snd);
    if (ch->volume != volume)
    {
        SetSoundVolume(*snd, volume);
        ch->volume = volume;
    }
    PlaySound(*snd);
    ch->state = true;
    ch->managed = managed;
    ch->lastWriteId = writeId++;
}

void _AudioManager_PlayMusicInner(AudioChannel* ch, const char* path, uint64_t hash, float volume, enum AudioFlags flags, bool managed)
{
    if (!AudioManager_LoadChannel(ch, ChannelType_Music, hash, path, flags))
        return;
    Music* snd = ch->soundObject;
    if (ch->state)
        StopMusicStream(*snd);
    if (ch->volume != volume)
    {
        SetMusicVolume(*snd, volume);
        ch->volume = volume;
    }
    PlayMusicStream(*snd);
    ch->state = true;
    ch->managed = managed;
    ch->lastWriteId = writeId++;
}

void AudioManager_PlaySound(const char* path, float volume, enum AudioFlags flags)
{
    if (!path)
        return;
    uint64_t hash = fnv1a_hash(path, strlen(path));
    AudioChannel* ch = AudioManager_GetChannel(hash, ChannelType_Sound);
    if (!ch) {
        eprintf("[AudioManager] No free interruptible channels to play Sound: %s\n", path);
        return;
    }
    _AudioManager_PlaySoundInner(ch, path, hash, volume, flags, false);
}

void AudioManager_PlayMusic(const char* path, float volume, enum AudioFlags flags)
{
    if (!path)
        return;
    uint64_t hash = fnv1a_hash(path, strlen(path));
    AudioChannel* ch = AudioManager_GetChannel(hash, ChannelType_Music);
    if (!ch) {
        eprintf("[AudioManager] No free interruptible channels to play MusicStream: %s\n", path);
        return;
    }
    _AudioManager_PlayMusicInner(ch, path, hash, volume, flags, false);
}

void AudioManager_PlaySoundCh(const char* path, int channel, float volume, enum AudioFlags flags)
{
    if (!path)
        return;
    uint64_t hash = fnv1a_hash(path, strlen(path));
    AudioChannel* ch = AudioManager_GetChannel(hash, ChannelType_Sound);
    if (!ch) {
        eprintf("[AudioManager] No free interruptible channels to play Sound: %s\n", path);
        return;
    }
    _AudioManager_PlaySoundInner(ch, path, hash, volume, flags, true);
}

void AudioManager_PlayMusicCh(const char* path, int channel, float volume, enum AudioFlags flags)
{
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot PlayMusicCh because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return;
    }
    uint64_t hash = fnv1a_hash(path, strlen(path));
    AudioChannel* ch = channels + (channel-1);
    if (ch->state && !ch->interruptible) {
        eprintf("[AudioManager] PlayMusicCh cannot play on busy uninterruptible channel %i\n", channel);
        return;
    }
    _AudioManager_PlayMusicInner(ch, path, hash, volume, flags, true);
}