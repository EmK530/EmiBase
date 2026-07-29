#include <string.h>

#include "EmiBase/AudioManager.h"

static AudioChannel channels[AUDIO_MAX_CHANNELS];
static bool initialized = false;

int AudioManager_Init(void)
{
    if(initialized)
        return 1;
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
    initialized = true;
    return 1;
}

void AudioManager_Update()
{
    AudioChannel* channel = channels;
    for(int i = 0; i < AUDIO_MAX_CHANNELS; i++)
    {
        if(channel->state == ChannelState_Playing) {
            if(channel->type == ChannelType_Sound) {
                Sound* snd = (Sound*)channel->soundObject;
                if(!IsSoundPlaying(*snd))
                    channel->state = ChannelState_Stopped;
            } else {
                Music* snd = (Music*)channel->soundObject;
                if(!IsMusicStreamPlaying(*snd)) {
                    if(channel->looped) {
                        PlayMusicStream(*snd);
                    } else {
                        channel->state = ChannelState_Stopped;
                        channel->type = ChannelType_Empty;
                        UnloadMusicStream(*snd);
                        MemFree(channel->soundObject);
                        memset(channel, 0, sizeof(AudioChannel));
                    }
                } else {
                    UpdateMusicStream(*snd);
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
        if (ch->type == desired && ch->soundHash == hash && ch->state == ChannelState_Stopped)
            return ch;
        if (firstFree == -1 && ch->type == ChannelType_Empty && !ch->managed)
            firstFree = i;
        if ((!ch->managed||ch->state==ChannelState_Stopped) && ch->interruptible && (ch->type!=ChannelType_Empty||ch->state==ChannelState_Stopped) && ch->lastWriteId < oldestWrite)
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

void AudioManager_ResumeCh(int channel)
{
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot ResumeCh because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return;
    }
    AudioChannel* ch = channels + (channel-1);
    if(!ch->managed)
        return;
    if(ch->state == ChannelState_Paused) {
        if(ch->type == ChannelType_Music) {
            ResumeMusicStream(*(Music*)ch->soundObject);
        } else {
            ResumeSound(*(Sound*)ch->soundObject);
        }
        ch->state = ChannelState_Playing;
    }
}

void AudioManager_PauseCh(int channel)
{
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot PauseCh because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return;
    }
    AudioChannel* ch = channels + (channel-1);
    if(!ch->managed)
        return;
    if(ch->state == ChannelState_Playing) {
        if(ch->type == ChannelType_Music) {
            PauseMusicStream(*(Music*)ch->soundObject);
        } else {
            PauseSound(*(Sound*)ch->soundObject);
        }
        ch->state = ChannelState_Paused;
    }
}

void _AudioManager_StopChInner(int channel, bool bypass)
{
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot StopCh because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return;
    }
    AudioChannel* ch = channels + (channel-1);
    if(!bypass && !ch->managed)
        return;
    if(ch->state != ChannelState_Stopped) {
        if(ch->type == ChannelType_Music) {
            Music* snd = (Music*)ch->soundObject;
            StopMusicStream(*snd);
            ch->type = ChannelType_Empty;
            UnloadMusicStream(*snd);
            MemFree(ch->soundObject);
            memset(ch, 0, sizeof(AudioChannel));
        } else {
            StopSound(*(Sound*)ch->soundObject);
        }
        ch->state = ChannelState_Stopped;
    }
}

void AudioManager_StopCh(int channel)
{
    _AudioManager_StopChInner(channel, false);
}

void AudioManager_StopAll()
{
    for(int i = 0; i < AUDIO_MAX_CHANNELS; i++)
    {
        _AudioManager_StopChInner(i+1, true);
    }
}

void AudioManager_SetChVolume(int channel, float volume)
{
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot SetChVolume because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return;
    }
    AudioChannel* ch = channels + (channel-1);
    if(!ch->managed || ch->type == ChannelType_Empty)
        return;
    if(ch->volume == volume)
        return;
    if(ch->type == ChannelType_Music) {
        SetMusicVolume(*(Music*)ch->soundObject, volume);
    } else {
        SetSoundVolume(*(Sound*)ch->soundObject, volume);
    }
    ch->volume = volume;
}

void AudioManager_SetChPitch(int channel, float pitch)
{
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot SetChPitch because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return;
    }
    AudioChannel* ch = channels + (channel-1);
    if(!ch->managed || ch->type == ChannelType_Empty)
        return;
    if(ch->type == ChannelType_Music) {
        SetMusicPitch(*(Music*)ch->soundObject, pitch);
    } else {
        SetSoundPitch(*(Sound*)ch->soundObject, pitch);
    }
}

void AudioManager_SetChPan(int channel, float pan)
{
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot SetChPan because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return;
    }
    AudioChannel* ch = channels + (channel-1);
    if(!ch->managed || ch->type == ChannelType_Empty)
        return;
    if(ch->type == ChannelType_Music) {
        SetMusicPan(*(Music*)ch->soundObject, pan);
    } else {
        SetSoundPan(*(Sound*)ch->soundObject, pan);
    }
}

bool AudioManager_IsChPlaying(int channel)
{
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot SetChPan because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return false;
    }
    AudioChannel* ch = channels + (channel-1);
    if(!ch->managed || ch->type == ChannelType_Empty)
        return false;
    return ch->state == ChannelState_Playing;
}

void AudioManager_SeekCh(int channel, float position)
{
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot SetChPan because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return;
    }
    AudioChannel* ch = channels + (channel-1);
    if(!ch->managed || ch->type != ChannelType_Music)
        return;
    SeekMusicStream(*(Music*)ch->soundObject, position);
}

float AudioManager_GetChTimeLength(int channel)
{
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot SetChPan because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return 0.0f;
    }
    AudioChannel* ch = channels + (channel-1);
    if(!ch->managed || ch->type != ChannelType_Music || ch->state == ChannelState_Stopped)
        return 0.0f;
    return GetMusicTimeLength(*(Music*)ch->soundObject);
}

float AudioManager_GetChTimePlayed(int channel)
{
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot SetChPan because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return 0.0f;
    }
    AudioChannel* ch = channels + (channel-1);
    if(!ch->managed || ch->type != ChannelType_Music || ch->state == ChannelState_Stopped)
        return 0.0f;
    return GetMusicTimePlayed(*(Music*)ch->soundObject);
}

static bool AudioManager_LoadChannel(AudioChannel* ch, enum ChannelType type, uint64_t hash, const char* path, enum AudioFlags flags)
{
    if (ch->soundHash == hash)
        return true;
    if (ch->state != ChannelState_Stopped)
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
    ch->looped = flags & AudioFlag_Looped;
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
    if (ch->state != ChannelState_Stopped)
        StopSound(*snd);
    if (ch->volume != volume)
    {
        SetSoundVolume(*snd, volume);
        ch->volume = volume;
    }
    PlaySound(*snd);
    ch->state = ChannelState_Playing;
    ch->managed = managed;
    ch->lastWriteId = writeId++;
}

void _AudioManager_PlayMusicInner(AudioChannel* ch, const char* path, uint64_t hash, float volume, enum AudioFlags flags, bool managed)
{
    if (!AudioManager_LoadChannel(ch, ChannelType_Music, hash, path, flags))
        return;
    Music* snd = ch->soundObject;
    if (ch->state != ChannelState_Stopped)
        StopMusicStream(*snd);
    if (ch->volume != volume)
    {
        SetMusicVolume(*snd, volume);
        ch->volume = volume;
    }
    snd->looping = ch->looped;
    PlayMusicStream(*snd);
    ch->state = ChannelState_Playing;
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
    if(channel < 1 || channel > AUDIO_MAX_CHANNELS) {
        eprintf("[AudioManager] Cannot PlaySoundCh because requested channel %i is outside range available (%i-%i)\n", channel, 1, AUDIO_MAX_CHANNELS);
        return;
    }
    uint64_t hash = fnv1a_hash(path, strlen(path));
    AudioChannel* ch = channels + (channel-1);
    if (ch->state != ChannelState_Stopped && !ch->interruptible) {
        eprintf("[AudioManager] PlaySoundCh cannot play on busy uninterruptible channel %i\n", channel);
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
    if (ch->state != ChannelState_Stopped && !ch->interruptible) {
        eprintf("[AudioManager] PlayMusicCh cannot play on busy uninterruptible channel %i\n", channel);
        return;
    }
    _AudioManager_PlayMusicInner(ch, path, hash, volume, flags, true);
}
