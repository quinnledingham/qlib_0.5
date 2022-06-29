#ifndef ASSET_H
#define ASSET_H

#define str(x) #x
#define xstr(x) str(x)
#define pairintstring(x) {x, xstr(x)}

struct game_asset_tag
{
    int ID;
    char Value[45];
};

struct game_asset
{
    game_asset_tag Tag;
    char FileName[45];
    void *Data;
};

struct game_assets
{
    game_asset Textures[GAI_Count];
    game_asset Spots[4];
    game_asset Fonts[GAFI_Count];
};
inline Texture *GetTexture(game_assets *Assets, int ID)
{
    Texture *Result = (Texture*)Assets->Textures[ID].Data;
    return Result;
}
inline Texture *GetTexture(game_assets *Assets, const char* ID)
{
    for (int i = 0; i < GAI_Count; i++) {
        if (Equal(Assets->Textures[i].Tag.Value, ID)) {
            return (Texture*)Assets->Textures[i].Data;
        }
    }
    return 0;
}
inline font *GetFont(game_assets *Assets, int ID)
{
    font *Result = (font*)Assets->Fonts[ID].Data;
    return Result;
}
inline font *GetFont(game_assets *Assets, const char* ID)
{
    for (int i = 0; i < GAI_Count; i++) {
        if (Equal(Assets->Fonts[i].Tag.Value, ID)) {
            return (font*)Assets->Fonts[i].Data;
        }
    }
    return 0;
}

inline void GameAssetLoadTag(game_asset *Assets, const char *FileName, int ID, const char *Value)
{
    game_asset *Temp = &Assets[ID];
    for (int i = 0; i < GetLength(FileName); i++) {
        Temp->FileName[i] = FileName[i];
    }
    //Temp->FileName = FileName;
    Temp->Tag.ID = ID;
    for (int i = 0; i < GetLength(Value); i++) {
        Temp->Tag.Value[i] = Value[i];
    }
}
#define GameAssetLoadTag(a, f, i) (GameAssetLoadTag(a, f, i, xstr(i)))

internal PLATFORM_WORK_QUEUE_CALLBACK(LoadTextureAsset)
{
    game_asset *GA = (game_asset*)Data;
    GA->Data = LoadTexture(GA->FileName);
}
internal PLATFORM_WORK_QUEUE_CALLBACK(LoadFontAsset)
{
    game_asset *GA = (game_asset*)Data;
    GA->Data = LoadFont(GA->FileName);
}

struct loaded_sound
{
    uint32 SampleCount;
    uint32 ChannelCount;
    int16 *Samples[2];
};

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
DEBUGLoadWAV(char *FileName)
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
        for(riff_iterator Iter = ParseChunkAt(Header + 1, (uint8 *)(Header + 1) + Header->Size - 4);
            IsValid(Iter);
            Iter = NextChunk(Iter))
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
        
        // TODO(casey): Load right channels!
        Result.ChannelCount = 1;
    }
    
    return(Result);
}

#endif //ASSET_H