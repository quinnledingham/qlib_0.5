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
#if QLIB_SLOW
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
typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int);
typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);

#endif // _WIN32

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"
#include "stb/stb_truetype.h"

#include "types.h"
#include "image.h"
#include "data_structures.h"
#include "renderer.h"
#include "text.h"
#include "gui.h"
#include "platform.h"
#include "application.h"
#include "math.h"
#include "memorymanager.h"
#include "strinq.h"
#include "cgltf/GLTFLoader.h"

#include "strinq.cpp"
#include "socketq.h"
#include "win32_application.cpp"
#include "data_structures.cpp"
#include "image.cpp"
#include "text.cpp"
#include "renderer.cpp"
//#include "socketq.cpp"
#include "gui.cpp"
#include "random.cpp"
