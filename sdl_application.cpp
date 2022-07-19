global_variable bool32 GlobalRunning;

struct sdl
{
    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDL_GLContext Context;
};

struct callback_data
{
    platform *p;
    sdl *SDL;
};

EM_BOOL emscripten_window_resized_callback(int eventType, const void *reserved, void *userData){
	//METHOD();
    
    callback_data *Data = (callback_data*)userData;
    
	double width, height;
	emscripten_get_element_css_size("canvas", &width, &height);
    
	int w = (int)width, h = (int)height;
    
    Data->p->Dimension.Width = w;
    Data->p->Dimension.Height = h;
    
	// resize SDL window
	//Platform* platform = (Platform*)userData;
	//SDL_SetWindowSize((SDL_Window*)platform->get_wnd_handle(), w, h);
    SDL_SetWindowSize(Data->SDL->Window, w, h);
    
	// engine-specific code - internal render size should be updated here
	//event_t e(ET_WINDOW_SIZE, w, h, timer::current_time());
	//LOGI("Window resized to %dx%d", w, h);
	//platform->on_window_size_changed(w, h);
	return true;
}

internal void
SDLProcessPendingEvents()
{
    SDL_Event Event;
    
    while (SDL_PollEvent(&Event))
    {
        switch (Event.type)
        {
            case SDL_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case SDL_WINDOWEVENT:
            {
                switch (Event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        //update_screen_size(Event.window.data1, Event.window.data2);
                        
                    } break;
                    
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        printf("SDL_WINDOWEVENT_SIZE_CHANGED (%d, %d)\n", Event.window.data1, Event.window.data2);
                    } break;
                }
                
            } break;
            
            default: break;
        }
    }
}

void MainLoop()
{
    //PlatformSetCD(CurrentDirectory);
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO);
    
    // Scuffed way of setting screen size
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    int SDLScreenWidth = DM.w;
    int SDLScreenHeight = DM.h;
    int SDLClientWidth = (int)ClientWidth;
    int SDLClientHeight = (int)ClientHeight;
    
    if (ClientWidth < 1) SDLClientWidth = (int)(ClientWidth * SDLScreenWidth);
    if (ClientHeight < 1) SDLClientHeight = (int)(ClientHeight * SDLScreenHeight);
    
    if (ClientWidth == 0) SDLClientWidth = SDLClientHeight;
    if (ClientHeight == 0) SDLClientHeight = SDLClientWidth;
    
    sdl SDL = {};
    
#if QLIB_OPENGL
    SDL_GL_LoadLibrary(NULL);
    
    // Request an OpenGL 4.5 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL.Window = SDL_CreateWindow(WindowName, 
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                  SDLClientWidth, SDLClientHeight, 
                                  SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    
    SDL.Context = SDL_GL_CreateContext(SDL.Window);
    
    // Check OpenGL properties
    printf("OpenGL loaded\n");
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    
    //glGenVertexArrays(1, &gVertexArrayObject);
    //glBindVertexArray(gVertexArrayObject);
    
    //SDL_GL_SetSwapInterval(1);
    
#else // QLIB_OPENGL
    SDL.Renderer = SDL_CreateRenderer(SDL.Window, -1, SDL_RENDERER_PRESENTVSYNC);
#endif // QLIB_OPENGL
    
    platform p = {};
    
    p.Memory.PermanentStorageSize = Permanent_Storage_Size;
    p.Memory.TransientStorageSize = Transient_Storage_Size;
    
    // TODO(casey): Handle various memory footprints (USING SYSTEM METRICS)
    uint64 TotalSize = p.Memory.PermanentStorageSize + p.Memory.TransientStorageSize;
    
    p.Memory.PermanentStorage = SDL_malloc(TotalSize);
    p.Memory.TransientStorage = ((uint8 *)p.Memory.PermanentStorage + p.Memory.PermanentStorageSize);
    
    GlobalRunning = true;
    
    callback_data Data = {};
    Data.p = &p;
    Data.SDL = &SDL;
    
#ifdef __EMSCRIPTEN__
    EmscriptenFullscreenStrategy strategy;
    strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
    strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
    strategy.canvasResizedCallback = emscripten_window_resized_callback;
    strategy.canvasResizedCallbackUserData = &Data;
    emscripten_enter_soft_fullscreen("canvas", &strategy);
#endif
    
    while (GlobalRunning)
    {
        printf("GlobalRunning\n");
        SDLProcessPendingEvents();
        
        SDL_GetWindowSize(SDL.Window, &p.Dimension.Width, &p.Dimension.Height);
        
        UpdateRender(&p);
        
        printf("%s\n", GlobalDebugBuffer.Data);
        memset(GlobalDebugBuffer.Data, 0, GlobalDebugBuffer.Size);
        GlobalDebugBuffer.Next = GlobalDebugBuffer.Data;
        
        /*
        SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
        SDL_Rect rect = {.x = WindowWidth/2, .y = WindowHeight/2, .w = 10, .h = 10};
        SDL_RenderFillRect(renderer, &rect);
        */
        
#if QLIB_OPENGL
        SDL_GL_SwapWindow(SDL.Window);
#else
        SDL_RenderPresent(renderer);
#endif
    }
    
    SDL_DestroyRenderer(SDL.Renderer);
    SDL_DestroyWindow(SDL.Window);
    
    emscripten_cancel_main_loop();
}

int main()
{
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop([]() { MainLoop(); }, 0, 1);
#else
    while (MainLoop(&SDL, &p));
#endif
    
    SDL_Quit();
    
    return 0;
}