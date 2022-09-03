#ifndef MATH_H
#define MATH_H

#include "types.h"

inline int32
RoundReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)roundf(Real32);
    return(Result);
}

// Takes trial and error to find right value
// https://bitbashing.io/comparing-floats.html
#define V3_EPSILON 0.000001f
#define MAT4_EPSILON 0.000001f
#define QUAT_EPSILON 0.000001f

// v2 Operations
inline v2
operator+(const v2 &l, const v2 &r)
{
    return v2(l.x + r.x, l.y + r.y);
}

inline v2
operator+=(const v2 &l, const v2& r)
{
    return v2(l.x + r.x, l.y + r.y);
}

inline v2
operator-(const v2 &l, const v2 &r)
{
    return v2(l.x - r.x, l.y - r.y);
}


inline v2
operator-(const v2 &l, float &r)
{
    return v2(l.x - r, l.y - r);
}

inline v2
operator*(const v2 &l, const v2 &r)
{
    return v2(l.x * r.x, l.y * r.y);
}

inline v2
operator*(const v2 &l, float &r)
{
    return v2(l.x * r, l.y * r);
}

inline v2
operator/(const v2 &l, float r)
{
    return v2(l.x / r, l.y / r);
}

inline v2
operator/(const v2 &l, const v2 &r)
{
    return v2(l.x / r.x, l.y / r.y);
}

inline v2
operator/(const iv2 &l, const v2 &r)
{
    return v2(l.x / r.x, l.y / r.y);
}

inline bool
operator==(const v2 &l, const v2 &r)
{
    if (l.x == r.x && l.y == r.y)
        return true;
    return false;
}

inline bool
operator!=(const v2 &l, const v2 &r)
{
    if (l.x == r.x && l.y == r.y)
        return false;
    return true;
}

inline bool
operator!=(const iv2 &l, const iv2 &r)
{
    if (l.x == r.x && l.y == r.y)
        return false;
    return true;
}

inline v2
abso(const v2 &v)
{
    v2 r = v2();
    if (v.x < 0)
        r.x = -v.x;
    else
        r.x = v.x;
    
    if (v.y < 0)
        r.y = -v.y;
    else
        r.y = v.y;
    return r;
}

inline real32
absol(real32 r)
{
    if (r < 0)
        r *= -1;
    return r;
}

// v3 Operations
v4 u32toV4(uint32 input)
{
    uint8 A = uint8(input >> 24);
    uint8 R = uint8(input >> 16);
    uint8 G = uint8(input >> 8);
    uint8 B = uint8(input);
    
    return v4(real32(R), real32(G), real32(B), real32(A));
}

v3 u32toV3(uint32 input)
{
    v4 r = u32toV4(input);
    return v3(r.x, r.y, r.z);
}

// Vector Addition
inline v3
operator+(const v3 &l, const v3 &r)
{
    return v3(l.x + r.x, l.y + r.y, l.z + r.z);
}

inline v3
operator+=(const v3 &l, const v3 &r)
{
    return v3(l.x + r.x, l.y + r.y, l.z + r.z);
}

// Vector Subtraction
inline v3
operator-(const v3 &l, const v3 &r)
{
    return v3(l.x - r.x, l.y - r.y, l.z - r.z);
}

// Scaling Vectors
inline v3
operator*(const v3 &v, float f)
{
    return v3(v.x * f, v.y * f, v.z * f);
}

// Multiplying Vectors
inline v3
operator*(const v3 &l, const v3 &r)
{
    return v3(l.x * r.x, l.y * r.y, l.z * r.z);
}

// Dot Product
inline real32
Dot(const v3 &l, const v3 &r)
{
    return (l.x * r.x) + (l.y * r.y) + (l.z * r.z);
}

// Square Length
inline real32
LenSq(const v3 &v)
{
    return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
}

// Length
inline real32
GetLength(const v3 &v)
{
    real32 LengthSquared = LenSq(v);
    if (LengthSquared < V3_EPSILON)
    {
        return 0.0f;
    }
    return sqrtf(LengthSquared);
}

// Normalize (modifies input vector)
inline void
Normalize(v3 &v)
{
    real32 LengthSquared = LenSq(v);
    if (LengthSquared < V3_EPSILON)
    {
        return;
    }
    
    real32 InvLen = 1.0f / sqrtf(LengthSquared);
    v.x *= InvLen;
    v.y *= InvLen;
    v.z *= InvLen;
}

// Normalized (does not modify input vector)
inline v3
Normalized(const v3 &v)
{
    real32 LengthSquared = LenSq(v);
    if (LengthSquared < V3_EPSILON)
    {
        return v;
    }
    real32 InvLen = 1.0f / sqrtf(LengthSquared);
    return v3(v.x * InvLen, v.y * InvLen, v.z * InvLen);
}

// Angle (Radians)
inline real32
Angle(const v3 &l, const v3 &r)
{
    real32 LenL = GetLength(l);
    real32 LenR = GetLength(r);
    
    if (LenL < V3_EPSILON || LenR < V3_EPSILON) {
        return 0.0f;
    }
    
    real32 dot = Dot(l, r);
    real32 len =  LenL * LenR ;
    return acosf(dot / len);
}

// Projection of A onto B
inline v3
Project(const v3 &a, const v3 &b)
{
    real32 MagBSq = GetLength(b);
    if (MagBSq < V3_EPSILON)
    {
        return v3();
    }
    float Scale = Dot(a, b) / MagBSq;
    return (b * Scale);
}

// Rejection of A onto B
inline v3
Reject(const v3 &a, const v3 &b)
{
    v3 Projection = Project(a, b);
    return (a - Projection);
}

inline v3
Rotate(const v3 &a, real32 angle)
{
    //angle = angle * Pi32 / 180.0f;
    angle = angle * DEG2RAD; 
    
    v3 Rotated = 
    {
        ((cosf(angle) * a.x) - (sinf(angle) * a.z)),
        a.y,
        ((sinf(angle) * a.x) - (cosf(angle) * a.z)),
    };
    
    if (Rotated.x < V3_EPSILON && Rotated.x > -V3_EPSILON)
    {
        Rotated.x = 0;
    }
    if (Rotated.y < V3_EPSILON && Rotated.y > -V3_EPSILON)
    {
        Rotated.y = 0;
    }
    if (Rotated.z < V3_EPSILON && Rotated.z > -V3_EPSILON)
    {
        Rotated.z = 0;
    }
    
    return Rotated;
}

// Vector Reflection (Bounce Reflection)
inline v3
Reflect(const v3 &a, const v3 &b)
{
    real32 MagBSq = GetLength(b);
    if (MagBSq < V3_EPSILON)
    {
        return v3();
    }
    real32 Scale = Dot(a, b) / MagBSq;
    v3 Proj2 = b * (Scale * 2);
    return (a - Proj2);
}

// Cross Product
inline v3
Cross(const v3 &l, const v3 &r)
{
    return v3(
              (l.y * r.z - l.z * r.y),
              (l.z * r.x - l.x * r.z),
              (l.x * r.y - l.y * r.x)
              );
}

// Linear Interpolation
inline v3
Lerp(const v3 &s, const v3 &e, float t)
{
    return v3(
              (s.x + (e.x - s.x) * t),
              (s.y + (e.y - s.y) * t),
              (s.z + (e.z - s.z) * t)
              );
}

// Spherical Linear Interpolation
inline v3
Slerp(const v3 &s, const v3 &e, real32 t)
{
    if (t < 0.01f)
    {
        return Lerp(s, e, t);
    }
    
    v3 From = Normalized(s);
    v3 To = Normalized(e);
    
    real32 Theta = Angle(From, To);
    real32 Sin_Theta = sinf(Theta);
    
    real32 a = sinf((1.0f - t) * Theta) / Sin_Theta;
    real32 b = sinf(t * Theta) / Sin_Theta;
    
    return From * a + To * b;
}

// Nlerp approximates Slerp
inline v3
Nlerp(const v3 &s, const v3 &e, float t)
{
    v3 Linear(
              (s.x + (e.x - s.x) * t),
              (s.y + (e.y - s.y) * t),
              (s.z + (e.z - s.z) * t)
              );
    return Normalized(Linear);
}

// Equals
inline bool32
operator==(const v3 &l, const v3 &r)
{
    v3 Diff(l - r);
    return LenSq(Diff) < V3_EPSILON;
}

// Not-Equals
inline bool32
operator!=(const v3 &l, const v3 &r)
{
    return !(l == r);
}

// mat4 Operations
// Equals
bool operator==(const mat4& a, const mat4& b)
{
    for (int i = 0; i < 16; ++i)
    {
        if (fabsf(a.v[i] - b.v[i]) > MAT4_EPSILON)
        {
            return false;
        }
    }
    return true;
}

// Not-Equals
bool operator!=(const mat4& a, const mat4& b)
{
    return !(a == b);
}

// Adding Matrices
mat4 operator+(const mat4& a, const mat4& b)
{
    return mat4(
                a.xx + b.xx, a.xy + b.xy, a.xz + b.xz, a.xw + b.xw,
                a.yx + b.yx, a.yy + b.yy, a.yz + b.yz, a.yw + b.yw,
                a.zx + b.zx, a.zy + b.zy, a.zz + b.zz, a.zw + b.zw,
                a.tx + b.tx, a.ty + b.ty, a.tz + b.tz, a.tw + b.tw
                );
}

// Scaling Matrix4
mat4 operator*(const mat4& m, float f) {
    return mat4(
                m.xx * f, m.xy * f, m.xz * f, m.xw * f,
                m.yx * f, m.yy * f, m.yz * f, m.yw * f,
                m.zx * f, m.zy * f, m.zz * f, m.zw * f,
                m.tx * f, m.ty * f, m.tz * f, m.tw * f
                );
}

// Dot-product of two rows of a matrix
#define M4D(aRow, bCol) \
a.v[0 * 4 + aRow] * b.v[bCol * 4 + 0] + \
a.v[1 * 4 + aRow] * b.v[bCol * 4 + 1] + \
a.v[2 * 4 + aRow] * b.v[bCol * 4 + 2] + \
a.v[3 * 4 + aRow] * b.v[bCol * 4 + 3]

// Matrix Multiplication
mat4 operator*(const mat4 &a, const mat4 &b) {
    return mat4(
                M4D(0,0), M4D(1,0), M4D(2,0), M4D(3,0),//Col 0
                M4D(0,1), M4D(1,1), M4D(2,1), M4D(3,1),//Col 1
                M4D(0,2), M4D(1,2), M4D(2,2), M4D(3,2),//Col 2
                M4D(0,3), M4D(1,3), M4D(2,3), M4D(3,3) //Col 3
                );
}

// Matrix Vector Multiplication
#define M4V4D(mRow, x, y, z, w) \
x * m.v[0 * 4 + mRow] + \
y * m.v[1 * 4 + mRow] + \
z * m.v[2 * 4 + mRow] + \
w * m.v[3 * 4 + mRow]

v4 operator*(const mat4& m, const v4& v) {
    return v4(
              M4V4D(0, v.x, v.y, v.z, v.w),
              M4V4D(1, v.x, v.y, v.z, v.w),
              M4V4D(2, v.x, v.y, v.z, v.w),
              M4V4D(3, v.x, v.y, v.z, v.w)
              );
}

// Transform a Vector (assuming the vector represents the direction and magnitude)
v3 TransformVector(const mat4& m, const v3& v) {
    return v3(
              M4V4D(0, v.x, v.y, v.z, 0.0f),
              M4V4D(1, v.x, v.y, v.z, 0.0f),
              M4V4D(2, v.x, v.y, v.z, 0.0f)
              );
}

// Transform Point (assumes W component of vector is 1)
v3 TransformPoint(const mat4& m, const v3& v) {
    return v3(
              M4V4D(0, v.x, v.y, v.z, 1.0f),
              M4V4D(1, v.x, v.y, v.z, 1.0f),
              M4V4D(2, v.x, v.y, v.z, 1.0f)
              );
}

// Transform Point (w component holds the value for W, if the input vector had been v4)
v3 TransformPoint(const mat4& m, const v3& v, float& w) {
    float _w = w;
    w = M4V4D(3, v.x, v.y, v.z, _w);
    return v3(
              M4V4D(0, v.x, v.y, v.z, _w),
              M4V4D(1, v.x, v.y, v.z, _w),
              M4V4D(2, v.x, v.y, v.z, _w)
              );
}

// Transpose
#define M4SWAP(x, y) \
{float t = x; x = y; y = t; }

void Transpose(mat4 &m) {
    M4SWAP(m.yx, m.xy);
    M4SWAP(m.zx, m.xz);
    M4SWAP(m.tx, m.xw);
    M4SWAP(m.zy, m.yz);
    M4SWAP(m.ty, m.yw);
    M4SWAP(m.tz, m.zw);
}

// Transpose (Does not modify input matrix)
mat4 Transposed(const mat4 &m) {
    return mat4(
                m.xx, m.yx, m.zx, m.tx,
                m.xy, m.yy, m.zy, m.ty,
                m.xz, m.yz, m.zz, m.tz,
                m.xw, m.yw, m.zw, m.tw
                );
}

// Finds the minor of element in the matrix
#define M4_3X3MINOR(x, c0, c1, c2, r0, r1, r2) \
(x[c0*4+r0]*(x[c1*4+r1]*x[c2*4+r2]-x[c1*4+r2]* \
x[c2*4+r1])-x[c1*4+r0]*(x[c0*4+r1]*x[c2*4+r2]- \
x[c0*4+r2]*x[c2*4+r1])+x[c2*4+r0]*(x[c0*4+r1]* \
x[c1*4+r2]-x[c0*4+r2]*x[c1*4+r1]))

// Finds the determinant
float Determinant(const mat4& m) {
    return m.v[0] *M4_3X3MINOR(m.v, 1, 2, 3, 1, 2, 3)
        - m.v[4] *M4_3X3MINOR(m.v, 0, 2, 3, 1, 2, 3)
        + m.v[8] *M4_3X3MINOR(m.v, 0, 1, 3, 1, 2, 3)
        - m.v[12]*M4_3X3MINOR(m.v, 0, 1, 2, 1, 2, 3);
}

mat4 Adjugate(const mat4& m) {
    //Cof (M[i, j]) = Minor(M[i, j]] * pow(-1, i + j)
    mat4 cofactor;
    cofactor.v[0] = M4_3X3MINOR(m.v, 1, 2, 3, 1, 2, 3);
    cofactor.v[1] =-M4_3X3MINOR(m.v, 1, 2, 3, 0, 2, 3);
    cofactor.v[2] = M4_3X3MINOR(m.v, 1, 2, 3, 0, 1, 3);
    cofactor.v[3] =-M4_3X3MINOR(m.v, 1, 2, 3, 0, 1, 2);
    cofactor.v[4] =-M4_3X3MINOR(m.v, 0, 2, 3, 1, 2, 3);
    cofactor.v[5] = M4_3X3MINOR(m.v, 0, 2, 3, 0, 2, 3);
    cofactor.v[6] =-M4_3X3MINOR(m.v, 0, 2, 3, 0, 1, 3);
    cofactor.v[7] = M4_3X3MINOR(m.v, 0, 2, 3, 0, 1, 2);
    cofactor.v[8] = M4_3X3MINOR(m.v, 0, 1, 3, 1, 2, 3);
    cofactor.v[9] =-M4_3X3MINOR(m.v, 0, 1, 3, 0, 2, 3);
    cofactor.v[10]= M4_3X3MINOR(m.v, 0, 1, 3, 0, 1, 3);
    cofactor.v[11]=-M4_3X3MINOR(m.v, 0, 1, 3, 0, 1, 2);
    cofactor.v[12]=-M4_3X3MINOR(m.v, 0, 1, 2, 1, 2, 3);
    cofactor.v[13]= M4_3X3MINOR(m.v, 0, 1, 2, 0, 2, 3);
    cofactor.v[14]=-M4_3X3MINOR(m.v, 0, 1, 2, 0, 1, 3);
    cofactor.v[15]= M4_3X3MINOR(m.v, 0, 1, 2, 0, 1, 2);
    return Transposed(cofactor);
}

mat4 Inverse(const mat4& m) {
    float det = Determinant(m);
    
    if (det == 0.0f) {
        //cout << " Matrix determinant is 0\n";
        return mat4();
    }
    mat4 adj = Adjugate(m);
    return adj * (1.0f / det);
}

void Invert(mat4& m) {
    float det = Determinant(m);
    if (det == 0.0f) {
        //std::cout << "Matrix determinant is 0\n";
        m = mat4();
        return;
    }
    m = Adjugate(m) * (1.0f / det);
}

// Frustum - Represents the space that a camera can see
mat4 Frustum(float l, float r, float b,
             float t, float n, float f) {
    if (l == r || t == b || n == f) {
        //std::cout << "Invalid frustum\n";
        return mat4(); // Error
    }
    return mat4(
                (2.0f * n) / (r - l),0, 0, 0,
                0, (2.0f * n) / (t - b), 0, 0,
                (r+l)/(r-l), (t+b)/(t-b), (-(f+n))/(f-n), -1,
                0, 0, (-2 * f * n) / (f - n), 0
                );
}

// Perspective Matrix - Creating a view frustum
mat4 Perspective(float fov, float aspect, float n, float f){
    float ymax = n * tanf(fov * 3.14159265359f / 360.0f);
    float xmax = ymax * aspect;
    return Frustum(-xmax, xmax, -ymax, ymax, n, f);
}

// Orthographic Projection - Maps linearly to NDC space
mat4 Ortho(float l, float r, float b, float t,
           float n, float f) {
    if (l == r || t == b || n == f) {
        return mat4(); // Error
    }
    return mat4(
                2.0f / (r - l), 0, 0, 0,
                0, 2.0f / (t - b), 0, 0,
                0, 0, -2.0f / (f - n), 0,
                -((r+l)/(r-l)),-((t+b)/(t-b)),-((f+n)/(f-n)), 1
                );
}

mat4 LookAt(const v3& position, const v3& target, const v3& up)
{
    v3 f = Normalized(target - position) * -1.0f;
    v3 r = Cross(up, f); // Right handed
    if (r == v3(0, 0, 0)) {
        return mat4(); // Error
    }
    Normalize(r);
    v3 u = Normalized(Cross(f, r)); // Right handed
    v3 t = v3(-Dot(r, position),
              -Dot(u, position),
              -Dot(f, position)
              );
    return mat4(
                // Transpose upper 3x3 matrix to invert it
                r.x, u.x, f.x, 0,
                r.y, u.y, f.y, 0,
                r.z, u.z, f.z, 0,
                t.x, t.y, t.z, 1
                );
}

mat4 Identity()
{
    return mat4();
}

mat4 Translate(mat4& m, v3 v)
{
    mat4 T = Identity();
    T.tx = v.x;
    T.ty = v.y;
    T.tz = v.z;
    T.tw = 1;
    
    return (m * T);
}

mat4 GetScale(mat4& m, float v)
{
    return (m * v);
}

// Quaternions
// Converts a angle from a axis into a quat.
quat AngleAxis(float angle, const v3& axis)
{
    v3 norm = Normalized(axis);
    float s = sinf(angle * 0.5f);
    
    return quat(norm.x * s, norm.y * s, norm.z * s,
                cosf(angle * 0.5f));
}

// Returns a quat which contains the rotation between two vectors.
// The two vectors are treated like they are points in the same sphere.
quat FromTo(const v3& from, const v3& to)
{
    v3 f = Normalized(from);
    v3 t = Normalized(to);
    if (f == t)
    {
        return quat();
    }
    else if (f == t * -1.0f)
    {
        v3 ortho = v3(1, 0, 0);
        if (fabsf(f.y) < fabsf(f.x))
        {
            ortho = v3(0, 1, 0);
        }
        if (fabsf(f.z) < fabs(f.y) && fabs(f.z) < fabsf(f.x))
        {
            ortho = v3(0, 0, 1); 
        }
        v3 axis = Normalized(Cross(f, ortho));
        return quat(axis.x, axis.y, axis.z, 0.0f);
    }
    v3 half = Normalized(f + t);
    v3 axis = Cross(f, half);
    return quat(axis.x, axis.y, axis.z, Dot(f, half));
}

// Gets axis that helps define quat
v3 GetAxis(const quat& q)
{
    return Normalized(v3(q.x, q.y, q.z));
}

// Gets angle that helps define quat
real32 GetAngle(const quat& q)
{
    return 2.0f * acosf(q.w);
}

// Component-wise operators
quat operator+(const quat& a, const quat& b)
{
    return quat(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

quat operator-(const quat& a, const quat& b)
{
    return quat(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

quat operator*(const quat& a, real32 b)
{
    return quat(a.x * b, a.y * b, a.z * b, a.w * b);
}

quat operator-(const quat& q)
{
    return quat(-q.x, -q.y, -q.z, -q.w);
}

// Comparison operations
bool operator==(const quat& left, const quat& right)
{
    return (fabsf(left.x - right.x) <= QUAT_EPSILON &&
            fabsf(left.y - right.y) <= QUAT_EPSILON &&
            fabsf(left.z - right.z) <= QUAT_EPSILON &&
            fabsf(left.w - right.w) <= QUAT_EPSILON);
}

bool operator!=(const quat& a, const quat& b)
{
    return !(a == b);
}

bool SameOrientation(const quat& l, const quat& r)
{
    return  (fabsf(l.x - r.x) <= QUAT_EPSILON &&
             fabsf(l.y - r.y) <= QUAT_EPSILON &&
             fabsf(l.z - r.z) <= QUAT_EPSILON &&
             fabsf(l.w - r.w) <= QUAT_EPSILON) ||
    (fabsf(l.x + r.x) <= QUAT_EPSILON &&
     fabsf(l.y + r.y) <= QUAT_EPSILON &&
     fabsf(l.z + r.z) <= QUAT_EPSILON &&
     fabsf(l.w + r.w) <= QUAT_EPSILON);
}

// Dot product
real32 Dot(const quat& a, const quat& b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
}

// Length Squared
real32 LenSq(const quat& q)
{
    return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

// Length
real32 GetLength(const quat& q)
{
    real32 lenSq = LenSq(q);
    if (lenSq < QUAT_EPSILON)
    {
        return 0.0f;
    }
    return sqrtf(lenSq);
}

// Normalize
void Normalize(quat &q)
{
    real32 lenSq = LenSq(q);
    if (lenSq < QUAT_EPSILON)
    {
        return;
    }
    real32 i_len = 1.0f / sqrtf(lenSq);
    
    q.x *= i_len;
    q.y *= i_len;
    q.z *= i_len;
    q.w *= i_len;
}

// Normalized
quat Normalized(const quat& q)
{
    real32 lenSq = LenSq(q);
    if (lenSq < QUAT_EPSILON)
    {
        return quat();
    }
    real32 il = 1.0f / sqrtf(lenSq); // il: inverse length
    
    return quat(q.x * il, q.y * il, q.z * il, q.w * il);
}

// Conjugate (inverse of normalized quaternion)
quat Conjugate(const quat& q)
{
    return quat(-q.x, -q.y, -q.z, q.w);
}

// Proper inverse of a quaternion
quat Inverse(const quat& q)
{
    real32 lenSq = LenSq(q);
    if (lenSq < QUAT_EPSILON)
    {
        return quat();
    }
    real32 recip = 1.0f / lenSq;
    return quat(-q.x * recip, -q.y * recip, -q.z * recip, q.w * recip);
}

// Multiplying quaternions
quat operator*(const quat& Q1, const quat& Q2)
{
    return quat(
                Q2.x*Q1.w + Q2.y*Q1.z - Q2.z*Q1.y + Q2.w*Q1.x,
                -Q2.x*Q1.z + Q2.y*Q1.w + Q2.z*Q1.x + Q2.w*Q1.y,
                Q2.x*Q1.y - Q2.y*Q1.x + Q2.z*Q1.w + Q2.w*Q1.z,
                -Q2.x*Q1.x - Q2.y*Q1.y - Q2.z*Q1.z + Q2.w*Q1.w
                );
}

// Alternative way of multiplying that is less performative
/*
quat operator*(const quat& Q1, const quat& Q2) {
    quat result;
    result.scalar = Q2.scalar * Q1.scalar -
        dot(Q2.vector, Q1.vector);
    result.vector = (Q1.vector * Q2.scalar) +
    (Q2.vector * Q1.scalar)+cross(Q2.vector, Q1.vector);
    return result;
}
*/

// Multiplying a vector and a quaternion
v3 operator*(const quat& q, const v3& v)
{
    return q.vector * 2.0f * Dot(q.vector, v) + 
        v * (q.scalar * q.scalar - Dot(q.vector, q.vector)) +
        Cross(q.vector, v) * 2.0f * q.scalar;
}


// Mix - Each quaternion is scaled by some weight value, then the resulting
// scaled quaternions are added together. All weights of all input quaternions
// must add up to 1.
// Does the same thing as lerp but travels on an arc.
quat Mix(const quat& from, const quat& to, real32 t)
{
    return from * (1.0f - t) + to * t;
}

quat Nlerp(const quat& from, const quat& to, real32 t)
{
    return Normalized(from + (to - from) * t);
}

// Power
quat operator^(const quat& q, real32 f)
{
    real32 angle = 2.0f * acosf(q.scalar);
    v3 axis = Normalized(q.vector);
    
    real32 halfCos = cosf(f * angle * 0.5f);
    real32 halfSin = sinf(f * angle * 0.5f);
    
    return quat(axis.x * halfSin,
                axis.y * halfSin,
                axis.z * halfSin,
                halfCos
                );
}

// Interpolate between two quaternions
quat Slerp(const quat& start, const quat& end, real32 t)
{
    if (fabsf(Dot(start, end)) > 1.0f - QUAT_EPSILON)
    {
        return Nlerp(start, end, t);
    }
    
    quat delta = Inverse(start) * end;
    return Normalized((delta ^ t) * start); 
}

// Rotation to look in direction
quat LookRotation(const v3& direction, const v3& up)
{
    // Find orthonormal basis vectors
    v3 f = Normalized(direction); // Object Forward
    v3 u = Normalized(up); // Desired Up
    v3 r = Cross(u, f); // Object Right
    u = Cross(f, r); // Object Up
    
    // From world forward to object forward
    quat worldToObject = FromTo(v3(0, 0, 1), f);
    
    // What direction is the new object up?
    v3 objectUp = worldToObject * v3(0, 1, 0);
    
    // From object up to desired up
    quat u2u = FromTo(objectUp, u);
    
    // Rotate to forward direction first
    // then twist to correct up
    quat result = worldToObject * u2u;
    
    // Don't forget to normalize the result
    return Normalized(result);
}

// Converting quaternions
mat4 QuatToMat4(const quat& q)
{
    v3 r = q * v3(1, 0, 0);
    v3 u = q * v3(0, 1, 0);
    v3 f = q * v3(0, 0, 1);
    
    return mat4(r.x, r.y, r.z, 0,
                u.x, u.y, u.z, 0,
                f.x, f.y, f.z, 0,
                0, 0, 0, 1
                );
}

quat Mat4ToQuat(const mat4& m)
{
    v3 up = Normalized(v3(m.up.x, m.up.y, m.up.z));
    v3 forward = Normalized(v3(m.forward.x, m.forward.y, m.forward.z));
    v3 right = Cross(up, forward);
    up = Cross(forward, right);
    
    return LookRotation(forward, up);
}

// Transform
Transform Combine(const Transform& a, const Transform& b)
{
    Transform out;
    
    out.Scale = a.Scale * b.Scale;
    out.Rotation = b.Rotation * a.Rotation;
    
    out.Position = a.Rotation * (a.Scale * b.Position);
    out.Position = a.Position + out.Position;
    
    return out;
}

Transform Inverse(const Transform& t)
{
    Transform inv;
    
    inv.Rotation = Inverse(t.Rotation);
    inv.Scale.x = fabs(t.Scale.x) < V3_EPSILON ? 0.0f : 1.0f / t.Scale.x;
    inv.Scale.y = fabs(t.Scale.y) < V3_EPSILON ? 0.0f : 1.0f / t.Scale.y;
    inv.Scale.z = fabs(t.Scale.z) < V3_EPSILON ? 0.0f : 1.0f / t.Scale.z;
    
    v3 invTrans = t.Position * -1.0f;
    inv.Position = inv.Rotation * (inv.Scale * invTrans);
    
    return inv;
}

Transform Mix(const Transform& a, const Transform& b, real32 t)
{
    quat bRot = b.Rotation;
    if (Dot(a.Rotation, bRot) < 0.0f)
    {
        bRot = -bRot;
    }
    
    return Transform(
                     Lerp(a.Position, b.Position, t),
                     Nlerp(a.Rotation, bRot, t),
                     Lerp(a.Scale, b.Scale, t)
                     );
}

mat4 TransformToMat4(const Transform& t)
{
    // First, extract the rotation basis of the transform
    v3 x = t.Rotation * v3(1, 0, 0);
    v3 y = t.Rotation * v3(0, 1, 0);
    v3 z = t.Rotation * v3(0, 0, 1);
    
    // Next, scale the basis vectors
    x = x * t.Scale.x;
    y = y * t.Scale.y;
    z = z * t.Scale.z;
    
    // Extract the position of the transform
    v3 p = t.Position;
    
    // Create matrix
    return mat4(x.x, x.y, x.z, 0, // X basis (& Scale)
                y.x, y.y, y.z, 0, // Y basis (& Scale)
                z.x, z.y, z.z, 0, // Z basis (& Scale)
                p.x, p.y, p.z, 1 // Position
                );
}

Transform Mat4ToTransform(const mat4& m)
{
    Transform out;
    
    out.Position = v3(m.v[12], m.v[13], m.v[14]);
    out.Rotation = Mat4ToQuat(m);
    
    mat4 rotScaleMat(
                     m.v[0], m.v[1], m.v[2], 0,
                     m.v[4], m.v[5], m.v[6], 0,
                     m.v[8], m.v[9], m.v[10], 0,
                     0, 0, 0, 1
                     );
    
    mat4 invRotMat = QuatToMat4(Inverse(out.Rotation));
    mat4 scaleSkewMat = rotScaleMat * invRotMat;
    
    out.Scale = v3(
                   scaleSkewMat.v[0],
                   scaleSkewMat.v[5],
                   scaleSkewMat.v[10]
                   );
    
    return out;
}

v3 TransformPoint(const Transform&a, const v3& b)
{
    v3 out;
    out = a.Rotation * (a.Scale * b);
    out = a.Position + out;
    return out;
}

v3 TransformVector(const Transform& a, const v3& b)
{
    v3 out;
    out = a.Rotation * (a.Scale * b);
    return out;
}

mat4 Rotate(mat4& m, float degs)
{
    return (m * QuatToMat4(AngleAxis(degs * DEG2RAD, v3(0, 0, 1))));
}

// Misc
typedef real32 resize_factor;
inline resize_factor  GetResizeFactor(real32 OldNum, real32 ResizedNum) { return (ResizedNum / OldNum); }
inline v2 GetResizeFactor(v2 OldDim, v2 NewDim)
{
    return NewDim / OldDim;
}
real32 ResizeEquivalentAmount(real32 ToResize, resize_factor ResizeFactor)
{
    if (ResizeFactor == 0)
        return ToResize;
    else
        return (ResizeFactor * ToResize);
}
real32 ResizeEquivalentAmount(real32 ToResize, real32 OldNum, real32 ResizedNum)
{
    if (OldNum == 0)
        return ToResize;
    else
        return ((ResizedNum / OldNum) * ToResize);
}
v2 ResizeEquivalentAmount(v2 ToResize, v2 ResizeFactors)
{
    if (ResizeFactors == 0)
        return ToResize;
    else
        return v2(ResizeFactors.x * ToResize.x, ResizeFactors.y * ToResize.y);
}

//https://stackoverflow.com/questions/26839558/hex-char-to-int-conversion
int Hex2Int(char Char)
{
    if (Char >= '0' && Char <= '9')
        return Char - '0';
    if (Char >= 'A' && Char <= 'F')
        return Char - 'A' + 10;
    if (Char >= 'a' && Char <= 'f')
        return Char - 'a' + 10;
    return -1;
}

inline unsigned long
createRGBA(int r, int g, int b, int a) 
{ 
    return ((a & 0xff) << 24) + ((r & 0xff) << 16) + ((g & 0xff) << 8) + ((b & 0xff));
}

inline unsigned long
createRGB(int r, int g, int b) 
{ 
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff); 
}

uint32 Pow(int a, int n)
{
    if (n == 0)
        return 1;
    
    uint32 Result = a;
    for (int i = 1; i < n; i++)
        Result *= a;
    return Result;
}

int StringHex2Int(char *Hex)
{
    uint32 Result = 0;
    
    for (int i = 2; i < 10; i++)
    {
        int Temp = Hex2Int(Hex[i]);
        Result += (Temp * Pow(16, 10 - i - 1));
    }
    return Result;
}

inline int
Random(int Low, int High)
{
    return (rand() % (High - Low + 1) + Low);
}

// End of Misc

#endif //MATH_H