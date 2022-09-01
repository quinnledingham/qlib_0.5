internal void
PlaySound(audio_state *AudioState, sound_id LoadedSound)
{
    if (AudioState->NumOfSounds > ArrayCount(AudioState->PlayingSounds)) {
        SDL_Log("Error: Too Many Sounds");
        return;
    }
    
    AudioState->NumOfSounds++;
    
    playing_sound *Sound = 0;
    for (uint32 i = 0; i < AudioState->NumOfSounds; i++) {
        playing_sound *PlayingSound = &AudioState->PlayingSounds[i];
        if (!PlayingSound->Playing) {
            Sound = PlayingSound;
            break;
        }
    }
    
    Assert(Sound != 0);
    Sound->SamplesPlayed = 0;
    Sound->Playing = true;
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
    // Can't hold all the samples if the fps is too low
    uint32 ChannelSize = SoundBuffer->MaxSampleCount * AUDIO_S16_BYTES;
    
    if (!AudioState->Initialized) {
        //SDL_Log("AudioState Channel Size: %d\n", ChannelSize);
        AudioState->Channel0 = (int16*)qalloc(ChannelSize);
        AudioState->Channel1 = (int16*)qalloc(ChannelSize);
        AudioState->Initialized = true;
    }
    
    // Don't write sound if the channels can't hold all of the samples
    if (SoundBuffer->SampleCount > SoundBuffer->MaxSampleCount || SoundBuffer->SampleCount == 0)
        return;
    
    // Clearing mixing channels
    {
        int16 *Dest0 = AudioState->Channel0;
        int16 *Dest1 = AudioState->Channel1;
        for (uint32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
        {
            *Dest0++ = 0;
            *Dest1++ = 0;
        }
    }
    
    v2 Volume = v2(0.5f, 0.5f);
    real32 SecondsPerSample = 1.0f / (real32)SoundBuffer->SamplesPerSecond;
    v2 dVolume = SecondsPerSample * v2(0.0f, 0.0f);
    
    // Filling in mixing channels
    for (uint32 i = 0; i < AudioState->NumOfSounds; i++) {
        int16 *Dest0 = AudioState->Channel0;
        int16 *Dest1 = AudioState->Channel1;
        
        playing_sound *PlayingSound = &AudioState->PlayingSounds[i];
        if (PlayingSound != 0 && PlayingSound->Playing)
        {
            loaded_sound *LoadedSound = GetSound((assets*)AudioState->Assets, PlayingSound->LoadedSound);
            u32 SamplesRemainingInSound = LoadedSound->SampleCount - PlayingSound->SamplesPlayed;
            if (SamplesRemainingInSound > (uint32)SoundBuffer->SampleCount)
                SamplesRemainingInSound = (uint32)SoundBuffer->SampleCount;
            
            // TODO(casey): Handle stereo!
            for (uint32 SampleIndex = 0; SampleIndex < (uint32)SoundBuffer->SampleCount; ++SampleIndex)
            {
                uint32 TestSoundSampleIndex = (PlayingSound->SamplesPlayed + SampleIndex) % LoadedSound->SampleCount;
                int16 SampleValue = LoadedSound->Samples[0][TestSoundSampleIndex];
                *Dest0++ += (int16)(AudioState->MasterVolume.v[0] * Volume.v[0] * SampleValue);
                *Dest1++ += (int16)(AudioState->MasterVolume.v[1] * Volume.v[1] * SampleValue);
                
                Volume = Volume + dVolume;
            }
            
            //SDL_Log("Sounds %d, %d/%d\n", AudioState->NumOfSounds, AudioState->PlayingSounds[0].SamplesPlayed, LoadedSound->SampleCount);
            PlayingSound->SamplesPlayed += (uint32)SoundBuffer->SampleCount;
            
            if ((uint32)PlayingSound->SamplesPlayed >= LoadedSound->SampleCount)
                PlayingSound->Playing = false;
        }
    }
    
    // Filling in buffer
    {
        int16 *Source0 = AudioState->Channel0;
        int16 *Source1 = AudioState->Channel1;
        
        int16 *SampleOut = SoundBuffer->Samples;
        for (uint32 SampleIndex = 0; SampleIndex < (uint32)SoundBuffer->SampleCount; ++SampleIndex)
        {
            // TODO(casey): Once this is in SIMD, clamp!
            
            *SampleOut++ = (int16)(*Source0++ + 0.5f);
            *SampleOut++ = (int16)(*Source1++ + 0.5f);
        }
    }
    
    // Adjusting NumOfSounds
    if (AudioState->NumOfSounds > 0) {
        playing_sound *Sound = &AudioState->PlayingSounds[AudioState->NumOfSounds - 1];
        if (!Sound->Playing)
            AudioState->NumOfSounds--;
    }
}

internal void
InitializeAudioState(audio_state *AudioState)
{
    AudioState->MasterVolume = v2(1.0f, 1.0f);
}

//
// Loading WAV Files from Handemade Hero
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
LoadWAV2(const char *FileName)
{
    SDL_AudioSpec Wav_Spec;
    uint32 Wav_Length;
    uint8 *Wav_Buffer;
    
    SDL_LoadWAV(FileName, &Wav_Spec, &Wav_Buffer, &Wav_Length);
    
    loaded_sound Result = {};
    Result.SampleCount = Wav_Length / 4;
    Result.Samples[0] = (int16*)Wav_Buffer;
    Result.Samples[1] = (int16*)Wav_Buffer;
    Result.ChannelCount = Wav_Spec.channels;
    
    return(Result);
}

internal loaded_sound
LoadWAV(const char *FileName)
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
                    Assert(fmt->nChannels == 2); // Not here in Casey's. He must support 1 channel.
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
        u32 SampleCount = SampleDataSize / (ChannelCount*sizeof(int16));
        if(ChannelCount == 1)
        {
            Result.Samples[0] = SampleData;
            Result.Samples[1] = 0;
        }
        else if(ChannelCount == 2)
        {
            Result.Samples[0] = SampleData;
            Result.Samples[1] = SampleData + Result.SampleCount;
            
            for(uint32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
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
        
        // TODO(casey): Load right channels!
        b32 AtEnd = true;
        Result.ChannelCount = 1;
        /*
        if(SectionSampleCount)
        {
            Assert((SectionFirstSampleIndex + SectionSampleCount) <= SampleCount);
            AtEnd = ((SectionFirstSampleIndex + SectionSampleCount) == SampleCount);
            SampleCount = SectionSampleCount;
            for(uint32 ChannelIndex = 0; ChannelIndex < Result.ChannelCount; ++ChannelIndex)
            {
                Result.Samples[ChannelIndex] += SectionFirstSampleIndex;
            }
        }
        */
        
        if(AtEnd) {
            for(uint32 ChannelIndex = 0; ChannelIndex < Result.ChannelCount; ++ChannelIndex) {
                for(u32 SampleIndex = SampleCount; SampleIndex < (SampleCount + 8); ++SampleIndex) {
                    Result.Samples[ChannelIndex][SampleIndex] = 0;
                }
            }
        }
        
        Result.SampleCount = SampleCount;
    }
    
    return(Result);
}