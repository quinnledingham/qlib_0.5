#ifndef RENDERER_H
#define RENDERER_H

#ifndef TYPES_H
#pragma message ("renderer.h requires types.h")
#endif
#ifndef DATA_STRUCTURES_H
#pragma message ("renderer.h requires data_structures.h")
#endif
#ifndef IMAGE_H
#pragma message ("renderer.h requires image.h")
#endif

struct render_attribute
{
    u32 Handle;
    u32 Count;
};

struct render_uniform {};

struct render_shader
{
    u32 Handle;
    Map Attributes;
    Map Uniforms;
};

struct render_index_buffer
{
    u32 Handle;
    u32 Count;
};

enum struct render_draw_mode
{
    points,
    line_strip,
    line_loop,
    lines,
    triangles,
    triangle_strip,
    triangle_fan
};

enum struct render_blend_mode
{
    gl_one,
    gl_src_alpha,
};
typedef render_blend_mode blend_mode;

struct render_camera
{
    v3 Position;
    v3 Target;
    v3 Up;
    
    real32 inAspectRatio;
    real32 FOV;
    v2 WindowDim;
} typedef camera;

enum struct render_piece_type
{
    color_rect,
    bitmap_rect,
    bitmap_id_rect
};

struct render_piece
{
    render_piece_type Type;
    
    v3 Coords;
    v2 Dim;
    real32 Rotation;
    render_blend_mode BlendMode;
    
    v2 ScissorCoords;
    v2 ScissorDim;
    
    uint32 Color;
    loaded_bitmap *Bitmap;
    bitmap_id BitmapID;
    
    inline render_piece() {}
    inline render_piece(render_piece_type _Type,
                        v3 _Coords,
                        v2 _Dim,
                        real32 _Rotation,
                        render_blend_mode _BlendMode,
                        v2 _ScissorCoords,
                        v2 _ScissorDim,
                        uint32 _Color,
                        loaded_bitmap *_Bitmap,
                        bitmap_id _BitmapID) :
    Type(_Type),
    Coords(_Coords),
    Dim(_Dim),
    Rotation(_Rotation),
    BlendMode(_BlendMode),
    ScissorCoords(_ScissorCoords),
    ScissorDim(_ScissorDim),
    Color(_Color),
    Bitmap(_Bitmap),
    BitmapID(_BitmapID)
    {}
};

#define RENDER_PIECE_GROUP_MAX_SIZE 10000
struct render_piece_group
{
    render_piece Pieces[RENDER_PIECE_GROUP_MAX_SIZE];
    uint32 Size = 0;
    uint32 MaxSize = RENDER_PIECE_GROUP_MAX_SIZE;
    
    render_piece* operator[](int i) { return &Pieces[i]; }
};
global_variable  render_piece_group RPGroup = {};
inline void Push(render_piece Piece)
{
    *RPGroup[RPGroup.Size++] = Piece;
}
inline void Push(v3 Coords, v2 Dim, loaded_bitmap *Bitmap, real32 Rotation, render_blend_mode BlendMode)
{
    Push(render_piece(render_piece_type::bitmap_rect, Coords, Dim, Rotation, BlendMode, 0, 0, 0, Bitmap, bitmap_id()));
}
inline void Push(v3 Coords, v2 Dim, v2 ScissorCoords, v2 ScissorDim, loaded_bitmap *Bitmap, real32 Rotation, render_blend_mode BlendMode)
{
    Push(render_piece(render_piece_type::bitmap_rect, Coords, Dim, Rotation, BlendMode, ScissorCoords, ScissorDim, 0, Bitmap, bitmap_id()));
}
inline void Push(v3 Coords, v2 Dim, bitmap_id BitmapID, real32 Rotation, render_blend_mode BlendMode)
{
    Push(render_piece(render_piece_type::bitmap_id_rect, Coords, Dim, Rotation, BlendMode, 0, 0, 0, 0, BitmapID));
}
inline void Push(v3 Coords, v2 Dim, uint32 Color, real32 Rotation)
{
    Push(render_piece(render_piece_type::color_rect, Coords, Dim, Rotation, render_blend_mode::gl_one, 0, 0, Color, 0, bitmap_id()));
}

inline void ClearPieceGroup(render_piece_group *Group)
{
    memset(Group->Pieces, 0, Group->MaxSize * sizeof(render_piece));
    Group->Size = 0;
}

void glDraw(u32 vertexCount, render_draw_mode DrawMode);
void glDraw(render_index_buffer& IndexBuffer, render_draw_mode DrawMode);
void glDrawInstanced(u32 VertexCount, render_draw_mode DrawMode, u32 NumInstances);
void glDrawInstanced(render_index_buffer& IndexBuffer,  render_draw_mode DrawMode, u32 InstanceCount);

void DrawRect(int x, int y, int width, int height, uint32 color);
void DrawRect(v3 Coords, v2 Size, uint32 color, real32 Rotation);
void DrawRect(v3 Coords, v2 Size, loaded_bitmap *Bitmap, real32 Rotation, render_blend_mode BlendMode);
void DrawRect(v3 Coords, v2 Size, v2 ScissorCoords, v2 ScissorDim, loaded_bitmap *Bitmap, real32 Rotation, render_blend_mode BlendMode);
void DrawRect(assets *Assets, v3 Coords, v2 Size, bitmap_id BitmapID, real32 Rotation, blend_mode BlendMode);

internal void
RenderPieceGroup(assets *Assets)
{
    render_piece_group &Group = RPGroup;
    
    // Z-Sort using Insertion Sort
    {
        uint32 i = 1;
        while (i < Group.Size) {
            int j = i;
            while (j > 0 && Group[j-1]->Coords.z > Group[j]->Coords.z) {
                render_piece Temp = *Group[j];
                *Group[j] = *Group[j-1];
                *Group[j-1] = Temp;
                j = j - 1;
            }
            i++;
        }
    }
    
    for (uint32 i = 0; i < Group.Size; i++) {
        render_piece *p = Group[i];
        if (p->Type == render_piece_type::bitmap_id_rect)
            DrawRect(Assets, p->Coords, p->Dim, p->BitmapID, p->Rotation, p->BlendMode);
        else if (p->Type == render_piece_type::bitmap_rect)
            DrawRect(p->Coords, p->Dim, p->ScissorCoords, p->ScissorDim, p->Bitmap, p->Rotation, p->BlendMode);
        else if (p->Type == render_piece_type::color_rect)
            DrawRect(p->Coords, p->Dim, p->Color, p->Rotation);
    }
    
    ClearPieceGroup(&Group);
}

// handle to the global opengl Vertex Array Object (VAO)
global_variable GLuint gVertexArrayObject = 0;

//
// Animation
//

enum struct interpolation
{
    constant,
    linear,
    cubic,
};

template<unsigned int N>
struct frame
{
    real32 Value[N];
    real32 In[N];
    real32 Out[N];
    real32 Time;
};
typedef frame<1> frame_scalar;
typedef frame<3> frame_vector;
typedef frame<4> frame_quaternion;

template<typename T, int N>
struct track
{
    DynArray<frame<N>> Frames;
    interpolation Interpolation;
    
    frame<N>& operator[](unsigned int index);
};
typedef track<real32, 1> track_scaler;
typedef track<v3, 3> track_vector;
typedef track<quat, 4> track_quaternion;

#endif //RENDERER_H
