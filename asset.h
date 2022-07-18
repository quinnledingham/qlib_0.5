#ifndef DEBUG_ASSET_H
#define DEBUG_ASSET_H

#define str(x) #x
#define xstr(x) str(x)
#define pairintstring(x) {x, xstr(x)}

struct asset_vector
{
    real32 E[Tag_Count];
};

struct asset_tag
{
    u32 ID;
    u32 Value;
};

enum struct asset_memory_type
{
    Bitmap,
    Font,
    Sound,
    
    Count
};

struct asset
{
    asset_memory_type MemoryType;
    union
    {
        loaded_bitmap Bitmap;
        loaded_font Font;
        loaded_sound Sound;
    };
};

struct asset_type
{
    uint32 FirstAssetIndex;
    uint32 OnePastLastAssetIndex;
};

struct assets
{
    u32 TagCount;
    asset_tag *Tags;
    
    u32 AssetCount;
    asset *Assets;
    
    asset_type AssetTypes[Asset_Count];
};

// Get ids
// bitmap_id
internal bitmap_id
GetBestMatchBitmap(assets *Assets, asset_type_id TypeID, asset_vector *MatchVector, asset_vector *WeightVector)
{
    u32 Start = Assets->AssetTypes[TypeID].FirstAssetIndex;
    u32 BestIndex = Start;
    for (u32 i = 0; i < Assets->TagCount; i++)
    {
        if (Assets->AssetTypes[TypeID].FirstAssetIndex <= Assets->Tags[i].Value &&
            Assets->Tags[i].Value < Assets->AssetTypes[TypeID].OnePastLastAssetIndex) {
            //if (MatchVector->E[i] > 0.0f)
            BestIndex = Assets->Tags[i].Value;
        }
        
    }
    return bitmap_id(BestIndex);
}

inline bitmap_id
GetIndexBitmap(assets *Assets, asset_type_id TypeID, u32 Index)
{
    return bitmap_id(Assets->AssetTypes[TypeID].FirstAssetIndex + Index);
}

inline bitmap_id
GetFirstBitmap(assets *Assets, asset_type_id TypeID)
{
    //asset *Asset = Assets->Assets + Assets->AssetTypes[TypeID].FirstAssetIndex;
    return bitmap_id(Assets->AssetTypes[TypeID].FirstAssetIndex);
}

// sound_id
inline sound_id
GetFirstSound(assets *Assets, asset_type_id TypeID)
{
    return sound_id(Assets->AssetTypes[TypeID].FirstAssetIndex);
}

// font_id
inline font_id 
GetFirstFont(assets *Assets, asset_type_id TypeID)
{
    return font_id(Assets->AssetTypes[TypeID].FirstAssetIndex);
}

// Get Types

inline loaded_bitmap* GetBitmap(assets *Assets, bitmap_id id)
{
    asset *Asset = Assets->Assets + id.id;
    return (loaded_bitmap*)&Asset->Bitmap;
}
inline loaded_sound* GetSound(assets *Assets, sound_id id)
{
    asset *Asset = Assets->Assets + id.id;
    return (loaded_sound*)&Asset->Sound;
}
inline loaded_font* GetFont(assets *Assets, font_id id)
{
    asset *Asset = Assets->Assets + id.id;
    return (loaded_font*)&Asset->Font;
}

//
// Asset File Builder 
//

struct debug_builder_asset_tag
{
    u32 ID;
};

struct debug_builder_asset
{
    asset Asset;
    asset_type_id Type;
    linked_list Tags;
};

struct debug_builder_assets
{
    linked_list Assets;
    u32 TagCount;
};

#endif //DEBUG_ASSET_H
