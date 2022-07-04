#ifndef WIN32_APPLICATION_H
#define WIN32_APPLICATION_H

struct win32_sound_output
{
    int SamplesPerSecond;
    uint32 RunningSampleIndex;
    int BytesPerSample;
    DWORD SecondaryBufferSize;
    DWORD SafetyBytes;
    
    // TODO(casey): Should running sample index be in bytes as well
    // TODO(casey): Math gets simpler if we add a "bytes per second" field?
};

struct win32_debug_time_marker
{
    DWORD OutputPlayCursor;
    DWORD OutputWriteCursor;
    DWORD OutputLocation;
    DWORD OutputByteCount;
    DWORD ExpectedFlipPlayCursor;
    
    DWORD FlipPlayCursor;
    DWORD FlipWriteCursor;
};

#define ScreenWidth GetSystemMetrics(SM_CXSCREEN)
#define ScreenHeight GetSystemMetrics(SM_CYSCREEN)

//
// Defaults for window setup
//

#ifndef WindowName
#define WindowName "qlib Application"
#endif

#ifndef ClientWidth
#define ClientWidth 1000
#endif

#ifndef ClientHeight
#define ClientHeight 1000
#endif

#ifndef Permanent_Storage_Size
#define Permanent_Storage_Size Megabytes(126);
#endif

#ifndef Transient_Storage_Size
#define Transient_Storage_Size Megabytes(1)
#endif

#ifndef IconFileName
#define IconFileName "icon.ico"
#endif

#ifndef CurrentDirectory
#define CurrentDirectory ""
#endif

#endif //WIN32_APPLICATION_H
