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
    int32 Pitch; //Channels
    void *Memory;
    
    void *Free;
};

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