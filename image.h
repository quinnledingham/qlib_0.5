#ifndef IMAGE_H
#define IMAGE_H

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"

#define BITMAP_BYTES_PER_PIXEL 4
struct loaded_bitmap
{
    void *Memory;
    
    int32 Width;
    int32 Height;
    int32 Pitch;
    int32 Channels;
    
    void *TextureHandle; // OpenGL
};

internal loaded_bitmap
LoadBitmap(const char *FileName)
{
    loaded_bitmap Bitmap = {};
    unsigned char* Data = stbi_load(FileName, &Bitmap.Width, &Bitmap.Height, &Bitmap.Channels, 0);
    Bitmap.Memory = (void*)Data;
    
    Assert(Data != 0);
    
    return Bitmap;
}

internal void
ResizeBitmap(loaded_bitmap *Bitmap, iv2 Dim)
{
    if (!stbir_resize_uint8((unsigned char*)Bitmap->Memory, Bitmap->Width, Bitmap->Height, 0,
                            (unsigned char*)Bitmap->Memory, Dim.x, Dim.y, 0, Bitmap->Channels))
        PrintqDebug(S() + "ResizeTexture(): stbir_resize_uint8 Error\n");
    
    Bitmap->Width = Dim.x;
    Bitmap->Height = Dim.y;
}

#endif //IMAGE_H