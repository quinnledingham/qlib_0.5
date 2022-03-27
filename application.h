#ifndef APPLICATION_H
#define APPLICATION_H

#if RENDERER_SLOW
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#include "String.h"
#include "types.h"
#include "renderer.h"


struct button_state
{
    bool32 NewEndedDown;
    bool32 EndedDown;
};

struct controller_input
{
    bool32 IsConnected;
    bool32 IsAnalog;
    real32 StickAverageX;
    real32 StickAverageY;
    
    union
    {
        button_state Buttons[12];
        struct
        {
            button_state MoveUp;
            button_state MoveDown;
            button_state MoveLeft;
            button_state MoveRight;
        };
    };
};

struct
platform_input
{
    float dt;
    controller_input Controllers[5];
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

struct
Camera
{
    v3 Position;
    v3 Target;
    v3 Up;
    
    float inAspectRatio;
    Shader* shader;
};

struct
Rect
{
    real32 x;
    real32 y;
    real32 width;
    real32 height;
    
    Shader* shader;
    unsigned int quadVAO;
    
    Attribute<v3> VertexPositions;
    IndexBuffer rIndexBuffer;
    Attribute<v3> VertexNormals = {};
    Attribute<v2> VertexTexCoords = {};
    
    void Init(Shader* s);
    void Destroy();
    void Draw(Texture &texture, v2 position, v2 size, float rotate,v3 color);
};



#endif //APPLICATION_H
