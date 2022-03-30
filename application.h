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

void Update(platform* p);

void Render(float inAspectRatio);

#endif //APPLICATION_H
