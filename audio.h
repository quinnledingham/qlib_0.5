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
    
    loaded_sound *LoadedSound;
    int32 SamplesPlayed;
    playing_sound *Next;
};

struct audio_state
{
    bool32 Initialized;
    real32 *Channel0;
    real32 *Channel1;
    
    uint32 NumOfSounds = 0;
    playing_sound PlayingSounds[10];
    playing_sound *FirstPlayingSound;
    playing_sound *FirstFreePlayingSound;
    
    v2 MasterVolume;
};



#endif //AUDIO_H
