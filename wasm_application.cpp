global_variable bool32 GlobalRunning;

struct sdl
{
    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDL_GLContext Context;
};

EM_BOOL emscripten_window_resized_callback(int eventType, const void *reserved, void *userData){
	//METHOD();
    
    sdl *SDL = (sdl*)userData;
    
	double width, height;
	emscripten_get_element_css_size("canvas", &width, &height);
    
	int w = (int)width, h = (int)height;
    
	// resize SDL window
	//Platform* platform = (Platform*)userData;
	//SDL_SetWindowSize((SDL_Window*)platform->get_wnd_handle(), w, h);
    SDL_SetWindowSize(SDL->Window, w, h);
    
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
    
    SDL.Window = SDL_CreateWindow(WindowName, 
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                  SDLClientWidth, SDLClientHeight, 
                                  SDL_WINDOW_RESIZABLE);
    SDL.Renderer = SDL_CreateRenderer(SDL.Window, -1, 
                                      SDL_RENDERER_PRESENTVSYNC);
    
    platform p = {};
    
    p.Memory.PermanentStorageSize = Permanent_Storage_Size;
    p.Memory.TransientStorageSize = Transient_Storage_Size;
    uint64 TotalSize = p.Memory.PermanentStorageSize + p.Memory.TransientStorageSize;
    p.Memory.PermanentStorage = SDL_malloc(TotalSize);
    p.Memory.TransientStorage = ((uint8*)p.Memory.PermanentStorage + p.Memory.PermanentStorageSize);
    
    EmscriptenFullscreenStrategy strategy;
    strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
    strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
    strategy.canvasResizedCallback = emscripten_window_resized_callback;
    strategy.canvasResizedCallbackUserData = &SDL;
    emscripten_enter_soft_fullscreen("canvas", &strategy);
    
    GlobalRunning = true;
    while (GlobalRunning)
    {
        SDLProcessPendingEvents();
        SDL_GetWindowSize(SDL.Window, &p.Dimension.Width, &p.Dimension.Height);
        
        UpdateRender(&p);
        
        printf("%s\n", GlobalDebugBuffer.Data);
        memset(GlobalDebugBuffer.Data, 0, GlobalDebugBuffer.Size);
        GlobalDebugBuffer.Next = GlobalDebugBuffer.Data;
        
        SDL_RenderPresent(SDL.Renderer);
    }
    
    SDL_DestroyRenderer(SDL.Renderer);
    SDL_DestroyWindow(SDL.Window);
    
    emscripten_cancel_main_loop();
}

int main (int argc, char *argv[])
{
    emscripten_set_main_loop(MainLoop, 0, 1);
    SDL_Quit();
    return 0;
}