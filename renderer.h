#ifndef RENDERER_H
#define RENDERER_H

#include "data_structures.h"

template<typename T>
struct Attribute
{
    unsigned int mHandle;
    unsigned int mCount;
    
    void Init();
    void Destroy();
    unsigned int Count();
    unsigned int GetHandle();
    void Set(T* inputArray, unsigned int arrayLength);
    void Set(DynArray<T> &input);
    void SetAttribPointer(unsigned int s);
    void BindTo(unsigned int slot);
    void UnBindFrom(unsigned int slot);
};

template<typename T>
struct Uniform
{
    void Init();
    void Init(const Uniform&);
    Uniform& operator=(const Uniform&);
    void Destroy();
    
    static void Set(unsigned int slot, const T& value);
    static void Set(unsigned int slot, T* arr, unsigned int len);
    static void Set(unsigned int slot, DynArray<T>& arr);
};

struct Shader
{
    u32 mHandle;
    
    Map mAttributes;
    Map mUniforms;
    
    void Init();
    void Init(const char* vertex, const char* fragment);
    void Destroy();
    void Bind();
    void UnBind();
    unsigned int GetHandle();
    unsigned int GetAttribute(const char* name);
    unsigned int GetUniform(const char* name);
};

struct IndexBuffer
{
    unsigned int mHandle;
    unsigned int mCount;
    
    void Init();
    void Init(const IndexBuffer& other);
    IndexBuffer& operator=(const IndexBuffer& other);
    void Destroy();
    
    void Set(unsigned int* rr, unsigned int len);
    void Set(DynArray<unsigned int> &input);
    
    unsigned int Count();
    unsigned int GetHandle();
};

struct Texture
{
    const char* ID;
    unsigned int mHandle;
    
    unsigned int mWidth;
    unsigned int mHeight;
    unsigned int mChannels;
    unsigned char* data;
    
    Image og;
    
    void Init();
    void Init(unsigned char*);
    void Init(Image* image);
    void Init(const char* path);
    void Init(const Texture& other);
    //Texture& operator=(const Texture& other);
    void Destroy();
    
    void Set(unsigned int uniform, unsigned int texIndex);
    void UnSet(unsigned int textureIndex);
    unsigned int GetHandle();
};

enum struct
DrawMode
{
    Points,
    LineStrip,
    LineLoop,
    Lines,
    Triangles,
    TriangleStrip,
    TriangleFan
};

enum struct
BlendMode
{
    gl_one,
    gl_src_alpha,
};

void glDraw(unsigned int vertexCount, DrawMode mode);
void glDraw(IndexBuffer& inIndexBuffer, DrawMode mode);
void glDrawInstanced(unsigned int vertexCount, DrawMode node, unsigned int numInstances);
void glDrawInstanced(IndexBuffer& inIndexBuffer, DrawMode mode, unsigned int instanceCount);

void DrawRect(int x, int y, int width, int height, uint32 color);
void DrawRect(v3 Coords, v2 Size, uint32 color, real32 Rotation);
void DrawRect(v3 Coords, v2 Size, Texture *Tex, real32 Rotation, BlendMode Mode);

enum struct
PieceType 
{
    ColorRect,
    TextureRect,
};

struct Piece
{
    v3 Coords;
    v2 Dim;
    real32 Rotation;
    BlendMode BMode;
    
    PieceType Type;
    
    uint32 Color;
    Texture *Tex;
    
    inline Piece() {}
    inline Piece(v3 _Coords, v2 _Dim, Texture *_Tex, real32 _Rotation, BlendMode _BMode) :
    Coords(_Coords), Dim(_Dim), Tex(_Tex), Rotation(_Rotation),  
    BMode(_BMode) {Type = PieceType::TextureRect;}
    
    inline Piece(v3 _Coords, v2 _Dim, uint32 _Color, real32 _Rotation) :
    Coords(_Coords), Dim(_Dim), Color(_Color), Rotation(_Rotation) {Type = PieceType::ColorRect;}
};

struct PieceGroup
{
    Piece Buffer[1000];
    int Size = 0;
    int MaxSize = 1000;
    
    void Push(Piece NewRect)
    {
        Buffer[Size] = NewRect;
        Size++;
    }
    
    void Clear()
    {
        memset(Buffer, 0, MaxSize * sizeof(Piece));
        Size = 0;
    }
    
    Piece* operator[](int i)
    {
        return &Buffer[i];
    }
};

#endif //RENDERER_H
