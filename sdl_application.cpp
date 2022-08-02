global_variable bool32 GlobalRunning;

struct sdl
{
    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDL_GLContext Context;
    
    SDL_AudioSpec AudioSpec;
    SDL_AudioDeviceID AudioDeviceID;
};

void
SDLPrintAudioSpec(SDL_AudioSpec *AudioSpec)
{
    printf("\nAudioSpec\n");
    printf("freq %d\n", AudioSpec->freq);
    printf("format %d\n", AudioSpec->format);
    printf("channels %d\n", AudioSpec->channels);
    printf("silence %d\n", AudioSpec->silence);
    printf("samples %d\n", AudioSpec->samples);
    printf("size %d\n", AudioSpec->size);
}

void
SDLPrintKeyMessage(platform_button_state *State, platform_button_message Msg)
{
    printf("\nKeyMsg\n");
    printf("EndedDown %d\n", State->EndedDown);
    printf("NewEndedDown %d\n", State->NewEndedDown);
    printf("Repeat %d\n", Msg.Repeat);
}

internal real32
SDLGetSeconds(uint32 *LastTicksCount)
{
    uint32 CurrentTicksCount = SDL_GetTicks();
    real32 ret = (real32)(CurrentTicksCount - *LastTicksCount) / 1000;
    *LastTicksCount = CurrentTicksCount;
    return ret; 
}

internal real32
SDLGetMilliseconds(uint32 *LastTicksCount)
{
    uint32 CurrentTicksCount = SDL_GetTicks();
    real32 ret = (real32)(CurrentTicksCount - *LastTicksCount);
    *LastTicksCount = CurrentTicksCount;
    return ret; 
}

internal void
SDLProcessPendingEvents(iv2 PlatformDim, platform_input *Input)
{
    platform_keyboard_input *Keyboard = &Input->Keyboard;
    platform_mouse_input *Mouse = &Input->Mouse;
    
    SDL_Event Event;
    
    while (SDL_PollEvent(&Event))
    {
        platform_button_message Msg = {};
        Msg.ButtonsToClear = &Input->ButtonsToClear;;
        
        switch (Event.type)
        {
            case SDL_KEYUP:
            case SDL_KEYDOWN:
            {
                uint32 KeyCode = Event.key.keysym.sym;
                if (Event.key.repeat)
                    Msg.Repeat = true;
                
                if (Event.key.state == SDL_PRESSED) {
                    Input->ActiveInput = platform_input_index::keyboard;
                    Msg.IsDown = true;
                }
                else
                    Msg.IsDown = false;
                
                if (KeyCode == SDLK_w) PlatformProcessKeyboardMessage(&Keyboard->W, Msg);
                else if (KeyCode == SDLK_a) PlatformProcessKeyboardMessage(&Keyboard->A, Msg);
                else if (KeyCode == SDLK_s) PlatformProcessKeyboardMessage(&Keyboard->S, Msg);
                else if (KeyCode == SDLK_d) PlatformProcessKeyboardMessage(&Keyboard->D, Msg);
                
                else if (KeyCode == SDLK_F5) PlatformProcessKeyboardMessage(&Keyboard->F5, Msg);
                else if (KeyCode == SDLK_F6) PlatformProcessKeyboardMessage(&Keyboard->F6, Msg);
                
                else if (KeyCode == SDLK_RETURN) PlatformProcessKeyboardMessage(&Keyboard->Enter, Msg);
                else if (KeyCode == SDLK_BACKSPACE) PlatformProcessKeyboardMessage(&Keyboard->Backspace, Msg);
                else if (KeyCode == SDLK_TAB) PlatformProcessKeyboardMessage(&Keyboard->Tab, Msg);
                else if (KeyCode == SDLK_PERIOD) PlatformProcessKeyboardMessage(&Keyboard->Period, Msg);
                else if (KeyCode == SDLK_ESCAPE) PlatformProcessKeyboardMessage(&Keyboard->Esc, Msg);
                
                else if (KeyCode == SDLK_UP) PlatformProcessKeyboardMessage(&Keyboard->Up, Msg);
                else if (KeyCode == SDLK_LEFT) PlatformProcessKeyboardMessage(&Keyboard->Left, Msg);
                else if (KeyCode == SDLK_DOWN) PlatformProcessKeyboardMessage(&Keyboard->Down, Msg);
                else if (KeyCode == SDLK_RIGHT) PlatformProcessKeyboardMessage(&Keyboard->Right, Msg);
                
                else if(KeyCode >= SDLK_0 && KeyCode <= SDLK_9)
                {
                    int index = KeyCode - SDLK_0;
                    PlatformProcessKeyboardMessage(&Keyboard->Numbers[index], Msg);
                }
                
                else if (KeyCode == SDLK_v) PlatformProcessKeyboardMessage(&Keyboard->V, Msg);
                else if (KeyCode == SDLK_LCTRL) PlatformProcessKeyboardMessage(&Keyboard->Ctrl, Msg);
                else if (KeyCode == SDLK_RCTRL) PlatformProcessKeyboardMessage(&Keyboard->Ctrl, Msg);
                
            } break;
            
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                uint32 Type = Event.button.button;
                
                if (Event.button.state == SDL_PRESSED) {
                    Input->ActiveInput = platform_input_index::mouse;
                    Msg.IsDown = true;
                }
                else
                    Msg.IsDown = false;
                
                if (Type == SDL_BUTTON_LEFT)
                    PlatformProcessKeyboardMessage(&Mouse->Left, Msg);
            } break;
            
            case SDL_MOUSEMOTION:
            {
                Mouse->X = Event.motion.x;
                Mouse->Y = Event.motion.y;
                Mouse->X -= (PlatformDim.Width / 2);
                Mouse->Y -= (PlatformDim.Height / 2);
                Input->ActiveInput = platform_input_index::mouse;
            } break;
            
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

bool MainLoop()
{
    PlatformSetCD(CurrentDirectory);
    
    sdl SDL = {};
    
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
    
    
    SDL.AudioSpec.freq = 48000;
    SDL.AudioSpec.format = AUDIO_S16;
    SDL.AudioSpec.channels = 2;
    SDL.AudioSpec.samples = 3999;
    
    SDL.AudioDeviceID = SDL_OpenAudioDevice(NULL, 0, &SDL.AudioSpec, NULL, 0);
    SDL_PauseAudioDevice(SDL.AudioDeviceID, 0);
    
    uint8 *Samples = (uint8*)SDL_malloc(SDL.AudioSpec.samples);
    SDL_memset(Samples, 0, SDL.AudioSpec.samples);
    
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
    SDL_GL_SetSwapInterval(0);
    
    // Check OpenGL properties
    printf("\nOpenGL loaded\n");
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);
    
#else // QLIB_OPENGL
    SDL.Renderer = SDL_CreateRenderer(SDL.Window, -1,
                                      SDL_RENDERER_PRESENTVSYNC);
#endif // QLIB_OPENGL
    
    platform p = {};
    
    p.Memory.PermanentStorageSize = Permanent_Storage_Size;
    p.Memory.TransientStorageSize = Transient_Storage_Size;
    uint64 TotalSize = p.Memory.PermanentStorageSize + p.Memory.TransientStorageSize;
    p.Memory.PermanentStorage = SDL_malloc(TotalSize);
    p.Memory.TransientStorage = ((uint8*)p.Memory.PermanentStorage + p.Memory.PermanentStorageSize);
    
    // Initing Memory Manager. Moving beginning past this input arr.
    Manager.Next = (char*)p.Memory.PermanentStorage;
    ArrInit(&p.Input.ButtonsToClear, 10, sizeof(platform_button_state*));
    Manager.Start = Manager.Next;
    
    uint32 LastFrameTicks = 0;
    uint32 LastFrameTicksMilli = 0;
    uint32 LastAudioTicks = 0;
    
    GlobalRunning = true;
    while (GlobalRunning)
    {
        SDLProcessPendingEvents(GetDim(&p), &p.Input);
        SDL_GetWindowSize(SDL.Window, &p.Dimension.Width, &p.Dimension.Height);
        
        p.Input.MillisecondsElapsed = SDLGetMilliseconds(&LastFrameTicksMilli);
        p.Input.WorkSecondsElapsed = p.Input.MillisecondsElapsed / 1000;
        
        UpdateRender(&p);
        
        if (p.Input.Quit) {
            GlobalRunning = false;
            continue;
        }
        
        // Cursor
        if (p.Input.Mouse.NewCursor)
        {
            if (p.Input.Mouse.Cursor == platform_cursor_mode::Arrow)
            {
                SDL_Cursor *Cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
                SDL_SetCursor(Cursor);
            }
            else if (p.Input.Mouse.Cursor == platform_cursor_mode::Hand)
            {
                SDL_Cursor *Cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
                SDL_SetCursor(Cursor);
            }
        }
        // End of Cursor
        
        if (GlobalDebugBuffer.Data[0] != 0)
            printf("%s\n", GlobalDebugBuffer.Data);
        memset(GlobalDebugBuffer.Data, 0, GlobalDebugBuffer.Size);
        GlobalDebugBuffer.Size = 0;
        GlobalDebugBuffer.Next = GlobalDebugBuffer.Data;
        
        // Audio
        platform_sound_output_buffer SoundBuffer = {};
        SoundBuffer.SamplesPerSecond = SDL.AudioSpec.freq;
        
        real32 Seconds = SDLGetSeconds(&LastAudioTicks);
        if (Seconds < 1)
            SoundBuffer.SampleCount = (int)ceil(Seconds * SoundBuffer.SamplesPerSecond);
        else
            SoundBuffer.SampleCount = 0;
        
        SoundBuffer.Samples = (int16*)Samples;
        //SoundBuffer.SampleCount += 5;
        
        if (p.AudioState.Paused.Value)
            PlayLoadedSound(&p.AudioState, &SoundBuffer);
        
        if (SoundBuffer.SampleCount * 4 < SDL.AudioSpec.samples)
            int success = SDL_QueueAudio(SDL.AudioDeviceID, SoundBuffer.Samples, SoundBuffer.SampleCount * 4);
        // End of Audio
        
        PlatformClearButtons(&p.Input, &p.Input.ButtonsToClear);
        
#if QLIB_OPENGL
        SDL_GL_SwapWindow(SDL.Window);
#else
        SDL_RenderPresent(SDL.Renderer);
#endif
    }
    
    SDL_DestroyRenderer(SDL.Renderer);
    SDL_DestroyWindow(SDL.Window);
    SDL_AudioQuit();
    
    return false;
}

int main(int argc, char *argv[])
{
    while (MainLoop());
    SDL_Quit();
    return 0;
}