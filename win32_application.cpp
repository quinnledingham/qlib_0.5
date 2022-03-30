// handle to the global opengl Vertex Array Object (VAO)
global_variable GLuint gVertexArrayObject = 0;

global_variable bool32 GlobalRunning;

internal void
Win32ProcessKeyboardMessage(platform_button_state *NewState, bool32 IsDown)
{
    //Assert(NewState->EndedDown != IsDown);
    if (IsDown == (bool32)true && NewState->EndedDown == (bool32)false)
    {
        NewState->NewEndedDown = true;
    }
    else
    {
        NewState->NewEndedDown = false;
    }
    
    NewState->EndedDown = IsDown;
    //++NewState->HalfTransitionCount;
}

internal void
Win32ProcessPendingMessages(platform_controller_input *KeyboardController)
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
    WNDCLASSEX WindowClass = {};
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = hInstance;
    WindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WindowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    WindowClass.lpszMenuName = 0;
    WindowClass.lpszClassName = "Win32 Game Window";
    RegisterClassEx(&WindowClass);
    
    // Center window on screen
    int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    int ClientWidth = 800;
    int ClientHeight = 600;
    RECT WindowRect;
    SetRect(&WindowRect,
            (ScreenWidth / 2) - (ClientWidth / 2),
            (ScreenHeight / 2) - (ClientHeight / 2),
            (ScreenWidth / 2) + (ClientWidth / 2),
            (ScreenHeight / 2) + (ClientHeight / 2));
    
    DWORD Style = (WS_OVERLAPPED | WS_CAPTION |
                   WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    // | WS_THICKFRAME to resize
    
    AdjustWindowRectEx(&WindowRect, Style, FALSE, 0);
    HWND hwnd = CreateWindowEx(0, WindowClass.lpszClassName,
                               "Game Window", Style, WindowRect.left,
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
    
    // Enabling vsync
    PFNWGLGETEXTENSIONSSTRINGEXTPROC
        _wglGetExtensionsStringEXT =
    (PFNWGLGETEXTENSIONSSTRINGEXTPROC)
        wglGetProcAddress("wglGetExtensionsStringEXT");
    bool swapControlSupported = strstr(_wglGetExtensionsStringEXT(),
                                       "WGL_EXT_swap_control") != 0;
    
    int vsynch = 0;
    
    if (swapControlSupported) {
        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
        (PFNWGLSWAPINTERVALEXTPROC)
            wglGetProcAddress("wglSwapIntervalEXT");
        PFNWGLGETSWAPINTERVALEXTPROC
            wglGetSwapIntervalEXT =
        (PFNWGLGETSWAPINTERVALEXTPROC)
            wglGetProcAddress("wglGetSwapIntervalEXT");
        if (wglSwapIntervalEXT(1)) {
            //std::cout << "Enabled vsynch\n";
            vsynch = wglGetSwapIntervalEXT();
        }
        else {
            //std::cout << "Could not enable vsynch\n";
        }
    }
    else { // !swapControlSupported
        //cout << "WGL_EXT_swap_control not supported\n";
    }
    
    // Getting the global VAO
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    // Gameplay Loop
    GlobalRunning = true;
    DWORD lastTick = GetTickCount();
    
    platform p = {};
    p.Initialized = false;
    p.Input.dt = 0;
    
    platform_controller_input *KeyboardController = &p.Input.Controllers[0];
    
    while (GlobalRunning) {
        
        Win32ProcessPendingMessages(KeyboardController);
        
        DWORD thisTick = GetTickCount();
        float dt = float(thisTick - lastTick) * 0.0001f;
        lastTick = thisTick;
        
        debug = {};
        memset(debug.Data, 0, debug.Size);
        debug.Next = debug.Data;
        
        p.Input.dt = dt;
        Update(&p);
        
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        ClientWidth = clientRect.right -
            clientRect.left;
        ClientHeight = clientRect.bottom -
            clientRect.top;
        
        glViewport(0, 0, ClientWidth, ClientHeight);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glPointSize(5.0f);
        glBindVertexArray(gVertexArrayObject);
        glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        float aspect = (float)ClientWidth /
        (float)ClientHeight;
        
        Render(aspect);
        
        char CharBuffer[OUTPUTBUFFER_SIZE];
        _snprintf_s(CharBuffer, sizeof(CharBuffer), "%s", debug.Data);
        OutputDebugStringA(CharBuffer);
        
        SwapBuffers(hdc);
        if (vsynch != 0) {
            glFinish();
        }
    } // End of game loop
    
    return(0);
}
