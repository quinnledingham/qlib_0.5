#ifndef ASSET_H
#define ASSET_H

#ifndef BITMAP_COUNT
#pragma message ("Define BITMAP_COUNT before asset.h")
#endif
#ifndef FONT_COUNT
#pragma message ("Define FONT_COUNT before asset.h")
#endif
#ifndef SOUND_COUNT
#pragma message ("Define SOUND_COUNT before asset.h")
#endif

#define bitmap_count BITMAP_COUNT
#define font_count FONT_COUNT
#define sound_count SOUND_COUNT

#define str(x) #x
#define xstr(x) str(x)
#define pairintstring(x) {x, xstr(x)}

#define ASSET_STRING_SIZE 45
struct asset_tag
{
    u32 ID;
    char Value[ASSET_STRING_SIZE];
};

struct asset
{
    asset_tag Tag;
    char FileName[ASSET_STRING_SIZE];
    void *Data;
};

inline void AssetInitTag(asset *Asset, const char *FileName, u32 ID, const char *Value)
{
    asset *Temp = &Asset[ID];
    Temp->Tag.ID = ID;
    for (int i = 0; i < GetLength(FileName); i++)
        Temp->FileName[i] = FileName[i];
    for (int i = 0; i < GetLength(Value); i++)
        Temp->Tag.Value[i] = Value[i];
}
#define AssetInitTag(a, f, i) (AssetInitTag(a, f, i, xstr(i)))

struct assets
{
    asset Sounds[SOUND_COUNT]; // loaded_sound
    asset Bitmaps[BITMAP_COUNT]; // resizable_bitmap
    asset Fonts[FONT_COUNT]; // font
};

internal PLATFORM_WORK_QUEUE_CALLBACK(LoadBitmapAsset)
{
    asset *Asset = (asset*)Data;
    Asset->Data = LoadResizableBitmap(Asset->FileName);
}
internal PLATFORM_WORK_QUEUE_CALLBACK(LoadFontAsset)
{
    asset *Asset = (asset*)Data;
    Asset->Data = LoadFont(Asset->FileName);
}
internal PLATFORM_WORK_QUEUE_CALLBACK(LoadSoundAsset)
{
    asset *Asset = (asset*)Data;
    loaded_sound Temp = LoadWAV(Asset->FileName);
    Asset->Data = qalloc((void*)&Temp, sizeof(loaded_sound));
}

inline resizable_bitmap* GetResizableBitmap(assets *Assets, int ID)
{
    return (resizable_bitmap*)Assets->Bitmaps[ID].Data; 
}
inline resizable_bitmap *GetResizableBitmap(assets *Assets, const char* ID)
{
    for (int i = 0; i < bitmap_count; i++) {
        if (Equal(Assets->Bitmaps[i].Tag.Value, ID)) {
            return (resizable_bitmap*)Assets->Bitmaps[i].Data;
        }
    }
    return 0;
}

inline font *GetFont(assets *Assets, int ID)
{
    return (font*)Assets->Fonts[ID].Data;
}
inline font *GetFont(assets *Assets, const char* ID)
{
    for (int i = 0; i < font_count; i++) {
        if (Equal(Assets->Fonts[i].Tag.Value, ID)) {
            return (font*)Assets->Fonts[i].Data;
        }
    }
    return 0;
}

inline loaded_sound *GetSound(assets *Assets, int ID)
{
    return (loaded_sound*)Assets->Sounds[ID].Data;
}

internal int
GetBitmapID(assets *Assets, const char *ID)
{
    for (int i = 0; i < bitmap_count; i++) {
        if (Equal(Assets->Bitmaps[i].Tag.Value, ID)) {
            return Assets->Bitmaps[i].Tag.ID;
        }
    }
    return 0;
}

#endif //ASSET_H
