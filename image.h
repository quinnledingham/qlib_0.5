#ifndef IMAGE_H
#define IMAGE_H

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "qlib/stb/stb_image.h"
#include "qlib/stb/stb_image_resize.h"

struct Image
{
    int x;
    int y;
    int n;
    unsigned char* data;
};

#define BITMAP_BYTES_PER_PIXEL 4
struct loaded_bitmap
{
    int32 Width;
    int32 Height;
    int32 Pitch;
    int32 Channels;
    void *Memory;
    
    void *Free;
};

struct resizable_bitmap
{
    loaded_bitmap *Original;
    loaded_bitmap *Resized;
};

internal loaded_bitmap*
LoadBitmap(const char *FileName)
{
    loaded_bitmap Bitmap = {};
    unsigned char* Data = stbi_load(FileName, &Bitmap.Width, &Bitmap.Height, &Bitmap.Channels, 0);
    Bitmap.Memory = (void*)Data;
    
    Assert(Bitmap.Width != 0);
    
    return (loaded_bitmap*)qalloc(&Bitmap, sizeof(loaded_bitmap));
}

internal resizable_bitmap*
LoadResizableBitmap(const char *FileName)
{
    resizable_bitmap Bitmap = {};
    Bitmap.Original = LoadBitmap(FileName);
    
    Assert(Bitmap.Original->Width != 0);
    
    return (resizable_bitmap*)qalloc(&Bitmap, sizeof(resizable_bitmap));
}

internal void
ResizeBitmap(resizable_bitmap *Bitmap, iv2 Dim)
{
    loaded_bitmap *Original = Bitmap->Original;
    loaded_bitmap *Resized = Bitmap->Resized;
    
    if (Resized == 0) {
        Bitmap->Resized = (loaded_bitmap*)qalloc(Original, sizeof(loaded_bitmap));
        Resized = Bitmap->Resized;
    }
    
    if (!stbir_resize_uint8((unsigned char*)Original->Memory, Original->Width, Original->Height, 0,
                            (unsigned char*)Resized->Memory, Dim.x, Dim.y, 0, Resized->Channels))
        PrintqDebug(S() + "ResizeTexture(): stbir_resize_uint8 Error\n");
    Resized->Width = Dim.x;
    Resized->Height = Dim.y;
}

internal Image
LoadImage(const char* FileName)
{
    Image NewImage;
    NewImage.data = stbi_load(FileName, &NewImage.x, &NewImage.y, &NewImage.n, 4);
    NewImage.n = 4;
    Assert(NewImage.data != 0); // Image loaded
    
    return NewImage;
}

internal Image
ResizeImage(Image *ToResize, int Width, int Height)
{
    Image ResizedImage = {};
    ResizedImage.data = (unsigned char*)malloc(Width * Height * ToResize->n);
    stbir_resize_uint8(ToResize->data, ToResize->x, ToResize->y, 0, ResizedImage.data, Width, Height, 0, ToResize->n);
    
    ResizedImage.x = Width;
    ResizedImage.y = Height;
    ResizedImage.n = ToResize->n;
    
#if SAVE_IMAGES
    stbi_image_free(ToResize->data);
#else
    
#endif
    
    return ResizedImage;
}

#endif //IMAGE_H