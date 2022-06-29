global_variable bool32 GlobalRunning;

#ifndef WIN32_THREAD_H
#pragma message ("win32_application.cpp requires win32_thread.h")
#endif

// NOTE(casey): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(casey): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void 
Win32InitThreads(win32_thread_info *ThreadInfo, int InfoArrayCount, platform_work_queue *Queue)
{
    uint32 InitialCount = 0;
    uint32 ThreadCount = InfoArrayCount;
    Queue->SemaphoreHandle = CreateSemaphoreEx(0, InitialCount, ThreadCount, 0, 0, SEMAPHORE_ALL_ACCESS);
    for(uint32 ThreadIndex = 0; ThreadIndex < ThreadCount; ++ThreadIndex) {
        win32_thread_info *Info = ThreadInfo + ThreadIndex;
        Info->Queue = Queue;
        Info->LogicalThreadIndex = ThreadIndex;
        
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Info, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
}

global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    // NOTE(casey): Load the library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if(DSoundLibrary)
    {
        // NOTE(casey): Get a DirectSound object! - cooperative
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)
            GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        
        // TODO(casey): Double-check that this works on XP - DirectSound8 or 7??
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;
            
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                
                // NOTE(casey): "Create" a primary buffer
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if(SUCCEEDED(Error))
                    {
                        // NOTE(casey): We have finally set the format!
                        OutputDebugStringA("Primary buffer format was set.\n");
                    }
                    else
                    {
                        // TODO(casey): Diagnostic
                    }
                }
                else
                {
                    // TODO(casey): Diagnostic
                }
            }
            else
            {
                // TODO(casey): Diagnostic
            }
            
            // TODO(casey): In release mode, should we not specify DSBCAPS_GLOBALFOCUS?
            
            // TODO(casey): DSBCAPS_GETCURRENTPOSITION2
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
#if HANDMADE_INTERNAL
            BufferDescription.dwFlags |= DSBCAPS_GLOBALFOCUS;
#endif
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
            if(SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary buffer created successfully.\n");
            }
        }
        else
        {
            // TODO(casey): Diagnostic
        }
    }
    else
    {
        // TODO(casey): Diagnostic
    }
}

internal void
Win32ClearBuffer(win32_sound_output *SoundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize,&Region1, &Region1Size, &Region2, 
                                             &Region2Size, 0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid
        uint8 *DestSample = (uint8 *)Region1;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region1Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        DestSample = (uint8 *)Region2;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region2Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite,
                     platform_sound_output_buffer *SourceBuffer)
{
    // TODO(casey): More strenuous test!
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                                             &Region1, &Region1Size,
                                             &Region2, &Region2Size,
                                             0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid
        
        // TODO(casey): Collapse these two loops
        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *)Region1;
        int16 *SourceSample = SourceBuffer->Samples;
        for(DWORD SampleIndex = 0;
            SampleIndex < Region1SampleCount;
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        
        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        DestSample = (int16 *)Region2;
        for(DWORD SampleIndex = 0;
            SampleIndex < Region2SampleCount;
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32LoadXInput(void)    
{
    // TODO(casey): Test this on Windows 8
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) {XInputGetState = XInputGetStateStub;}
        
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) {XInputSetState = XInputSetStateStub;}
        
        // TODO(casey): Diagnostic
        
    }
    else
    {
        // TODO(casey): Diagnostic
    }
}

internal real32
Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold)
{
    real32 Result = 0;
    
    if(Value < -DeadZoneThreshold)
    {
        Result = (real32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    }
    else if(Value > DeadZoneThreshold)
    {
        Result = (real32)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }
    
    return(Result);
}


internal platform_window_dimension
Win32GetWindowDimension(HWND Window)
{
    platform_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    
    return(Result);
}

internal void
Win32ProcessKeyboardMessage(platform_button_state *NewState, bool32 IsDown)
{
    //fprintf(stderr, "%d %d\n", NewState->EndedDown, IsDown);
    if (IsDown) 
    {
        NewState->NewEndedUp = false;
        
        if (NewState->EndedDown != IsDown)
            NewState->NewEndedDown = true;
        else
            NewState->NewEndedDown = false;
    }
    else if (!IsDown)
    {
        NewState->NewEndedDown = false;
        
        if (NewState->EndedDown != IsDown)
            NewState->NewEndedUp = true;
        else
            NewState->NewEndedUp = false;
    }
    
    
    NewState->EndedDown = IsDown;
}

internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState, DWORD ButtonBit, platform_button_state *NewState)
{
    bool32 IsDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    Win32ProcessKeyboardMessage(NewState, IsDown);
}


internal void
Win32ProcessPendingMessages(platform_keyboard_input *Keyboard, platform_mouse_input *Mouse)
{
    MSG Message;
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch (Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case WM_KEYUP:
            case WM_KEYDOWN:
            {
                uint32 VKCode = (uint32)Message.wParam;
                bool32 WasDown = ((Message.lParam & (1 << 30)) != 0); // The previous key state. The value is 1 if the key is down before the message is sent, or it is zero if the key is up.
                bool32 IsDown = ((Message.lParam & (1 << 31)) == 0); // Always 0 for WM_KEYDOWN
                
                if (WasDown != IsDown)
                {
                    if (VKCode == 'W')
                        Win32ProcessKeyboardMessage(&Keyboard->ControllerInput->MoveUp, IsDown);
                    else if (VKCode == 'A')
                        Win32ProcessKeyboardMessage(&Keyboard->ControllerInput->MoveLeft, IsDown);
                    else if (VKCode == 'S')
                        Win32ProcessKeyboardMessage(&Keyboard->ControllerInput->MoveDown, IsDown);
                    else if (VKCode == 'D')
                        Win32ProcessKeyboardMessage(&Keyboard->ControllerInput->MoveRight, IsDown);
                    else if(VKCode == VK_UP)
                        Win32ProcessKeyboardMessage(&Keyboard->Up, IsDown);
                    else if(VKCode == VK_LEFT)
                        Win32ProcessKeyboardMessage(&Keyboard->Left, IsDown);
                    else if(VKCode == VK_DOWN)
                        Win32ProcessKeyboardMessage(&Keyboard->Down, IsDown);
                    else if(VKCode == VK_RIGHT)
                        Win32ProcessKeyboardMessage(&Keyboard->Right, IsDown);
                    else if(VKCode == VK_F5)
                        Win32ProcessKeyboardMessage(&Keyboard->F5, IsDown);
                    else if(VKCode == VK_F6)
                        Win32ProcessKeyboardMessage(&Keyboard->F6, IsDown);
                    else if(VKCode == VK_ESCAPE)
                        Win32ProcessKeyboardMessage(&Keyboard->ControllerInput->Start, IsDown);
                    else if(VKCode == VK_OEM_PERIOD)
                        Win32ProcessKeyboardMessage(&Keyboard->Period, IsDown);
                    else if(VKCode == VK_BACK)
                        Win32ProcessKeyboardMessage(&Keyboard->Backspace, IsDown);
                    else if(VKCode == VK_TAB)
                        Win32ProcessKeyboardMessage(&Keyboard->Tab, IsDown);
                    else if(VKCode == VK_RETURN)
                        Win32ProcessKeyboardMessage(&Keyboard->ControllerInput->Enter, IsDown);
                    else if(VKCode >= '0' && VKCode <= '9')
                    {
                        int index = VKCode - '0';
                        Win32ProcessKeyboardMessage(&Keyboard->Numbers[index], IsDown);
                    }
                    
                    // alt-f4
                    bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
                    if((VKCode == VK_F4) && AltKeyWasDown)
                    {
                        GlobalRunning = false;
                    }
                    
                    bool32 CrtlKeyWasDown = (Message.lParam & (1 << 17));
                    if ((VKCode == 'V') && CrtlKeyWasDown)
                    {
                        HGLOBAL hglb; 
                        OpenClipboard(0);
                        hglb = GetClipboardData(CF_TEXT);
                        if (hglb != 0) {
                            memcpy(&Keyboard->Clipboard, hglb, (int)GlobalSize(hglb));
                            Win32ProcessKeyboardMessage(&Keyboard->CtrlV, IsDown);
                        }
                        CloseClipboard();
                    }
                }
                
            } break;
            
            case WM_LBUTTONDOWN:
            {
                Win32ProcessKeyboardMessage(&Mouse->Left, true);
            } break;
            
            case WM_LBUTTONUP:
            {
                Win32ProcessKeyboardMessage(&Mouse->Left, false);
            } break;
            
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

internal void
Win32ResizeDIBSection(platform_offscreen_buffer *Buffer, int Width, int Height)
{
    // TODO(casey): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.
    
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    
    int BytesPerPixel = 4;
    Buffer->BytesPerPixel = BytesPerPixel;
    
    // NOTE(casey): When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    // NOTE(casey): Thank you to Chris Hecker of Spy Party fame
    // for clarifying the deal with StretchDIBits and BitBlt!
    // No more DC for us.
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width*BytesPerPixel;
    
    // TODO(casey): Probably clear this to black
}

global_variable int64 GlobalPerfCountFrequency;

inline LARGE_INTEGER
Win32GetWallClock(void)
{    
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline real32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    real32 Result = ((real32)(End.QuadPart - Start.QuadPart) /
                     (real32)GlobalPerfCountFrequency);
    return(Result);
}

internal void
Win32DisplayBufferInWindow(platform_offscreen_buffer *Buffer, HDC DeviceContext, 
                           int WindowWidth, int WindowHeight)
{
    if((WindowWidth >= Buffer->Width*2) &&
       (WindowHeight >= Buffer->Height*2))
    {
        StretchDIBits(DeviceContext,
                      0, 0, 2*Buffer->Width, 2*Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
    else
    {
        int OffsetX = 0;
        int OffsetY = 0;
        
        // NOTE(casey): For prototyping purposes, we're going to always blit
        // 1-to-1 pixels to make sure we don't introduce artifacts with
        // stretching while we are learning to code the renderer!
        StretchDIBits(DeviceContext,
                      OffsetX, OffsetY, Buffer->Width, Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
}

#ifdef QLIB_WINDOW_APPLICATION

internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window, UINT MESSAGE, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    switch(MESSAGE)
    {
        case WM_CLOSE:
        {
            // TODO(casey): Handle this with a message to the user?
            GlobalRunning = false;
        } break;
        
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        
        case WM_DESTROY:
        {
            if (gVertexArrayObject != 0) {
                HDC hdc = GetDC(Window);
                HGLRC hglrc = wglGetCurrentContext();
                
                glBindVertexArray(0);
                glDeleteVertexArrays(1, &gVertexArrayObject);
                gVertexArrayObject = 0;
                
                wglMakeCurrent(NULL, NULL);
                wglDeleteContext(hglrc);
                ReleaseDC(Window, hdc);
                
                PostQuitMessage(0);
            }
            else
            {
                
            }
            // TODO(casey): Handle this as an error - recreate window?
            GlobalRunning = false;
        } break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(!"Keyboard input came in through a non-dispatch message!");
        } break;
        
        case WM_PAINT:
        case WM_ERASEBKGND:
        default:
        {
            // OutputDebugStringA("default\n");
            Result = DefWindowProcA(Window, MESSAGE, WParam, LParam);
        } break;
    }
    return(Result);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    SetCurrentDirectory("../game/data");
    
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    WNDCLASSEX WindowClass = {};
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = hInstance;
    
    WindowClass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    WindowClass.hIconSm = (HICON)LoadImage(hInstance, "icon.ico", IMAGE_ICON, 100, 100, 
                                           LR_LOADFROMFILE | LR_LOADTRANSPARENT);
    
    //WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    WindowClass.lpszMenuName = 0;
    WindowClass.lpszClassName = "Win32 Game Window";
    RegisterClassEx(&WindowClass);
    
    // Center window on screen
    int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    int ClientWidth = 1000;
    int ClientHeight = 1000;
    RECT WindowRect;
    SetRect(&WindowRect,
            (ScreenWidth / 2) - (ClientWidth / 2),
            (ScreenHeight / 2) - (ClientHeight / 2),
            (ScreenWidth / 2) + (ClientWidth / 2),
            (ScreenHeight / 2) + (ClientHeight / 2));
    
    Win32LoadXInput();
    
    Win32ResizeDIBSection(&GlobalBackbuffer, ClientWidth, ClientHeight);
    
    DWORD Style = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
    // | WS_THICKFRAME to resize
    
    AdjustWindowRectEx(&WindowRect, Style, FALSE, 0);
    HWND hwnd = CreateWindowEx(0, WindowClass.lpszClassName,
                               "Coffee Cow", Style, WindowRect.left,
                               WindowRect.top, WindowRect.right -
                               WindowRect.left, WindowRect.bottom -
                               WindowRect.top, NULL, NULL,
                               hInstance, szCmdLine);
    HDC hdc = GetDC(hwnd);
    
    // Setting the pixel format
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);
    
    // Temporary OpenGL context to get a pointer to wglCreateContext
    HGLRC tempRC = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempRC);
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    
    // Getting OpenGL 4.6 Core context profile
    const int attribList[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_FLAGS_ARB, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0, };
    HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribList);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempRC);
    wglMakeCurrent(hdc, hglrc);
    
    // Loading OpenGL 4.6 Core functions with glad
    if (!gladLoadGL()) {
        PrintqDebug("Could not initialize GLAD\n");
    }
    else {
        PrintqDebug("OpenGL Version \n");//" + GLVersion.major + "." + GLVersion.minor + "\n");
        //std::cout <<  <<
        //GLVersion.major << "." << GLVersion.minor <<
        //"\n";
    }
    
    int vsynch = 0;
#if VSYNC
    // Enabling vsync
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = 
    (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    bool swapControlSupported = strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") != 0;
    
    if (swapControlSupported)
    {
        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = 
        (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
        
        if (wglSwapIntervalEXT(1))
        {
            OutputDebugStringA("Enabled vsynch\n");
            vsynch = wglGetSwapIntervalEXT();
        }
        else
        {
            OutputDebugStringA("Could not enable vsynch\n");
        }
    }
    else
    { 
        // !swapControlSupported
        OutputDebugStringA("WGL_EXIT_swap_control not supported\n");
    }
#endif
    
    // Getting the global VAO
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    // NOTE(casey): Set the Windows scheduler granularity to 1ms
    // so that our Sleep() can be more granular.
    UINT DesiredSchedulerMS = 1;
    bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    
    win32_sound_output SoundOutput = {};
    
    // TODO(casey): How do we reliably query on this on Windows?
    int MonitorRefreshHz = 165;
    HDC RefreshDC = GetDC(hwnd);
    int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
    ReleaseDC(hwnd, RefreshDC);
    if(Win32RefreshRate > 1)
    {
        MonitorRefreshHz = Win32RefreshRate;
    }
    real32 GameUpdateHz = (real32)(MonitorRefreshHz / 2.0f);
    real32 TargetSecondsPerFrame = 1.0f / (real32)GameUpdateHz;
    
    // TODO(casey): Make this like sixty seconds?
    SoundOutput.SamplesPerSecond = 48000;
    SoundOutput.BytesPerSample = sizeof(int16)*2;
    SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
    // TODO(casey): Actually compute this variance and see
    // what the lowest reasonable value is.
    SoundOutput.SafetyBytes = (int)(((real32)SoundOutput.SamplesPerSecond*(real32)SoundOutput.BytesPerSample / GameUpdateHz)/3.0f);
    Win32InitDSound(hwnd, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
    Win32ClearBuffer(&SoundOutput);
    GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
    
    // TODO(casey): Pool with bitmap VirtualAlloc
    // TODO(casey): Remove MaxPossibleOverrun?
    u32 MaxPossibleOverrun = 2*8*sizeof(u16);
    int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize + MaxPossibleOverrun, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    platform p = {};
    
    p.SoundOutputBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
    p.SoundOutputBuffer.Samples = Samples;
    
#if QLIB_INTERNAL
    LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
    LPVOID BaseAddress = 0;
#endif
    
    p.Memory.PermanentStorageSize = Megabytes(126);
    //p.Memory.PermanentStorageSize = Gigabytes(1);
    p.Memory.TransientStorageSize = Megabytes(1);
    //p.Memory.PermanentStorageSize = Megabytes(256);
    //p.Memory.TransientStorageSize = Gigabytes(1);
    
    // TODO(casey): Handle various memory footprints (USING SYSTEM METRICS)
    uint64 TotalSize = p.Memory.PermanentStorageSize + p.Memory.TransientStorageSize;
    p.Memory.PermanentStorage = VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    p.Memory.TransientStorage = ((uint8 *)p.Memory.PermanentStorage + p.Memory.PermanentStorageSize);
    
    GlobalDebugBuffer.Mutex = CreateMutex(NULL, FALSE, NULL);
    Manager.Mutex = CreateMutex(NULL, FALSE, NULL);
    
    if (p.Memory.PermanentStorage && p.Memory.TransientStorage)
    {
        p.Input.Keyboard.ControllerInput = &p.Input.Controllers[0];
        
        // Gameplay Loop
        GlobalRunning = true;
        DWORD lastTick = GetTickCount();
        
        p.Initialized = false;
        
        // Multitheading
        win32_thread_info ThreadInfo[7];
        Win32InitThreads(ThreadInfo, ArrayCount(ThreadInfo), &p.Queue);
        
        LARGE_INTEGER LastCounter =  Win32GetWallClock();
        LARGE_INTEGER FlipWallClock = Win32GetWallClock();
        
        int DebugTimeMarkerIndex = 0;
        win32_debug_time_marker DebugTimeMarkers[30] = {0};
        DWORD AudioLatencyBytes = 0;
        real32 AudioLatencySeconds = 0;
        bool32 SoundIsValid = false;
        
        while (GlobalRunning) {
            
            Win32ProcessPendingMessages(&p.Input.Keyboard, &p.Input.Mouse);
            
#if QLIB_INTERNAL
            switch(WaitForSingleObject(GlobalDebugBuffer.Mutex, INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    // PrintqDebug - DebugBuffer
                    memset(GlobalDebugBuffer.Data, 0, GlobalDebugBuffer.MaxSize);
                    GlobalDebugBuffer.Next = GlobalDebugBuffer.Data;
                    GlobalDebugBuffer.Size = 0;
                }
                _finally{if(!ReleaseMutex(GlobalDebugBuffer.Mutex)){}}break;case WAIT_ABANDONED:return false;
            }
#endif
            platform_offscreen_buffer Buffer = {};
            Buffer.Memory = GlobalBackbuffer.Memory;
            Buffer.Width = GlobalBackbuffer.Width;
            Buffer.Height = GlobalBackbuffer.Height;
            Buffer.Pitch = GlobalBackbuffer.Pitch;
            Buffer.BytesPerPixel = GlobalBackbuffer.BytesPerPixel;
            
            p.Dimension = Win32GetWindowDimension(hwnd);
            
            // Mouse
            POINT MouseP;
            GetCursorPos(&MouseP);
            ScreenToClient(hwnd, &MouseP);
            
            LONG NewMouseX = MouseP.x - (p.Dimension.Width / 2); // Move origin to middle of screen;
            LONG NewMouseY = MouseP.y - (p.Dimension.Height / 2);
            if (NewMouseX != p.Input.Mouse.X || NewMouseY != p.Input.Mouse.Y) {
                p.Input.Mouse.X = NewMouseX;
                p.Input.Mouse.Y = NewMouseY;
                p.Input.Mouse.Moved = true;
            }
            else {
                p.Input.Mouse.Moved = false;
            }
            p.Input.Mouse.Z = 0; // TODO(casey): Support mousewheel?
            
            // TODO(casey): Need to not poll disconnected controllers to avoid
            // xinput frame rate hit on older libraries...
            // TODO(casey): Should we poll this more frequently
            DWORD MaxControllerCount = XUSER_MAX_COUNT;
            if(MaxControllerCount > (ArrayCount(p.Input.Controllers) - 1))
            {
                MaxControllerCount = (ArrayCount(p.Input.Controllers) - 1);
            }
            
            for (DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex)
            {
                DWORD OurControllerIndex = ControllerIndex + 1;
                platform_controller_input *Controller = GetController(&p.Input, OurControllerIndex);
                //platform_controller_input *NewController = GetController(NewInput, OurControllerIndex);
                
                XINPUT_STATE ControllerState;
                if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                {
                    Controller->IsConnected = true;
                    //Controller->IsAnalog = OldController->IsAnalog;
                    
                    // NOTE(casey): This controller is plugged in
                    // TODO(casey): See if ControllerState.dwPacketNumber increments too rapidly
                    XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
                    
                    // TODO(casey): This is a square deadzone, check XInput to
                    // verify that the deadzone is "round" and show how to do
                    // round deadzone processing.
                    Controller->StickAverageX = Win32ProcessXInputStickValue(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                    Controller->StickAverageY = Win32ProcessXInputStickValue(Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                    
                    if((Controller->StickAverageX != 0.0f) ||
                       (Controller->StickAverageY != 0.0f))
                    {
                        Controller->IsAnalog = true;
                    }
                    
                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
                    {
                        Controller->StickAverageY = 1.0f;
                        Controller->IsAnalog = false;
                    }
                    
                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                    {
                        Controller->StickAverageY = -1.0f;
                        Controller->IsAnalog = false;
                    }
                    
                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                    {
                        Controller->StickAverageX = -1.0f;
                        Controller->IsAnalog = false;
                    }
                    
                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                    {
                        Controller->StickAverageX = 1.0f;
                        Controller->IsAnalog = false;
                    }
                    
                    real32 Threshold = 0.5f;
                    Win32ProcessXInputDigitalButton((Controller->StickAverageX < -Threshold) ? 1 : 0, 1, &Controller->MoveLeft);
                    Win32ProcessXInputDigitalButton((Controller->StickAverageX > Threshold) ? 1 : 0, 1, &Controller->MoveRight);
                    Win32ProcessXInputDigitalButton((Controller->StickAverageY < -Threshold) ? 1 : 0, 1, &Controller->MoveDown);
                    Win32ProcessXInputDigitalButton((Controller->StickAverageY > Threshold) ? 1 : 0, 1, &Controller->MoveUp);
                    
                    Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_A, &Controller->Enter);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_B, &Controller->ActionRight);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_X, &Controller->ActionLeft);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_Y, &Controller->ActionUp);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, &Controller->LeftShoulder);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, &Controller->RightShoulder);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_START, &Controller->Start);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_BACK, &Controller->Back);
                }
                else
                {
                    // NOTE(casey): The controller is not available
                    Controller->IsConnected = false;
                }
            }
            
            LARGE_INTEGER WorkCounter = Win32GetWallClock();
            p.Input.WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
            LastCounter =  Win32GetWallClock();
            
            if (p.Dimension.Width != 0 && p.Dimension.Height != 0)
                UpdateRender(&p);
            
            if (p.Input.Mouse.NewCursor)
            {
                if (p.Input.Mouse.Cursor == platform_cursor_mode::Arrow)
                {
                    HCURSOR curs = LoadCursor(NULL, IDC_ARROW);
                    SetCursor(curs); 
                }
                else if (p.Input.Mouse.Cursor == platform_cursor_mode::Hand)
                {
                    HCURSOR curs = LoadCursor(NULL, IDC_HAND);
                    SetCursor(curs); 
                }
            }
            
            BEGIN_BLOCK(AudioUpdate);
            
            LARGE_INTEGER AudioWallClock = Win32GetWallClock();
            real32 FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);
            
            DWORD PlayCursor;
            DWORD WriteCursor;
            if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
            {
                /* NOTE(casey):
    
                                  Here is how sound output computation works.
    
                                  We define a safety value that is the number
                                  of samples we think our game update loop
                                  may vary by (let's say up to 2ms)
                          
                                  When we wake up to write audio, we will look
                                  and see what the play cursor position is and we
                                  will forecast ahead where we think the play
                                  cursor will be on the next frame boundary.
    
                                  We will then look to see if the write cursor is
                                  before that by at least our safety value.  If
                                  it is, the target fill position is that frame
                                  boundary plus one frame.  This gives us perfect
                                  audio sync in the case of a card that has low
                                  enough latency.
    
                                  If the write cursor is _after_ that safety
                                  margin, then we assume we can never sync the
                                  audio perfectly, so we will write one frame's
                                  worth of audio plus the safety margin's worth
                                  of guard samples.
                               */
                if(!SoundIsValid)
                {
                    SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
                    SoundIsValid = true;
                }
                
                DWORD ByteToLock = ((SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize);
                
                DWORD ExpectedSoundBytesPerFrame = (int)((real32)(SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample) / GameUpdateHz);
                real32 SecondsLeftUntilFlip = (TargetSecondsPerFrame - FromBeginToAudioSeconds);
                DWORD ExpectedBytesUntilFlip = (DWORD)((SecondsLeftUntilFlip/TargetSecondsPerFrame)*(real32)ExpectedSoundBytesPerFrame);
                
                DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;
                
                DWORD SafeWriteCursor = WriteCursor;
                if(SafeWriteCursor < PlayCursor)
                {
                    SafeWriteCursor += SoundOutput.SecondaryBufferSize;
                }
                Assert(SafeWriteCursor >= PlayCursor);
                SafeWriteCursor += SoundOutput.SafetyBytes;
                
                bool32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);                        
                
                DWORD TargetCursor = 0;
                if(AudioCardIsLowLatency)
                {
                    TargetCursor = (ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame);
                }
                else
                {
                    TargetCursor = (WriteCursor + ExpectedSoundBytesPerFrame + SoundOutput.SafetyBytes);
                }
                TargetCursor = (TargetCursor % SoundOutput.SecondaryBufferSize);
                
                DWORD BytesToWrite = 0;
                if(ByteToLock > TargetCursor)
                {
                    BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                    BytesToWrite += TargetCursor;
                }
                else
                {
                    BytesToWrite = TargetCursor - ByteToLock;
                }
                
                platform_sound_output_buffer SoundBuffer = {};
                SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                SoundBuffer.Samples = Samples;
                game_state *GameState = (game_state *)p.Memory.PermanentStorage;
                //OutputTestSineWave(GameState, &SoundBuffer, 256);
                
                int16 *SampleOut = SoundBuffer.Samples;
                for(int SampleIndex = 0; SampleIndex < SoundBuffer.SampleCount; ++SampleIndex)
                {
                    uint32 TestSoundSampleIndex = (GameState->TestSampleIndex + SampleIndex) % GameState->TestSound.SampleCount;
                    int16 SampleValue = GameState->TestSound.Samples[0][TestSoundSampleIndex];
                    *SampleOut++ = SampleValue;
                    *SampleOut++ = SampleValue;
                }
                
                GameState->TestSampleIndex += SoundBuffer.SampleCount;
                
#if QLIB_INTERNAL
                win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
                Marker->OutputPlayCursor = PlayCursor;
                Marker->OutputWriteCursor = WriteCursor;
                Marker->OutputLocation = ByteToLock;
                Marker->OutputByteCount = BytesToWrite;
                Marker->ExpectedFlipPlayCursor = ExpectedFrameBoundaryByte;
                
                DWORD UnwrappedWriteCursor = WriteCursor;
                if(UnwrappedWriteCursor < PlayCursor)
                {
                    UnwrappedWriteCursor += SoundOutput.SecondaryBufferSize;
                }
                AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
                AudioLatencySeconds = (((real32)AudioLatencyBytes / (real32)SoundOutput.BytesPerSample) / (real32)SoundOutput.SamplesPerSecond);
                
#if 0
                char TextBuffer[256];
                _snprintf_s(TextBuffer, sizeof(TextBuffer), "BTL:%u TC:%u BTW:%u - PC:%u WC:%u DELTA:%u (%fs)\n",
                            ByteToLock, TargetCursor, BytesToWrite, PlayCursor, WriteCursor, AudioLatencyBytes, 
                            AudioLatencySeconds);
                OutputDebugStringA(TextBuffer);
#endif
#endif   
                Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
            }
            END_BLOCK(AudioUpdate);
            
#if QLIB_INTERNAL
            switch(WaitForSingleObject(GlobalDebugBuffer.Mutex, INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    char CharBuffer[OUTPUTBUFFER_SIZE];
                    _snprintf_s(CharBuffer, sizeof(CharBuffer), "%s", GlobalDebugBuffer.Data);
                    OutputDebugStringA(CharBuffer);
                }
                _finally{if(!ReleaseMutex(GlobalDebugBuffer.Mutex)){}}break;case WAIT_ABANDONED:return false;
            }
#endif
            if (p.Input.Quit)
            {
                GlobalRunning = false;
            }
            
#if QLIB_OPENGL
            SwapBuffers(hdc);
            if (vsynch != 0) {
                glFinish();
            }
#else
            Win32DisplayBufferInWindow(&GlobalBackbuffer, hdc, p.Dimension.Width, p.Dimension.Height);
            if((GlobalBackbuffer.Width != p.Dimension.Width) || (GlobalBackbuffer.Height != p.Dimension.Height))
            {
                Win32ResizeDIBSection(&GlobalBackbuffer, p.Dimension.Width, p.Dimension.Height);
            }
#endif
            
#if 0
            BEGIN_BLOCK(FramerateWait);
            
            LARGE_INTEGER WorkCounter2 = Win32GetWallClock();
            real32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter2);
            
            // TODO(casey): NOT TESTED YET!  PROBABLY BUGGY!!!!!
            real32 SecondsElapsedForFrame = WorkSecondsElapsed;
            if(SecondsElapsedForFrame < TargetSecondsPerFrame)
            {                        
                if(SleepIsGranular)
                {
                    DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame -
                                                       SecondsElapsedForFrame));
                    if(SleepMS > 0)
                    {
                        Sleep(SleepMS);
                    }
                }
                
                real32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                           Win32GetWallClock());
                if(TestSecondsElapsedForFrame < TargetSecondsPerFrame)
                {
                    // TODO(casey): LOG MISSED SLEEP HERE
                }
                
                while(SecondsElapsedForFrame < TargetSecondsPerFrame)
                {                            
                    SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                    Win32GetWallClock());
                }
            }
            else
            {
                // TODO(casey): MISSED FRAME RATE!
                // TODO(casey): Logging
            }
            
            END_BLOCK(FramerateWait);
#endif
            
            FlipWallClock = Win32GetWallClock();
        } // End of game loop
    }
    else // if (p.Memory.PermanentStorage && p.Memory.TransientStorage)
    {
        
    }
    
    return(0);
}
#endif // QLIB_WINDOW_APPLICATION

#ifdef QLIB_CONSOLE_APPLICATION
#pragma comment(linker, "/subsystem:console")
int main(int argc, const char** argv)
{
    platform p = {};
    
#if QLIB_INTERNAL
    LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
    LPVOID BaseAddress = 0;
#endif
    
    p.Memory.PermanentStorageSize = Megabytes(100);
    p.Memory.TransientStorageSize = Megabytes(1);
    
    // TODO(casey): Handle various memory footprints (USING SYSTEM METRICS)
    uint64 TotalSize = p.Memory.PermanentStorageSize + p.Memory.TransientStorageSize;
    p.Memory.PermanentStorage = VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    p.Memory.TransientStorage = ((uint8 *)p.Memory.PermanentStorage + p.Memory.PermanentStorageSize);
    
    if (p.Memory.PermanentStorage && p.Memory.TransientStorage)
    {
        win32_thread_info ThreadInfo[7];
        Win32InitThreads(ThreadInfo, ArrayCount(ThreadInfo), &p.Queue);
        
        while(1)
        {
            Update(&p);
            
            char CharBuffer[OUTPUTBUFFER_SIZE];
            _snprintf_s(CharBuffer, sizeof(CharBuffer), "%s", GlobalDebugBuffer.Data);
            OutputDebugStringA(CharBuffer);
            
            GlobalDebugBuffer = {};
            memset(GlobalDebugBuffer.Data, 0, GlobalDebugBuffer.Size);
            GlobalDebugBuffer.Next = GlobalDebugBuffer.Data;
        }
    } // p.Memory.PermanentStorage && p.Memory.TransientStorage
    return (0);
}
#endif // QLIB_CONSOLE_APPLICATION


