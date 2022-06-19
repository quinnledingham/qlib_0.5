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

struct ImageHeader
{
    unsigned int x;
    unsigned int y;
    unsigned int n;
};

#define BITMAP_BYTES_PER_PIXEL 4
struct loaded_bitmap
{
    int32 Width;
    int32 Height;
    int32 Pitch;
    void *Memory;
    
    void *Free;
};

#endif //IMAGE_H