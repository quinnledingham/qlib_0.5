//
// Asset File Builder
//

internal void
BuilderAssetTag(linked_list *Tags, asset_tag_id Tag1)
{
    debug_builder_asset_tag *Tag = (debug_builder_asset_tag*)qalloc(sizeof(debug_builder_asset_tag));
    Tag->ID = Tag1;
    LinkedListAddNode(Tags, (void*)Tag);
}

internal debug_builder_asset*
BuilderAddBitmap(debug_builder_assets *Assets, const char *FileName, asset_type_id Type)
{
    debug_builder_asset *BuilderAsset = Qalloc(debug_builder_asset);
    BuilderAsset->Type = Type;
    BuilderAsset->Asset.Bitmap = LoadBitmap2(FileName);
    BuilderAsset->Asset.MemoryType = asset_memory_type::Bitmap;
    LinkedListAddNode(&Assets->Assets, (void*)BuilderAsset);
    return BuilderAsset;
}

inline void BuilderAddBitmapTag(debug_builder_assets *Assets, 
                                const char *FileName, 
                                asset_type_id Type,
                                asset_tag_id Tag1)
{
    debug_builder_asset *BuilderAsset = BuilderAddBitmap(Assets, FileName, Type);
    BuilderAssetTag(&BuilderAsset->Tags, Tag1);
    Assets->TagCount++;
}

internal debug_builder_asset*
BuilderAddSound(debug_builder_assets *Assets, const char *FileName, asset_type_id Type)
{
    debug_builder_asset *BuilderAsset = Qalloc(debug_builder_asset);
    BuilderAsset->Type = Type;
    BuilderAsset->Asset.Sound = LoadWAV(FileName);
    BuilderAsset->Asset.MemoryType = asset_memory_type::Sound;
    LinkedListAddNode(&Assets->Assets, (void*)BuilderAsset);
    return BuilderAsset;
}

internal debug_builder_asset*
BuilderAddFont(debug_builder_assets *Assets, const char *FileName, asset_type_id Type)
{
    debug_builder_asset *BuilderAsset = Qalloc(debug_builder_asset);
    BuilderAsset->Type = Type;
    BuilderAsset->Asset.Font = LoadFont2(FileName);
    BuilderAsset->Asset.MemoryType = asset_memory_type::Font;
    LinkedListAddNode(&Assets->Assets, (void*)BuilderAsset);
    return BuilderAsset;
}

internal void
BuilderMakeFile(debug_builder_assets *Assets)
{
    assets FinalAssets = {};
    linked_list PrintList = {};
    
    // Order them by asset types and determine ranges of types
    for (int i = 0; i < Asset_Count; i++) {
        LinkedListIReset(&Assets->Assets);
        for (u32 j = 0; j < Assets->Assets.NodeCount; j++) {
            debug_builder_asset *BuilderAsset = (debug_builder_asset*)LinkedListIGetNext(&Assets->Assets);
            
            if ((int)BuilderAsset->Type == i) {
                if (FinalAssets.AssetTypes[i].FirstAssetIndex == 0) {
                    FinalAssets.AssetTypes[i].FirstAssetIndex = PrintList.NodeCount;
                    FinalAssets.AssetTypes[i].OnePastLastAssetIndex = PrintList.NodeCount + 1;
                }
                else
                    FinalAssets.AssetTypes[i].OnePastLastAssetIndex++;
                
                LinkedListAddNode(&PrintList, (void*)BuilderAsset);
            }
        }
    }
    
    FinalAssets.Assets = (asset*)qalloc(sizeof(asset) * Assets->Assets.NodeCount);
    FinalAssets.Tags = (asset_tag*)qalloc(sizeof(asset_tag) * Assets->TagCount);
    
    // Put all of the assets and tags into chunks of memory
    LinkedListIReset(&PrintList);
    for (u32 i = 0; i < PrintList.NodeCount; i++) {
        debug_builder_asset *BuilderAsset = (debug_builder_asset*)LinkedListIGetNext(&PrintList);
        memcpy(&FinalAssets.Assets[i], &BuilderAsset->Asset, sizeof(asset));
        FinalAssets.AssetCount++;
        
        LinkedListIReset(&BuilderAsset->Tags);
        for (u32 j = 0; j < BuilderAsset->Tags.NodeCount; j++) {
            debug_builder_asset_tag *BuilderTag = (debug_builder_asset_tag*)LinkedListIGetNext(&BuilderAsset->Tags);
            asset_tag *NextTag = &FinalAssets.Tags[FinalAssets.TagCount];
            NextTag->ID = BuilderTag->ID;
            NextTag->Value = i;
            FinalAssets.TagCount++;
        }
    }
    
    FILE *AssetFile = fopen("assets.ethan", "wb");
    
    fwrite(&FinalAssets.TagCount, sizeof(u32), 1, AssetFile);
    fwrite(FinalAssets.Tags, sizeof(asset_tag) * FinalAssets.TagCount, 1, AssetFile);
    fwrite(&FinalAssets.AssetCount, sizeof(u32), 1, AssetFile);
    fwrite(FinalAssets.Assets, sizeof(asset) * FinalAssets.AssetCount, 1, AssetFile);
    fwrite(FinalAssets.AssetTypes, sizeof(asset_type) * Asset_Count, 1, AssetFile);
    
    // Large chunk of memory for each asset
    for (u32 i = 0; i < FinalAssets.AssetCount; i++) {
        if (FinalAssets.Assets[i].MemoryType == asset_memory_type::Bitmap)
        {
            loaded_bitmap *Bitmap = (loaded_bitmap*)&FinalAssets.Assets[i].Bitmap;
            fwrite(Bitmap->Memory, Bitmap->Channels, Bitmap->Width * Bitmap->Height, AssetFile);
        }
        else if (FinalAssets.Assets[i].MemoryType == asset_memory_type::Sound)
        {
            loaded_sound *Sound = (loaded_sound*)&FinalAssets.Assets[i].Sound;
            fwrite(Sound->Samples[0], AUDIO_S16_BYTES, Sound->SampleCount, AssetFile);
        }
        else if (FinalAssets.Assets[i].MemoryType == asset_memory_type::Font)
        {
            loaded_font *Font = (loaded_font*)&FinalAssets.Assets[i].Font;
            fwrite(Font->TTFFile.Contents, 1, Font->TTFFile.ContentsSize, AssetFile);
        }
    }
    fclose(AssetFile);
}

internal void
BuilderLoadFile(assets *Assets)
{
    FILE *AssetFile = fopen("assets.ethan", "rb");
    
    fread(&Assets->TagCount, sizeof(u32), 1, AssetFile);
    Assets->Tags = (asset_tag*)qalloc(sizeof(asset_tag) * Assets->TagCount);
    fread(Assets->Tags, sizeof(asset_tag) * Assets->TagCount, 1, AssetFile);
    
    fread(&Assets->AssetCount, sizeof(u32), 1, AssetFile);
    Assets->Assets = (asset*)qalloc(sizeof(asset) * Assets->AssetCount);
    fread(Assets->Assets, sizeof(asset) * Assets->AssetCount, 1, AssetFile);
    
    fread(Assets->AssetTypes, sizeof(asset_type) * Asset_Count, 1, AssetFile);
    
    for (u32 i = 0; i < Assets->AssetCount; i++) {
        if (Assets->Assets[i].MemoryType == asset_memory_type::Bitmap) {
            loaded_bitmap *Bitmap = (loaded_bitmap*)&Assets->Assets[i].Bitmap;
            Bitmap->Memory = qalloc(Bitmap->Width * Bitmap->Height * Bitmap->Channels);
            fread(Bitmap->Memory, Bitmap->Channels, Bitmap->Width * Bitmap->Height, AssetFile);
            TextureInit(Bitmap);
        }
        else if (Assets->Assets[i].MemoryType == asset_memory_type::Sound) {
            loaded_sound *Sound = (loaded_sound*)&Assets->Assets[i].Sound;
            Sound->Samples[0] = (int16*)qalloc(Sound->SampleCount * AUDIO_S16_BYTES);
            Sound->Samples[1] = Sound->Samples[0] + Sound->SampleCount;
            fread(Sound->Samples[0], AUDIO_S16_BYTES, Sound->SampleCount, AssetFile);
        }
        else if (Assets->Assets[i].MemoryType == asset_memory_type::Font)
        {
            loaded_font *Font = (loaded_font*)&Assets->Assets[i].Font;
            Font->TTFFile.Contents = qalloc(Font->TTFFile.ContentsSize);
            fread(Font->TTFFile.Contents, 1, Font->TTFFile.ContentsSize, AssetFile);
            stbtt_InitFont(&Font->Info, (u8 *)Font->TTFFile.Contents, stbtt_GetFontOffsetForIndex((u8 *)Font->TTFFile.Contents, 0));
        }
    }
    
    fclose(AssetFile);
}