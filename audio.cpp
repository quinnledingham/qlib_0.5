/*
internal void
OutputTestSineWave(game_state *GameState, platform_sound_output_buffer *SoundBuffer, int ToneHz)
{
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;    
    
    int16 *SampleOut = SoundBuffer->Samples;
    for(int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
        // TODO(casey): Draw this out for people
#if 1
        real32 SineValue = sinf(GameState->tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
#else
        int16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
        
#if 1
        GameState->tSine += Tau32*1.0f/(real32)WavePeriod;
        if(GameState->tSine > Tau32)
        {
            GameState->tSine -= Tau32;
        }
#endif
    }
}
*/


internal void
PlaySound(audio_state *AudioState, loaded_sound *LoadedSound)
{
    playing_sound *Sound = &AudioState->PlayingSounds[AudioState->NumOfSounds++];
    Sound->SamplesPlayed = 0;
    // TODO(casey): Should these default to 0.5f/0.5f for centerred?
    Sound->CurrentVolume = Sound->TargetVolume = v2(0.5f, 0.5f);
    Sound->LoadedSound = LoadedSound;
}

internal void
ChangeVolume(audio_state *AudioState, playing_sound *Sound, real32 FadeDurationInSeconds, v2 Volume)
{
    if (FadeDurationInSeconds <= 0.0f)
    {
        Sound->CurrentVolume = Sound->TargetVolume = Volume;
    }
    else
    {
        real32 OneOverFade = 1.0f / FadeDurationInSeconds;
        Sound->TargetVolume = Volume;
        Sound->dCurrentVolume = OneOverFade*(Sound->TargetVolume - Sound->CurrentVolume);
    }
}

internal void
PlayLoadedSound(audio_state *AudioState, platform_sound_output_buffer *SoundBuffer)
{
    if (!AudioState->Initialized) {
        
        AudioState->Channel0 = (real32*)qalloc(4000);
        AudioState->Channel1 = (real32*)qalloc(4000);
        AudioState->Initialized = true;
    }
    
    if (SoundBuffer->SampleCount > 1000|| SoundBuffer->SampleCount == 0)
        return;
    //fprintf(stderr, "%d\n", SoundBuffer->SampleCount);
    
    {
        real32 *Dest0 = AudioState->Channel0;
        real32 *Dest1 = AudioState->Channel1;
        for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
        {
            *Dest0++ = 0.0f;
            *Dest1++ = 0.0f;
        }
    }
    
    v2 Volume = v2(0.5f, 0.5f);
    real32 SecondsPerSample = 1.0f / (real32)SoundBuffer->SamplesPerSecond;
    v2 dVolume = SecondsPerSample * v2(0.0f, 0.0f);
    
    for (uint32 i = 0; i < AudioState->NumOfSounds; i++) {
        real32 *Dest0 = AudioState->Channel0;
        real32 *Dest1 = AudioState->Channel1;
        
        playing_sound *PlayingSound = &AudioState->PlayingSounds[i];
        if (PlayingSound != 0)
        {
            loaded_sound *LoadedSound = PlayingSound->LoadedSound;
            u32 SamplesRemainingInSound = LoadedSound->SampleCount - PlayingSound->SamplesPlayed;
            if (SamplesRemainingInSound > (uint32)SoundBuffer->SampleCount)
                SamplesRemainingInSound = (uint32)SoundBuffer->SampleCount;
            
            // TODO(casey): Handle stereo!
            for (uint32 SampleIndex = 0; SampleIndex < (uint32)SoundBuffer->SampleCount; ++SampleIndex)
            {
                uint32 TestSoundSampleIndex = (PlayingSound->SamplesPlayed + SampleIndex) % LoadedSound->SampleCount;
                real32 SampleValue = LoadedSound->Samples[0][TestSoundSampleIndex];
                *Dest0++ += AudioState->MasterVolume.v[0] * Volume.v[0] * SampleValue;
                *Dest1++ += AudioState->MasterVolume.v[1] * Volume.v[1] * SampleValue;
                
                Volume = Volume + dVolume;
            }
            
            PlayingSound->SamplesPlayed += (uint32)SoundBuffer->SampleCount;
            
            if ((uint32)PlayingSound->SamplesPlayed >= LoadedSound->SampleCount)
            {
                AudioState->NumOfSounds--;
            }
        }
    }
    // NOTE(casey): Convert to 16-bit
    {
        real32 *Source0 = AudioState->Channel0;
        real32 *Source1 = AudioState->Channel1;
        
        int16 *SampleOut = SoundBuffer->Samples;
        for (uint32 SampleIndex = 0; SampleIndex < (uint32)SoundBuffer->SampleCount; ++SampleIndex)
        {
            // TODO(casey): Once this is in SIMD, clamp!
            
            *SampleOut++ = (int16)(*Source0++ + 0.5f);
            *SampleOut++ = (int16)(*Source1++ + 0.5f);
        }
    }
}

internal void
InitializeAudioState(audio_state *AudioState)
{
    AudioState->MasterVolume = v2(1.0f, 1.0f);
}

//
// Loading WAV Files
//

struct WAVE_header
{
    uint32 RIFFID;
    uint32 Size;
    uint32 WAVEID;
};

#define RIFF_CODE(a, b, c, d) (((uint32)(a) << 0) | ((uint32)(b) << 8) | ((uint32)(c) << 16) | ((uint32)(d) << 24))
enum
{
    WAVE_ChunkID_fmt = RIFF_CODE('f', 'm', 't', ' '),
    WAVE_ChunkID_data = RIFF_CODE('d', 'a', 't', 'a'),
    WAVE_ChunkID_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
    WAVE_ChunkID_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
};
struct WAVE_chunk
{
    uint32 ID;
    uint32 Size;
};

struct WAVE_fmt
{
    uint16 wFormatTag;
    uint16 nChannels;
    uint32 nSamplesPerSec;
    uint32 nAvgBytesPerSec;
    uint16 nBlockAlign;
    uint16 wBitsPerSample;
    uint16 cbSize;
    uint16 wValidBitsPerSample;
    uint32 dwChannelMask;
    uint8 SubFormat[16];
};

struct riff_iterator
{
    uint8 *At;
    uint8 *Stop;
};

inline riff_iterator
ParseChunkAt(void *At, void *Stop)
{
    riff_iterator Iter;
    
    Iter.At = (uint8 *)At;
    Iter.Stop = (uint8 *)Stop;
    
    return(Iter);
}

inline riff_iterator
NextChunk(riff_iterator Iter)
{
    WAVE_chunk *Chunk = (WAVE_chunk *)Iter.At;
    uint32 Size = (Chunk->Size + 1) & ~1;
    Iter.At += sizeof(WAVE_chunk) + Size;
    
    return(Iter);
}

inline bool32
IsValid(riff_iterator Iter)
{    
    bool32 Result = (Iter.At < Iter.Stop);
    
    return(Result);
}

inline void *
GetChunkData(riff_iterator Iter)
{
    void *Result = (Iter.At + sizeof(WAVE_chunk));
    
    return(Result);
}

inline uint32
GetType(riff_iterator Iter)
{
    WAVE_chunk *Chunk = (WAVE_chunk *)Iter.At;
    uint32 Result = Chunk->ID;
    
    return(Result);
}

inline uint32
GetChunkDataSize(riff_iterator Iter)
{
    WAVE_chunk *Chunk = (WAVE_chunk *)Iter.At;
    uint32 Result = Chunk->Size;
    
    return(Result);
}

internal loaded_sound
LoadWAV(char *FileName)
{
    loaded_sound Result = {};
    
    entire_file ReadResult = ReadEntireFile(FileName);    
    if(ReadResult.ContentsSize != 0)
    {
        WAVE_header *Header = (WAVE_header *)ReadResult.Contents;
        Assert(Header->RIFFID == WAVE_ChunkID_RIFF);
        Assert(Header->WAVEID == WAVE_ChunkID_WAVE);
        
        uint32 ChannelCount = 0;
        uint32 SampleDataSize = 0;
        int16 *SampleData = 0;
        for(riff_iterator Iter = ParseChunkAt(Header + 1, (uint8 *)(Header + 1) + Header->Size - 4); IsValid(Iter); Iter = NextChunk(Iter))
        {
            switch(GetType(Iter))
            {
                case WAVE_ChunkID_fmt:
                {
                    WAVE_fmt *fmt = (WAVE_fmt *)GetChunkData(Iter);
                    Assert(fmt->wFormatTag == 1); // NOTE(casey): Only support PCM
                    Assert(fmt->nSamplesPerSec == 48000);
                    Assert(fmt->wBitsPerSample == 16);
                    Assert(fmt->nBlockAlign == (sizeof(int16)*fmt->nChannels));
                    ChannelCount = fmt->nChannels;
                } break;
                
                case WAVE_ChunkID_data:
                {
                    SampleData = (int16 *)GetChunkData(Iter);
                    SampleDataSize = GetChunkDataSize(Iter);
                } break;
            }
        }
        
        Assert(ChannelCount && SampleData);
        
        Result.ChannelCount = ChannelCount;
        Result.SampleCount = SampleDataSize / (ChannelCount*sizeof(int16));
        if(ChannelCount == 1)
        {
            Result.Samples[0] = SampleData;
            Result.Samples[1] = 0;
        }
        else if(ChannelCount == 2)
        {
            Result.Samples[0] = SampleData;
            Result.Samples[1] = SampleData + Result.SampleCount;
            
#if 0
            for(uint32 SampleIndex = 0;
                SampleIndex < Result.SampleCount;
                ++SampleIndex)
            {
                SampleData[2*SampleIndex + 0] = (int16)SampleIndex;
                SampleData[2*SampleIndex + 1] = (int16)SampleIndex;
            }
#endif
            
            for(uint32 SampleIndex = 0;
                SampleIndex < Result.SampleCount;
                ++SampleIndex)
            {
                int16 Source = SampleData[2*SampleIndex];
                SampleData[2*SampleIndex] = SampleData[SampleIndex];
                SampleData[SampleIndex] = Source;
            }
        }
        else
        {
            Assert(!"Invalid channel count in WAV file");
        }
        
        /*
        // TODO(casey): Load right channels!
        Result.ChannelCount = 1;
        if (SectionSampleCount)
        {
            Result.SampleCount = SectionSampleCount;
            for (uint32 ChannelIndex = 0; ChannelIndex < Result.ChannelCount; ++ChannelIndex)
            {
                Result.Samples[ChannelIndex] += SectionFirstSampleIndex;
            }
        }
        */
    }
    
    return(Result);
}