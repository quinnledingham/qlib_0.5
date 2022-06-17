// handle to the global opengl Vertex Array Object (VAO)
global_variable GLuint gVertexArrayObject = 0;

global_variable bool32 GlobalRunning;

// Multithreading
internal void
Win32AddEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    // TODO(casey): Switch to InterlockedCompareExchange eventually
    // so that any thread can add?
    uint32 NewNextEntryToWrite = (Queue->NextEntryToWrite + 1) % ArrayCount(Queue->Entries);
    Assert(NewNextEntryToWrite != Queue->NextEntryToRead);
    platform_work_queue_entry *Entry = Queue->Entries + Queue->NextEntryToWrite;
    Entry->Callback = Callback;
    Entry->Data = Data;
    ++Queue->CompletionGoal;
    _WriteBarrier();
    _mm_sfence();
    Queue->NextEntryToWrite = NewNextEntryToWrite;
    ReleaseSemaphore(Queue->SemaphoreHandle, 1, 0);
}

internal bool32
Win32DoNextWorkQueueEntry(platform_work_queue *Queue)
{
    bool32 WeShouldSleep = false;
    
    uint32 OriginalNextEntryToRead = Queue->NextEntryToRead;
    uint32 NewNextEntryToRead = (OriginalNextEntryToRead + 1) % ArrayCount(Queue->Entries);
    if(OriginalNextEntryToRead != Queue->NextEntryToWrite)
    {
        uint32 Index = InterlockedCompareExchange((LONG volatile *)&Queue->NextEntryToRead,
                                                  NewNextEntryToRead,
                                                  OriginalNextEntryToRead);
        if(Index == OriginalNextEntryToRead)
        {        
            platform_work_queue_entry Entry = Queue->Entries[Index];
            Entry.Callback(Queue, Entry.Data);
            InterlockedIncrement((LONG volatile *)&Queue->CompletionCount);
        }
    }
    else
    {
        WeShouldSleep = true;
    }
    
    return(WeShouldSleep);
}

internal void
Win32CompleteAllWork(platform_work_queue *Queue)
{
    while(Queue->CompletionGoal != Queue->CompletionCount)
    {
        Win32DoNextWorkQueueEntry(Queue);
    }
    
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}

internal void
Win32CancelAllWork(platform_work_queue *Queue)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}

struct win32_thread_info
{
    int LogicalThreadIndex;
    platform_work_queue *Queue;
};
DWORD WINAPI
ThreadProc(LPVOID lpParameter)
{
    win32_thread_info *ThreadInfo = (win32_thread_info *)lpParameter;
    
    for(;;)
    {
        if(Win32DoNextWorkQueueEntry(ThreadInfo->Queue))
        {
            WaitForSingleObjectEx(ThreadInfo->Queue->SemaphoreHandle, INFINITE, FALSE);
        }
    }
    
    //    return(0);
}

internal PLATFORM_WORK_QUEUE_CALLBACK(DoWorkerWork)
{
    char Buffer[256];
    wsprintf(Buffer, "Thread %u: %s\n", GetCurrentThreadId(), (char *)Data);
    OutputDebugStringA(Buffer);
}

// End of Multithreading

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
    //Assert(NewState->EndedDown != IsDown);
    if (IsDown && !NewState->EndedDown)
        NewState->NewEndedDown = true;
    else
        NewState->NewEndedDown = false;
    
    NewState->EndedDown = IsDown;
    //++NewState->HalfTransitionCount;
}

internal void
Win32ProcessPendingMessages(platform_controller_input *KeyboardController, platform_keyboard_input *Keyboard)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)Message.wParam;
                //bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                
                if(VKCode == 'W')
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
                }
                else if(VKCode == 'A')
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                }
                else if(VKCode == 'S')
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                }
                else if(VKCode == 'D')
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                }
                else if(VKCode == VK_F5)
                {
                    Win32ProcessKeyboardMessage(&Keyboard->F5, IsDown);
                }
                else if(VKCode == VK_UP)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown);
                }
                else if(VKCode == VK_LEFT)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                }
                else if(VKCode == VK_DOWN)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown);
                }
                else if(VKCode == VK_RIGHT)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                }
                else if(VKCode == VK_ESCAPE)
                {
                    Win32ProcessKeyboardMessage(&Keyboard->Escape, IsDown);
                }
                else if(VKCode >= '0' && VKCode <= '9')
                {
                    int index = VKCode - '0';
                    Win32ProcessKeyboardMessage(&Keyboard->Numbers[index], IsDown);
                }
                else if (VKCode == VK_OEM_PERIOD)
                {
                    Win32ProcessKeyboardMessage(&Keyboard->Period, IsDown);
                }
                else if (VKCode == VK_BACK)
                {
                    Win32ProcessKeyboardMessage(&Keyboard->Backspace, IsDown);
                }
                
                // alt-f4
                bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
                if((VKCode == VK_F4) && AltKeyWasDown)
                {
                    GlobalRunning = false;
                }
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
            //            OutputDebugStringA("default\n");
            Result = DefWindowProcA(Window, MESSAGE, WParam, LParam);
        } break;
    }
    return(Result);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    /*
    win32_thread_info ThreadInfo[7];
    
    platform_work_queue Queue = {};
    
    uint32 InitialCount = 0;
    uint32 ThreadCount = ArrayCount(ThreadInfo);
    Queue.SemaphoreHandle = CreateSemaphoreEx(0,
                                              InitialCount,
                                              ThreadCount,
                                              0, 0, SEMAPHORE_ALL_ACCESS);
    for(uint32 ThreadIndex = 0;
        ThreadIndex < ThreadCount;
        ++ThreadIndex)
    {
        win32_thread_info *Info = ThreadInfo + ThreadIndex;
        Info->Queue = &Queue;
        Info->LogicalThreadIndex = ThreadIndex;
        
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Info, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
    
    Win32AddEntry(&Queue, DoWorkerWork, "String A0");
    Win32AddEntry(&Queue, DoWorkerWork, "String A1");
    Win32AddEntry(&Queue, DoWorkerWork, "String A2");
    Win32AddEntry(&Queue, DoWorkerWork, "String A3");
    Win32AddEntry(&Queue, DoWorkerWork, "String A4");
    Win32AddEntry(&Queue, DoWorkerWork, "String A5");
    Win32AddEntry(&Queue, DoWorkerWork, "String A6");
    Win32AddEntry(&Queue, DoWorkerWork, "String A7");
    Win32AddEntry(&Queue, DoWorkerWork, "String A8");
    Win32AddEntry(&Queue, DoWorkerWork, "String A9");
    
    Win32AddEntry(&Queue, DoWorkerWork, "String B0");
    Win32AddEntry(&Queue, DoWorkerWork, "String B1");
    Win32AddEntry(&Queue, DoWorkerWork, "String B2");
    Win32AddEntry(&Queue, DoWorkerWork, "String B3");
    Win32AddEntry(&Queue, DoWorkerWork, "String B4");
    Win32AddEntry(&Queue, DoWorkerWork, "String B5");
    Win32AddEntry(&Queue, DoWorkerWork, "String B6");
    Win32AddEntry(&Queue, DoWorkerWork, "String B7");
    Win32AddEntry(&Queue, DoWorkerWork, "String B8");
    Win32AddEntry(&Queue, DoWorkerWork, "String B9");
    
    Win32CompleteAllWork(&Queue);
    */
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
    WindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WindowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
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
    
    
    Win32ResizeDIBSection(&GlobalBackbuffer, ClientWidth, ClientHeight);
    
    DWORD Style = (WS_OVERLAPPED | WS_CAPTION |
                   WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
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
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW
        | PFD_DOUBLEBUFFER;
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
        //std::cout << "Could not initialize GLAD\n";
    }
    else {
        //std::cout << "OpenGL Version " <<
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
    
    platform p = {};
    
#if QLIB_INTERNAL
    LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
    LPVOID BaseAddress = 0;
#endif
    
    p.Memory.PermanentStorageSize = Gigabytes(1);
    p.Memory.TransientStorageSize = Megabytes(1);
    //p.Memory.PermanentStorageSize = Megabytes(256);
    //p.Memory.TransientStorageSize = Gigabytes(1);
    
    // TODO(casey): Handle various memory footprints (USING SYSTEM METRICS)
    uint64 TotalSize = p.Memory.PermanentStorageSize + p.Memory.TransientStorageSize;
    p.Memory.PermanentStorage = VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    p.Memory.TransientStorage = ((uint8 *)p.Memory.PermanentStorage + p.Memory.PermanentStorageSize);
    
    if (p.Memory.PermanentStorage && p.Memory.TransientStorage)
    {
        platform_input Input[2] = {};
        platform_input *NewInput = &Input[0];
        platform_input *OldInput = &Input[1];
        
        // Gameplay Loop
        GlobalRunning = true;
        DWORD lastTick = GetTickCount();
        
        p.Initialized = false;
        p.Input.dt = 0;
        
        // Multithreading
        win32_thread_info ThreadInfo[7];
        
        uint32 InitialCount = 0;
        uint32 ThreadCount = ArrayCount(ThreadInfo);
        p.Queue.SemaphoreHandle = CreateSemaphoreEx(0, InitialCount, ThreadCount, 0, 0, SEMAPHORE_ALL_ACCESS);
        for(uint32 ThreadIndex = 0; ThreadIndex < ThreadCount; ++ThreadIndex) {
            win32_thread_info *Info = ThreadInfo + ThreadIndex;
            Info->Queue = &p.Queue;
            Info->LogicalThreadIndex = ThreadIndex;
            
            DWORD ThreadID;
            HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Info, 0, &ThreadID);
            CloseHandle(ThreadHandle);
        }
        // End of Multithreading
        
        //platform_controller_input *KeyboardController = &p.Input.Controllers[0];
        
        LARGE_INTEGER LastCounter =  Win32GetWallClock();
        
        while (GlobalRunning) {
            
            platform_controller_input *OldKeyboardController = GetController(OldInput, 0);
            platform_controller_input *NewKeyboardController = GetController(&p.Input, 0);
            *NewKeyboardController = {};
            NewKeyboardController->IsConnected = true;
            for(int ButtonIndex = 0;
                ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
                ++ButtonIndex)
            {
                NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                    OldKeyboardController->Buttons[ButtonIndex].EndedDown;
            }
            
            platform_keyboard_input *OldKeyboard = GetKeyboard(OldInput, 0);
            platform_keyboard_input *NewKeyboard = GetKeyboard(&p.Input, 0);
            *NewKeyboard = {};
            //NewKeyboardController->IsConnected = true;
            for(int ButtonIndex = 0;
                ButtonIndex < ArrayCount(NewKeyboard->Buttons);
                ++ButtonIndex)
            {
                NewKeyboard->Buttons[ButtonIndex].EndedDown =
                    OldKeyboard->Buttons[ButtonIndex].EndedDown;
            }
            Win32ProcessPendingMessages(NewKeyboardController, NewKeyboard);
            
            DWORD thisTick = GetTickCount();
            float dt = float(thisTick - lastTick) * 0.0001f;
            lastTick = thisTick;
            
            // PrintqDebug - DebugBuffer
            GlobalDebugBuffer= {};
            memset(GlobalDebugBuffer.Data, 0, GlobalDebugBuffer.Size);
            GlobalDebugBuffer.Next = GlobalDebugBuffer.Data;
            
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
            p.Input.MouseX = MouseP.x - (p.Dimension.Width / 2); // Move origin to middle of screen
            p.Input.MouseY = MouseP.y - (p.Dimension.Height / 2); 
            p.Input.MouseZ = 0; // TODO(casey): Support mousewheel?
            Win32ProcessKeyboardMessage(&p.Input.MouseButtons[0],
                                        GetKeyState(VK_LBUTTON) & (1 << 15));
            Win32ProcessKeyboardMessage(&p.Input.MouseButtons[1],
                                        GetKeyState(VK_MBUTTON) & (1 << 15));
            Win32ProcessKeyboardMessage(&p.Input.MouseButtons[2],
                                        GetKeyState(VK_RBUTTON) & (1 << 15));
            Win32ProcessKeyboardMessage(&p.Input.MouseButtons[3],
                                        GetKeyState(VK_XBUTTON1) & (1 << 15));
            Win32ProcessKeyboardMessage(&p.Input.MouseButtons[4],
                                        GetKeyState(VK_XBUTTON2) & (1 << 15));
            
            LARGE_INTEGER WorkCounter = Win32GetWallClock();
            p.Input.WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
            LastCounter =  Win32GetWallClock();
            p.Input.dt = dt;
            UpdateRender(&p);
            
            Win32ResizeDIBSection(&GlobalBackbuffer, p.Dimension.Width,
                                  p.Dimension.Height);
            
            if (p.Input.NewCursor)
            {
                if (p.Input.Cursor == Arrow)
                {
                    HCURSOR curs = LoadCursor(NULL, IDC_ARROW);
                    SetCursor(curs); 
                }
                else if (p.Input.Cursor == Hand)
                {
                    HCURSOR curs = LoadCursor(NULL, IDC_HAND);
                    SetCursor(curs); 
                }
            }
            
            char CharBuffer[OUTPUTBUFFER_SIZE];
            _snprintf_s(CharBuffer, sizeof(CharBuffer), "%s", GlobalDebugBuffer.Data);
            OutputDebugStringA(CharBuffer);
            
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
            
            platform_input *Temp = &p.Input;
            p.Input = *OldInput;
            OldInput = Temp;
        } // End of game loop
    }
    else // if (p.Memory.PermanentStorage && p.Memory.TransientStorage)
    {
        
    }
    
    return(0);
}
