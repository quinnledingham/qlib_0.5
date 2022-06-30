#ifndef ASSET_H
#define ASSET_H

#define str(x) #x
#define xstr(x) str(x)
#define pairintstring(x) {x, xstr(x)}

enum game_asset_id
{
    GAI_Background,
    GAI_Grass,
    GAI_Rocks,
    GAI_Grid,
    GAI_CoffeeTex,
    GAI_Head,
    GAI_Straight,
    GAI_Corner,
    GAI_CoffeeStreak,
    GAI_HeadOutline,
    GAI_StraightOutline,
    GAI_CornerOutline,
    GAI_Tail,
    GAI_Tongue,
    GAI_Miz,
    GAI_Join,
    GAI_JoinAlt,
    GAI_JoinHover,
    GAI_JoinAltHover,
    GAI_MainMenuBack,
    
    GAI_Count
};
enum game_asset_font_id
{
    GAFI_Rubik,
    
    GAFI_Count
};
enum game_asset_sound_id
{
    GASI_Bloop,
    GASI_Violence,
    
    GASI_Count
};

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
    game_asset Sounds[GASI_Count];
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
inline loaded_sound *GetSound(game_assets *Assets, int ID)
{
    loaded_sound *Result = (loaded_sound*)Assets->Sounds[ID].Data;
    return Result;
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
internal PLATFORM_WORK_QUEUE_CALLBACK(LoadSoundAsset)
{
    game_asset *GA = (game_asset*)Data;
    loaded_sound Temp = DEBUGLoadWAV(GA->FileName);
    GA->Data = qalloc((void*)&Temp, sizeof(loaded_sound));
}

#endif //ASSET_H