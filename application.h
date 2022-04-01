#ifndef APPLICATION_H
#define APPLICATION_H

#if RENDERER_SLOW
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

struct platform_button_state
{
    bool32 NewEndedDown;
    bool32 EndedDown;
};

struct platform_controller_input
{
    bool32 IsConnected;
    bool32 IsAnalog;
    real32 StickAverageX;
    real32 StickAverageY;
    
    union
    {
        platform_button_state Buttons[12];
        struct
        {
            platform_button_state MoveUp;
            platform_button_state MoveDown;
            platform_button_state MoveLeft;
            platform_button_state MoveRight;
        };
    };
};

struct
platform_input
{
    float dt;
    platform_controller_input Controllers[5];
};

struct
platform_memory
{
    uint64 PermanentStorageSize;
    void *PermanentStorage; // REQUIRED to be cleared to zero at startup
    
    uint64 TransientStorageSize;
    void *TransientStorage; // REQUIRED to be cleared to zero at startup
};

struct
platform
{
    bool32 Initialized;
    
    platform_input Input;
    platform_memory Memory;
};

#define OUTPUTBUFFER_SIZE 1000

// PrintqDebug
struct platform_debug_buffer
{
    int Size = OUTPUTBUFFER_SIZE;
    char Data[OUTPUTBUFFER_SIZE];
    char* Next;
};
global_variable platform_debug_buffer GlobalDebugBuffer = {};

// Start Possible Win32
struct platform_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};
global_variable platform_offscreen_buffer GlobalBackbuffer;

struct platform_window_dimension
{
    int Width;
    int Height;
};
// End Win32

struct
Camera
{
    v3 Position;
    v3 Target;
    v3 Up;
    
    float inAspectRatio;
    Shader* shader;
    float FOV;
    float F;
};

struct
Rect
{
    real32 x;
    
    real32 y;
    real32 width;
    real32 height;
    
    Shader* shader;
    
    Attribute<v3> VertexPositions;
    IndexBuffer rIndexBuffer;
    Attribute<v3> VertexNormals = {};
    Attribute<v2> VertexTexCoords = {};
    
    void Init(Shader* s);
    void Destroy();
    void Draw(Texture &texture, v2 position, v2 size, float rotate,v3 color);
};

void Update(platform* p);

void Render(float inAspectRatio);

#endif //APPLICATION_H
