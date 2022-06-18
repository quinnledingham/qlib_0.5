#ifndef APPLICATION_H
#define APPLICATION_H

/*
  NOTE(casey): Services that the platform layer provides to the game
*/
#if SNAKE_INTERNAL
/* IMPORTANT(casey):

   These are NOT for doing anything in the shipping game - they are
   blocking and the write doesn't protect against lost data!
*/
struct debug_read_file_result
{
    uint32 ContentsSize;
    void *Contents;
};
internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
internal void DEBUGPlatformFreeFileMemory(void *Memory);
internal bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize, void *Memory);
#endif

inline uint32
SafeTruncateUInt64(uint64 Value)
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return(Result);
}

#define BITMAP_BYTES_PER_PIXEL 4

#if QLIB_WINDOW

struct Camera
{
    v3 Position;
    v3 Target;
    v3 Up;
    
    float inAspectRatio;
    Shader* shader;
    float FOV;
    float F;
    platform_window_dimension Dimension;
};

struct loaded_bitmap
{
    int32 Width;
    int32 Height;
    int32 Pitch;
    void *Memory;
    
    void *Free;
};

internal void
RenderBitmap(loaded_bitmap *Bitmap, real32 RealX, real32 RealY);

void UpdateRender(platform* p);

#else

void Update(platform* p);

#endif

#endif //APPLICATION_H
