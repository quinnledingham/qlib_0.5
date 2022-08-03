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
typedef render_attribute render_index_buffer;

struct render_shader
{
    u32 Handle;
    Map Attributes;
    Map Uniforms;
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

enum struct render_coord_system
{
    /*
(0, 0) = center of screen
right = +x
left = -x
up = +y
down = -y
towards camera = +z
away from camera = -z
*/
    center, 
    
    /*
(0, 0) = top right of screen
right = +x
left = -x
up = -y
down = +y
*/
    top_right,
};

struct render_camera
{
    // Camera Attributes
    v3 Position;
    v3 Target;
    v3 Up;
    real32 inAspectRatio;
    real32 FOV;
    
    // OpenGL Info
    bool32 Mode3D;
    bool32 OpenGLInitialized;
    
    mat4 Projection;
    mat4 View;
    
    iv2 PlatformDim;
    iv2 WindowDim;
    
    render_coord_system CoordSystem;
} typedef camera;

enum struct render_piece_type
{
    color_rect,
    bitmap_rect,
    bitmap_id_rect
};

struct render_bitmap
{
    v2 ScissorCoords;
    v2 ScissorDim;
    loaded_bitmap *Bitmap;
    bitmap_id BitmapID;
};

struct render_piece
{
    render_piece_type Type;
    
    v3 Coords;
    v2 Dim;
    real32 Rotation;
    render_blend_mode BlendMode;
    render_coord_system CoordSystem;
    
    uint32 Color;
    
    v2 ScissorCoords;
    v2 ScissorDim;
    loaded_bitmap *Bitmap;
    bitmap_id BitmapID;
    
    inline render_piece() {}
    inline render_piece(render_piece_type _Type,
                        v3 _Coords,
                        v2 _Dim,
                        real32 _Rotation,
                        render_blend_mode _BlendMode,
                        render_coord_system _CoordSystem,
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
    CoordSystem(_CoordSystem),
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

global_variable render_blend_mode GlobalBlendMode;
#define BLEND_MODE_GL_SRC_ALPHA render_blend_mode::gl_src_alpha
#define BLEND_MODE_GL_ONE render_blend_mode::gl_one
#define qlibBlendMode(i) (GlobalBlendMode = i)

global_variable render_coord_system GlobalCoordSystem;
#define QLIB_TOP_RIGHT render_coord_system::top_right
#define QLIB_CENTER render_coord_system::center
#define qlibCoordSystem(i) (GlobalCoordSystem = i)

//bitmap_rect,
inline void Push(v3 Coords, v2 Dim, v2 ScissorCoords, v2 ScissorDim, loaded_bitmap *Bitmap, real32 Rotation, render_blend_mode BlendMode)
{
    Push(render_piece(render_piece_type::bitmap_rect, Coords, Dim, Rotation, BlendMode, GlobalCoordSystem, ScissorCoords, ScissorDim, 0, Bitmap, bitmap_id()));
}
inline void Push(v3 Coords, v2 Dim, loaded_bitmap *Bitmap, real32 Rotation, render_blend_mode BlendMode)
{
    Push(Coords, Dim, 0, 0, Bitmap, Rotation, BlendMode);
}
inline void Push(v3 Coords, v2 Dim, loaded_bitmap *Bitmap)
{
    Push(Coords, Dim, Bitmap, 0.0f, GlobalBlendMode);
}

//bitmap_id_rect
inline void Push(v3 Coords, v2 Dim, bitmap_id BitmapID, real32 Rotation, render_blend_mode BlendMode)
{
    Push(render_piece(render_piece_type::bitmap_id_rect, Coords, Dim, Rotation, BlendMode, GlobalCoordSystem, 0, 0, 0, 0, BitmapID));
}
inline void Push(v3 Coords, v2 Dim, bitmap_id BitmapID, real32 Rotation)
{
    Push(Coords, Dim, BitmapID, Rotation, GlobalBlendMode);
}
inline void Push(v3 Coords, v2 Dim, bitmap_id BitmapID)
{
    Push(Coords, Dim, BitmapID, 0.0f);
}

// color_rect,
inline void Push(v3 Coords, v2 Dim, uint32 Color, real32 Rotation)
{
    Push(render_piece(render_piece_type::color_rect, Coords, Dim, Rotation, render_blend_mode::gl_one, GlobalCoordSystem, 0, 0, Color, 0, bitmap_id()));
}
inline void Push(v3 Coords, v2 Dim, uint32 Color)
{
    Push(Coords, Dim, Color, 0.0f);
}

inline void ClearPieceGroup(render_piece_group *Group)
{
    memset(Group->Pieces, 0, Group->MaxSize * sizeof(render_piece));
    Group->Size = 0;
}

void DrawRect(render_camera *Camera, render_piece *Piece, render_shader *Shader);
void DrawRect(render_camera *Camera, render_piece *Piece);
void DrawRect(render_camera *Camera, render_piece *Piece, loaded_bitmap *Bitmap);
void DrawRect(render_camera *Camera, render_piece *Piece, assets *Assets);

void BeginRenderer(camera *C);

internal void
RenderPieceGroup(render_camera *Camera, assets *Assets)
{
    BeginRenderer(Camera);
    
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
        
#if QLIB_OPENGL
        if (p->BlendMode == render_blend_mode::gl_one)
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        else if (p->BlendMode == render_blend_mode::gl_src_alpha)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
        
        p->Coords = p->Coords + v3(p->Dim.x/2, p->Dim.y/2, 0.0f);
        
        if (p->CoordSystem == render_coord_system::top_right)
        {
            v2 CoordsShift = v2(-(real32)Camera->WindowDim.x/2, -(real32)Camera->WindowDim.y/2);
            p->Coords = p->Coords + v3(CoordsShift, 0);
            p->Coords.y *= -1;
        }
        else if (p->CoordSystem == render_coord_system::center)
        {
            
        }
        
        if (p->Type == render_piece_type::bitmap_id_rect)
            DrawRect(Camera, p, Assets);
        if (p->Type == render_piece_type::bitmap_rect)
            DrawRect(Camera, p, p->Bitmap);
        else if (p->Type == render_piece_type::color_rect)
            DrawRect(Camera, p);
    }
    
    ClearPieceGroup(&Group);
}

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
