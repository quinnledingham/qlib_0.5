#ifndef AUDIO_H
#define AUDIO_H

struct loaded_sound
{
    uint32 SampleCount;
    uint32 ChannelCount;
    int16 *Samples[2];
};

struct playing_sound
{
    v2 CurrentVolume;
    v2 dCurrentVolume;
    v2 TargetVolume;
    
    sound_id LoadedSound;
    
    int32 SamplesPlayed;
    playing_sound *Next;
};

struct audio_state
{
    bool32 Initialized;
    int16 *Channel0;
    int16 *Channel1;
    
    uint32 NumOfSounds = 0;
    playing_sound PlayingSounds[10];
    playing_sound *FirstPlayingSound;
    playing_sound *FirstFreePlayingSound;
    
    v2 MasterVolume;
    
    void *Assets;
    
    qlib_bool Paused;
};

//internal void PlaySound(audio_state *AudioState, loaded_sound *LoadedSound);
//internal void PlayLoadedSound(audio_state *AudioState, platform_sound_output_buffer *SoundBuffer);
internal loaded_sound LoadWAV(const char *FileName);

#define AUDIO_S16_BYTES 4

#endif //AUDIO_H
