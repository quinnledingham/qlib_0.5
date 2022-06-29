#ifndef TYPES_H
#define TYPES_H

#if QLIB_SLOW
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define AlignPow2(Value, Alignment) ((Value + ((Alignment) - 1)) & ~((Alignment) - 1))
#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)

#define internal static 
#define local_persist static 
#define global_variable static

#define Pi32 3.14159265359f
#define Tau32 6.28318530717958647692f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

typedef int8 s8;
typedef int8 s08;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;

typedef uint8 u8;
typedef uint8 u08;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef real32 r32;
typedef real64 r64;

typedef uintptr_t umm;
typedef intptr_t smm;

#define DEG2RAD 0.0174533f

typedef struct v2
{
    union
    {
        struct
        {
            real32 x;
            real32 y;
        };
        struct
        {
            real32 Width;
            real32 Height;
        };
        real32 v[2];
    };
    inline v2() : x(0.0f), y(0.0f) {}
    inline v2(int32 _xy) : x((real32)_xy), y((real32)_xy) {}
    inline v2(real32 _xy) : x(_xy), y(_xy) {}
    inline v2(real32 _x, real32 _y) : x(_x), y(_y) {}
    inline v2(real32 *fv) : x(fv[0]), y(fv[1]) {}
    inline v2(int32 _x, int32 _y) : x((real32)_x), y((real32)_y) {}
    inline v2(unsigned int _x, unsigned int _y) : x((real32)_x), y((real32)_y) {}
} Dim;

struct iv2
{
    union
    {
        struct
        {
            int x;
            int y;
        };
        int v[2];
    };
    inline iv2() : x(0), y(0) {}
    inline iv2(int _x, int _y) : x(_x), y(_y) {}
    inline iv2(int* fv) : x(fv[0]), y(fv[1]) {}
    inline iv2(v2 _v) : x((int)_v.x), y((int)_v.y) {}
};

struct v3
{
    union
    {
        struct
        {
            real32 x;
            real32 y;
            real32 z;
        };
        real32 v[3];
    };
    inline v3() : x(0.0f), y(0.0f), z(0.0f) {}
    inline v3(real32 _x, real32 _y, real32 _z) : x(_x), y(_y), z(_z) {}
    inline v3(real32 *fv) : x(fv[0]), y(fv[1]), z(fv[2]) {}
    inline v3(v2 v, real32 f) : x(v.x), y(v.y), z(f) {}
};

struct v4
{
    union
    {
        struct
        {
            real32 x;
            real32 y;
            real32 z;
            real32 w;
        };
        real32 v[4];
    };
    inline v4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    inline v4(real32 _x, real32 _y, real32 _z, real32 _w) : x(_x), y(_y), z(_z), w(_w) {}
    inline v4(int32 _x, int32 _y, int32 _z, int32 _w) : x((real32)_x), y((real32)_y), z((real32)_z), w((real32)_w) {}
    inline v4(real32 *fv) : x(fv[0]), y(fv[1]), z(fv[2]), w(fv[3]) {}
};

struct iv4
{
    union
    {
        struct
        {
            int x;
            int y;
            int z;
            int w;
        };
        int v[4];
    };
    inline iv4() : x(0), y(0), z(0), w(0) {}
    inline iv4(int _x, int _y, int _z, int _w) : x(_x), y(_y), z(_z), w(_w) {}
    inline iv4(int *fv) : x(fv[0]), y(fv[1]), z(fv[2]), w(fv[3]) {}
};

struct mat4
{
    union
    {
        real32 v[16];
        
        struct
        {
            v4 right;
            v4 up;
            v4 forward;
            v4 position;
        };
        
        struct {
            // row 1 row 2 row 3 row 4
            /*col 1*/float xx;float xy;float xz;float xw;
            /*col 2*/float yx;float yy;float yz;float yw;
            /*col 3*/float zx;float zy;float zz;float zw;
            /*col 4*/float tx;float ty;float tz;float tw;
        };
        
        struct {
            float c0r0;float c0r1;float c0r2;float c0r3;
            float c1r0;float c1r1;float c1r2;float c1r3;
            float c2r0;float c2r1;float c2r2;float c2r3;
            float c3r0;float c3r1;float c3r2;float c3r3;
        };
        
        struct {
            float r0c0;float r1c0;float r2c0;float r3c0;
            float r0c1;float r1c1;float r2c1;float r3c1;
            float r0c2;float r1c2;float r2c2;float r3c2;
            float r0c3;float r1c3;float r2c3;float r3c3;
        };
    };
    // Identity Matrix
    inline mat4() :
    xx(1), xy(0), xz(0), xw(0),
    yx(0), yy(1), yz(0), yw(0),
    zx(0), zy(0), zz(1), zw(0),
    tx(0), ty(0), tz(0), tw(1) {}
    
    inline mat4(float f) :
    xx(f), xy(0), xz(0), xw(0),
    yx(0), yy(f), yz(0), yw(0),
    zx(0), zy(0), zz(f), zw(0),
    tx(0), ty(0), tz(0), tw(f) {}
    
    inline mat4(float *fv) :
    xx( fv[0]), xy( fv[1]), xz( fv[2]), xw( fv[3]),
    yx( fv[4]), yy( fv[5]), yz( fv[6]), yw( fv[7]),
    zx( fv[8]), zy( fv[9]), zz(fv[10]), zw(fv[11]),
    tx(fv[12]), ty(fv[13]), tz(fv[14]), tw(fv[15]) { }
    
    inline mat4(
                float _00, float _01, float _02, float _03,
                float _10, float _11, float _12, float _13,
                float _20, float _21, float _22, float _23,
                float _30, float _31, float _32, float _33) :
    xx(_00), xy(_01), xz(_02), xw(_03),
    yx(_10), yy(_11), yz(_12), yw(_13),
    zx(_20), zy(_21), zz(_22), zw(_23),
    tx(_30), ty(_31), tz(_32), tw(_33) { }
};

struct quat
{
    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
        struct
        {
            v3 vector;
            float scalar;
        };
        float v[4];
    };
    
    inline quat() : x(0), y(0), z(0), w(1) { }
    inline quat(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) { }
};

struct Transform
{
    v3 Position;
    quat Rotation;
    v3 Scale;
    
    inline Transform(const v3& p, const quat& r, const v3& s) : Position(p), Rotation(r), Scale(s) {}
    inline Transform() : Position(v3(0, 0, 0)), Rotation(quat(0, 0, 0, 1)), Scale(v3(1, 1, 1)) {}
};

struct qlib_bool
{
    bool Value;
    bool New;
};
inline void Toggle(qlib_bool *Bool)
{
    Bool->Value = !Bool->Value;
    Bool->New = true;
}
inline bool GetNew(qlib_bool *Bool)
{
    bool Ret = Bool->New;
    Bool->New = false;
    return Ret;
}

#endif //TYPES_H
