#include "debug_buffer.cpp"

#if defined(_WIN32)

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include "glad/glad.h"
#include "glad/glad.c"
#undef APIENTRY
#include <windows.h>
#include <iostream>

// Loading opengl32 from code
#if RENDERDER_SLOW
#pragma comment(linker, "/subsystem:console")
int main(int argc, const char** argv) 
{
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
}
#else
#pragma comment(linker, "/subsystem:windows")
#endif
#pragma comment(lib, "opengl32.lib")

// opengl declarations
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)
(HDC, HGLRC, const int*);
typedef const char*
(WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int);
typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);

#endif // _WIN32

#include "types.h"
#include "renderer.h"
#include "application.h"
#include "math.h"
#include "strinq.h"
#include "data_structures.h"
#include "application.h"
#include "renderer.h"
#include "cgltf/GLTFLoader.h"

#include "win32_application.cpp"
#include "data_structures.cpp"
#include "strinq.cpp"
#include "renderer.cpp"