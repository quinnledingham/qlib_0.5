#ifndef APPLICATION_H
#define APPLICATION_H

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <stdint.h>
#include <intrin.h>
#include <stdio.h>

#ifdef QLIB_WINDOW_APPLICATION
#include "glad/glad.h"
#include "glad/glad.c"
#undef APIENTRY
/*
#if QLIB_SLOW
#pragma comment(linker, "/subsystem:console")
int main(int argc, const char** argv) { return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT); }
#else
#pragma comment(linker, "/subsystem:windows")
#endif
*/
#pragma comment(lib, "opengl32.lib")
// opengl declarations
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)
(HDC, HGLRC, const int*);
typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int);
typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);
#endif // QLIB_WINDOW_APPLICATION

#endif // _WIN32

#include "qlib/types.h"
#include "qlib/platform.h"
#include "memorymanager.h"

#ifdef QLIB_WINDOW_APPLICATION
#include "data_structures.h"
#include "image.h"
#include "renderer.h"
#include "text.h"
#include "math.h"
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
// handle to the global opengl Vertex Array Object (VAO)
global_variable GLuint gVertexArrayObject = 0;
#include "image.cpp"
#include "text.cpp"
#include "renderer.cpp"
void UpdateRender(platform* p);
#endif

#ifdef QLIB_CONSOLE_APPLICATION
void Update(platform* p);
#endif // QLIB_CONSOLE_APPLICATION

#include "win32_application.cpp"

#endif //APPLICATION_H
