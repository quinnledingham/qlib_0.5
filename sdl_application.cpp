global_variable bool32 GlobalRunning;

struct sdl
{
    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDL_GLContext Context;
    
    SDL_AudioSpec AudioSpec;
    SDL_AudioDeviceID AudioDeviceID;
    
    // Cursors
    SDL_Cursor *CursorArrow;
    SDL_Cursor *CursorHand;
};

struct sdl_controller_button
{
    int PlatformControllerButton;
    SDL_GameControllerButton SDLControllerButton;
};

void Log(char* Text)
{
    SDL_Log("%s", Text);
}

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
    real32 ret = (real32)(CurrentTicksCount - *LastTicksCount) / 1000.0f;
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

struct sdl_thread_info
{
    platform_work_queue *Queue;
    u32 LogicalThreadIndex;
};

static int ThreadProc2(void *Data)
{
    sdl_thread_info *ThreadInfo = (sdl_thread_info*)Data;
    
    while (1) {
        
    }
    
    return 0;
}

internal void
SDLInitThreads(sdl_thread_info *ThreadInfo, int InfoArrayCount, platform_work_queue *Queue)
{
    u32 InitialCount = 0;
    u32 ThreadCount = InfoArrayCount;
    Queue->SemaphoreHandle = SDL_CreateSemaphore(InitialCount);
    for (u32 ThreadIndex = 0; ThreadIndex < ThreadCount; ThreadIndex++) {
        sdl_thread_info *Info = ThreadInfo + ThreadIndex;
        Info->Queue;
        Info->LogicalThreadIndex = ThreadIndex;
        
        strinq Name = S() + "Thread" + ThreadIndex;
        SDL_CreateThread(ThreadProc2, Name.Data, Info);
    }
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
                    Input->ActiveInput = active_input_type::Keyboard;
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
                    Input->ActiveInput = active_input_type::Mouse;
                    Msg.IsDown = true;
                }
                else
                    Msg.IsDown = false;
                
                if (Type == SDL_BUTTON_LEFT)
                    PlatformProcessKeyboardMessage(&Mouse->Left, Msg);
                
                SDL_GetMouseState(&Mouse->X, &Mouse->Y);
            } break;
            
            case SDL_MOUSEMOTION:
            {
                Mouse->X = Event.motion.x;
                Mouse->Y = Event.motion.y;
                Input->ActiveInput = active_input_type::Mouse;
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
    
    // Setting up SDL Window
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
    
    SDL.Window = SDL_CreateWindow(WindowName, 
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                  SDLClientWidth, SDLClientHeight, 
                                  SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    loaded_bitmap IconBitmap = LoadBitmap(IconFileName);
    ResizeBitmap(&IconBitmap, v2(72, 72));
    SDL_Surface* Icon = SDL_CreateRGBSurfaceWithFormatFrom(IconBitmap.Memory, IconBitmap.Width,  IconBitmap.Height, 
                                                           32,  4 * IconBitmap.Width, SDL_PIXELFORMAT_RGBA32);
    SDL_SetWindowIcon(SDL.Window, Icon);
    // End of setting up SDL Window
    
    // Setting up SDL Audio
    SDL.AudioSpec.freq = 48000;
    SDL.AudioSpec.format = AUDIO_S16; // signed 16 bit (int16)
    SDL.AudioSpec.channels = 2;
    SDL.AudioSpec.samples = 4000; // 12 fps
    
    SDL.AudioDeviceID = SDL_OpenAudioDevice(NULL, 0, &SDL.AudioSpec, NULL, 0);
    SDL_PauseAudioDevice(SDL.AudioDeviceID, 0);
    
    int16 *Samples = (int16*)SDL_malloc(SDL.AudioSpec.samples * AUDIO_S16_BYTES);
    SDL_memset(Samples, 0, SDL.AudioSpec.samples * AUDIO_S16_BYTES);
    // End of setting up SDL audio
    
    // Setting up SDL Cursors
    SDL.CursorArrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    SDL.CursorHand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    // End of settign up SDL Cursors
    
#if QLIB_OPENGL
    SDL_GL_LoadLibrary(NULL);
    
    // Request an OpenGL 4.5 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL.Context = SDL_GL_CreateContext(SDL.Window);
    SDL_GL_SetSwapInterval(0);
    
    // Check OpenGL properties
    Log("OpenGL loaded\n");
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress))
        Log("gladLoadGLLoader Failed\n");
    Log(string() + "Vendor:   " + glGetString(GL_VENDOR) + "\n");
    Log(string() + "Renderer: " + glGetString(GL_RENDERER) + "\n");
    Log(string() + "Version:  " + glGetString(GL_VERSION) + "\n");
    
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);
    
#else // QLIB_OPENGL
    SDL.Renderer = SDL_CreateRenderer(SDL.Window, -1, SDL_RENDERER_PRESENTVSYNC);
#endif // QLIB_OPENGL
    
    platform p = {};
    
    // Setting up platform memory
    p.Memory.PermanentStorageSize = Permanent_Storage_Size;
    p.Memory.TransientStorageSize = Transient_Storage_Size;
    uint64 TotalSize = p.Memory.PermanentStorageSize + p.Memory.TransientStorageSize;
    p.Memory.PermanentStorage = SDL_malloc(TotalSize);
    p.Memory.TransientStorage = ((uint8*)p.Memory.PermanentStorage + p.Memory.PermanentStorageSize);
    
    // Initing Memory Manager. Moving beginning past this input arr.
    Manager.Next = (char*)p.Memory.PermanentStorage;
    ArrInit(&p.Input.ButtonsToClear, 10, sizeof(platform_button_state*));
    Manager.Start = Manager.Next;
    // End of setting up platform memory
    
    uint32 LastFrameTicks = 0;
    uint32 LastFrameTicksMilli = 0;
    uint32 LastAudioTicks = 0;
    
    // Setting up input
    // Binding sdl buttons to platform buttons
    SDL_Log("NumJoysticks %d: %s\n", SDL_NumJoysticks(), SDL_GetError());
    
    platform_controller_input PtfmCtrl;
    sdl_controller_button ControllerButtons[6] =
    {
        {ButtonIndex(PtfmCtrl, DPadUp), SDL_CONTROLLER_BUTTON_DPAD_UP},
        {ButtonIndex(PtfmCtrl, DPadDown), SDL_CONTROLLER_BUTTON_DPAD_DOWN},
        {ButtonIndex(PtfmCtrl, DPadLeft), SDL_CONTROLLER_BUTTON_DPAD_LEFT},
        {ButtonIndex(PtfmCtrl, DPadRight), SDL_CONTROLLER_BUTTON_DPAD_RIGHT},
        {ButtonIndex(PtfmCtrl, Start), SDL_CONTROLLER_BUTTON_START},
        {ButtonIndex(PtfmCtrl, A), SDL_CONTROLLER_BUTTON_A},
    };
    
    p.Input.ActiveInput = active_input_type::Keyboard;
    // End of setting up input
    
    // Check out wav file
    /*
    SDL_AudioSpec WavFile;
    uint8 *TempBuf = 0;
    uint32 TempLen = 0;
    SDL_LoadWAV("sounds/gulp.wav", &WavFile, &TempBuf, &TempLen);
    SDLPrintAudioSpec(&WavFile);
    */
    // End of messing around
    
    // MultiThreading
    sdl_thread_info ThreadInfo[7];
    SDLInitThreads(ThreadInfo, ArrayCount(ThreadInfo), &p.Queue);
    // End of Multithreading
    
    // Random
    srand((unsigned int)time(NULL));
    // End of Random
    
    memset(GlobalDebugBuffer.Data, 0, GlobalDebugBuffer.Size);
    GlobalDebugBuffer.Size = 0;
    GlobalDebugBuffer.Next = GlobalDebugBuffer.Data;
    
    GlobalRunning = true;
    while (GlobalRunning)
    {
        SDLProcessPendingEvents(GetDim(&p), &p.Input);
        SDL_GetWindowSize(SDL.Window, &p.Dimension.Width, &p.Dimension.Height);
        
        // Controller Input
        for (int i = 0; i < SDL_NumJoysticks(); i++) {
            if (SDL_IsGameController(i)) {
                SDL_GameController *SDLController = SDL_GameControllerOpen(i);
                if (SDLController) {
                    platform_controller_input *Controller = GetController(&p.Input, i);
                    
                    for (int Btn = 0; Btn < ArrayCount(ControllerButtons); Btn++) {
                        
                        platform_button_state *PlatformButton = &Controller->Buttons[ControllerButtons[Btn].PlatformControllerButton];
                        
                        platform_button_message Msg =
                        {
                            SDL_GameControllerGetButton(SDLController, ControllerButtons[Btn].SDLControllerButton),
                            PlatformButton->EndedDown,
                            &p.Input.ButtonsToClear,
                        };
                        
                        PlatformProcessKeyboardMessage(PlatformButton, Msg);
                        
                        if (PlatformButton->EndedDown)
                            p.Input.ActiveInput = active_input_type::Controller;
                    }
                }
            }
            else
                SDL_Log("Could not open GameController %i: %s\n", i, SDL_GetError());
        }
        // End of Controller Input
        
        p.Input.MillisecondsElapsed = SDLGetMilliseconds(&LastFrameTicksMilli);
        p.Input.WorkSecondsElapsed = p.Input.MillisecondsElapsed / 1000;
        
        UpdateRender(&p);
        
        if (p.Quit) {
            GlobalRunning = false;
            continue;
        }
        
        // Cursor
        if (p.Input.Mouse.NewCursor)
        {
            if (p.Input.Mouse.Cursor == platform_cursor_mode::Arrow)
                SDL_SetCursor(SDL.CursorArrow);
            else if (p.Input.Mouse.Cursor == platform_cursor_mode::Hand)
                SDL_SetCursor(SDL.CursorHand);
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
            SoundBuffer.SampleCount = (int)floor(Seconds * SoundBuffer.SamplesPerSecond);
        else
            SoundBuffer.SampleCount = 0;
        
        //SDL_Log("SecondsElapsed: %f, SampleCount: %d", Seconds, SoundBuffer.SampleCount );
        //SDL_Log("%d\n", SoundBuffer.SampleCount);
        
        SoundBuffer.Samples = Samples;
        SoundBuffer.MaxSampleCount = SDL.AudioSpec.samples;
        //SoundBuffer.SampleCount += 5;
        
        if (p.AudioState.Paused.Value)
            PlayLoadedSound(&p.AudioState, &SoundBuffer);
        
        if (SoundBuffer.SampleCount < SDL.AudioSpec.samples)
            int success = SDL_QueueAudio(SDL.AudioDeviceID, SoundBuffer.Samples, SoundBuffer.SampleCount * AUDIO_S16_BYTES);
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
    //SDL_AudioQuit();
    
    return false;
}

int main(int argc, char *argv[])
{
    while (MainLoop());
    SDL_Quit();
    return 0;
}