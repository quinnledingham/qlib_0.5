#ifndef APPLICATION_H
#define APPLICATION_H

#ifdef _WIN32

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#ifdef QLIB_WINDOW_APPLICATION
#include "glad/glad.h"
#include "glad/glad.c"
//#undef APIENTRY
#endif
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdint.h>
#include <intrin.h>
#include <stdio.h>
#include <xinput.h>

#include "types.h"
#include "audio.h"
#include "platform.h"
#include "win32_thread.h"

#ifdef QLIB_WINDOW_APPLICATION
#if QLIB_SLOW
#pragma comment(linker, "/subsystem:console")
int main(int argc, const char** argv) { return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT); }
#else
#pragma comment(linker, "/subsystem:windows")
#endif
#pragma comment(lib, "opengl32.lib")

/*
// Enabling Discrete Graphics?
extern "C" 
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

extern "C"
{
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
*/

// opengl declarations
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC, HGLRC, const int*);
typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int);
typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);

#include "memorymanager.h"
#include "data_structures.h"
#include "image.h"
#include "math.h"

internal void TextureInit(loaded_bitmap *Bitmap);

#include "text.h"
#include "asset.h"
#include "renderer.h"

#include "asset.cpp"
#include "text.cpp"
#include "audio.cpp"

void UpdateRender(platform* p);
#endif

#ifdef QLIB_CONSOLE_APPLICATION
void Update(platform* p);
#endif // QLIB_CONSOLE_APPLICATION

#include "win32_application.h"
#include "win32_application.cpp"

#include "renderer.cpp"
#include "menu.h"
#include "random.h"
#include "socketq.h"

#endif // _WIN32

#ifdef __EMSCRIPTEN__

#include <SDL2/SDL.h>
#include <stdio.h>
#include <emscripten/emscripten.h>
#include <string.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>

using emscripten::val;

#endif

#endif //APPLICATION_H
