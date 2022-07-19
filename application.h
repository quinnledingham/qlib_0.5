#ifndef APPLICATION_H
#define APPLICATION_H

#ifdef QLIB_DISCRETE_GRAPHICS
// Enabling Discrete Graphics?
extern "C" 
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

extern "C"
{
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

// Load OpenGL
#ifdef QLIB_OPENGL

#include "glad/glad.h"
#include "glad/glad.c"

#pragma comment(lib, "opengl32.lib")

#ifndef __EMSCRIPTEN__
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
#endif 

// handle to the global opengl Vertex Array Object (VAO)
static GLuint gVertexArrayObject = 0;

#endif // QLIB_OPENGL

// Load Windows
#ifdef _WIN32

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <intrin.h>
#include <xinput.h>

#include <stdint.h>
#include <stdio.h>

inline void PlatformSetCD(const char* Dir) { SetCurrentDirectory(Dir); }

#endif // _WIN32

// Load Wasm
#ifdef __EMSCRIPTEN__

#include <SDL2/SDL.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>
#include <emscripten/fetch.h>

#include <stdio.h>
#include <string.h>

using emscripten::val;

#endif // __EMSCRIPTEN__

// Use SDL
#ifdef QLIB_SDL
/*
#include "sdl-vc/include/SDL.h"
#include "sdl-vc/include/SDL_main.h"
#include "sdl-vc/include/SDL_video.h"
#include "sdl-vc/include/SDL_opengl.h"
*/
#include <stdio.h>
#include <string.h>

//#pragma comment(linker, "/subsystem:console")
//#define main SDL_main

#endif

#ifdef QLIB_WINDOW_APPLICATION

#include "types.h"
#include "audio.h"
#include "platform.h"
#include "memorymanager.h"
#include "data_structures.h"
#include "image.h"
#include "math.h"
internal void TextureInit(loaded_bitmap *Bitmap);
#include "text.h"
#include "asset.h"
#include "renderer.h"
#include "random.h"
//#include "socketq.h"

#include "asset.cpp"
#include "text.cpp"
#include "menu.h"
#include "audio.cpp"
#include "renderer.cpp"

void UpdateRender(platform* p);

#endif // QLIB_WINDOW_APPLICATION

#ifdef QLIB_CONSOLE_APPLICATION

#include "types.h"
#include "audio.h"
#include "platform.h"
#include "win32_thread.h"
#include "random.h"
#include "socketq.h"

void Update(platform* p);

#endif

// Defining platform specific functions
#ifdef QLIB_SDL

#include "sdl_application.cpp"

#else

#if _WIN32

#include "win32_thread.h"
#include "win32_application.h"
#include "win32_application.cpp"

#endif // _WIN32

#endif 
// End of defining platform specific functions

#endif //APPLICATION_H
