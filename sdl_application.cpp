global_variable bool32 GlobalRunning;

struct sdl
{
    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDL_GLContext Context;
    
    SDL_AudioSpec AudioSpec;
    SDL_AudioDeviceID AudioDeviceID;
};

struct sdl_button_message
{
    uint8 IsDown;
    uint8 Repeat;
    arr *NewEndedDownButtons;
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
SDLPrintKeyMessage(platform_button_state *State, sdl_button_message Msg)
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

internal void
SDLProcessKeyboardMessage(platform_button_state *State, sdl_button_message Msg)
{
    if (Msg.IsDown == SDL_PRESSED) {
        if (!Msg.Repeat) {
            State->NewEndedDown = true;
            ArrPushPointer(Msg.NewEndedDownButtons, State, sizeof(platform_button_state*));
        }
        else
            State->NewEndedDown = false;
        
        State->EndedDown = true;
    }
    else if (Msg.IsDown == SDL_RELEASED) {
        State->NewEndedDown = false;
        State->EndedDown = false;
    }
}

internal void
SDLProcessPendingEvents(platform_keyboard_input *Keyboard, 
                        platform_mouse_input *Mouse,
                        arr *NewEndedDownButtons)
{
    SDL_Event Event;
    
    while (SDL_PollEvent(&Event))
    {
        sdl_button_message Msg = {};
        Msg.NewEndedDownButtons = NewEndedDownButtons;
        
        switch (Event.type)
        {
            case SDL_KEYUP:
            case SDL_KEYDOWN:
            {
                uint32 KeyCode = Event.key.keysym.sym;
                Msg.Repeat = Event.key.repeat;
                Msg.IsDown = Event.key.state;
                
                if (Event.type == SDL_KEYDOWN)
                    Msg.IsDown = true;
                
                if (KeyCode == SDLK_w)
                    SDLProcessKeyboardMessage(&Keyboard->W, Msg);
                else if (KeyCode == SDLK_a)
                    SDLProcessKeyboardMessage(&Keyboard->A, Msg);
                else if (KeyCode == SDLK_s)
                    SDLProcessKeyboardMessage(&Keyboard->S, Msg);
                else if (KeyCode == SDLK_d)
                    SDLProcessKeyboardMessage(&Keyboard->D, Msg);
                else if (KeyCode == SDLK_RETURN)
                    SDLProcessKeyboardMessage(&Keyboard->ControllerInput->Enter, Msg);
            } break;
            
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                uint32 Type = Event.button.button;
                Msg.IsDown = Event.button.state;
                if (Type == SDL_BUTTON_LEFT)
                    SDLProcessKeyboardMessage(&Mouse->Left, Msg);
            } break;
            
            case SDL_MOUSEMOTION:
            {
                Mouse->X = Event.motion.x;
                Mouse->Y = Event.motion.y;
                Mouse->Moved = true;
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
    
    /*
    // SDL_audio
    for (int i = 0; i < SDL_GetNumAudioDrivers(); ++i)
    {
        const char *DriverName = SDL_GetAudioDriver(i);
        
        if (SDL_AudioInit(DriverName))
            printf("Audio driver failed to initialize: %s\n", DriverName);
        else
            printf("Audio driver initialized: %s\n", DriverName);
        
    }
    */
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
    
    // Check OpenGL properties
    printf("\nOpenGL loaded\n");
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);
    
    //SDL_GL_SetSwapInterval(1);
    
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
    ArrInit(&p.Input.NewEndedDownButtons, 10, sizeof(platform_button_state*));
    Manager.Start = Manager.Next;
    
    p.Input.Keyboard.ControllerInput = &p.Input.Controllers[0];
    
    uint32 LastFrameTicks = 0;
    uint32 LastAudioTicks = 0;
    
    GlobalRunning = true;
    while (GlobalRunning)
    {
        p.Input.Mouse.Moved = false;
        SDLProcessPendingEvents(&p.Input.Keyboard, &p.Input.Mouse, &p.Input.NewEndedDownButtons);
        SDL_GetWindowSize(SDL.Window, &p.Dimension.Width, &p.Dimension.Height);
        
        if (p.Input.Mouse.Moved) {
            p.Input.Mouse.X -= (p.Dimension.Width / 2);
            p.Input.Mouse.Y -= (p.Dimension.Height / 2);
        }
        
        p.Input.WorkSecondsElapsed = SDLGetSeconds(&LastFrameTicks);
        
        UpdateRender(&p);
        
        if (p.Input.Quit) {
            GlobalRunning = false;
            continue;
        }
        
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
            SoundBuffer.SampleCount = (int)roundf(Seconds * SoundBuffer.SamplesPerSecond);
        else
            SoundBuffer.SampleCount = 0;
        
        SoundBuffer.Samples = (int16*)Samples;
        SoundBuffer.SampleCount += 50;
        
        if (p.AudioState.Paused.Value)
            PlayLoadedSound(&p.AudioState, &SoundBuffer);
        
        if (SoundBuffer.SampleCount * 4 < SDL.AudioSpec.samples)
            int success = SDL_QueueAudio(SDL.AudioDeviceID, SoundBuffer.Samples, SoundBuffer.SampleCount * 4);
        // End of Audio
        
        // Clearing Input NewEndedDowns
        if (p.Input.NewEndedDownButtons.CurrentSize != p.Input.NewEndedDownButtons.MaxSize)
        {
            for (int i = 0; i < p.Input.NewEndedDownButtons.CurrentSize; i++) {
                platform_button_state **Button = (platform_button_state**)p.Input.NewEndedDownButtons[i];
                (*Button)->NewEndedDown = false;
            }
        }
        else
        {
            for (int i = 0; i < ArrayCount(p.Input.Keyboard.Buttons); i++)
                p.Input.Keyboard.Buttons[i].NewEndedDown = false;
            
            for (int i = 0; i < ArrayCount(p.Input.Mouse.Buttons); i++) 
                p.Input.Mouse.Buttons[i].NewEndedDown = false;
        }
        ArrClear(&p.Input.NewEndedDownButtons, sizeof(platform_button_state*));
        // End of Clearing Input NewEndedDowns
        
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