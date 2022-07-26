#ifndef PLATFORM_H
#define PLATFORM_H
/*
  NOTE(casey): Services that the game provides to the platform layer.
  (this may expand in the future - sound on separate thread, etc.)
*/

// FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

// Start Possible Win32
// TODO(casey): In the future, rendering _specifically_ will become a three-tiered abstraction!!!
struct platform_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};
global_variable platform_offscreen_buffer GlobalBackbuffer;

struct platform_window_dimension
{
    int Width;
    int Height;
};

struct
platform_memory
{
    uint64 PermanentStorageSize;
    void *PermanentStorage; // REQUIRED to be cleared to zero at startup
    
    uint64 TransientStorageSize;
    void *TransientStorage; // REQUIRED to be cleared to zero at startup
};
// End Win32

struct platform_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16 *Samples;
};
// NOTE(casey): At the moment, this has to be a very fast function, it cannot be
// more than a millisecond or so.
// TODO(casey): Reduce the pressure on this function's performance by measuring it
// or asking about it, etc.
#define PLATFORM_GET_SOUND_SAMPLES(name) void name(platform_memory *Memory, platform_sound_output_buffer *SoundBuffer)
typedef PLATFORM_GET_SOUND_SAMPLES(platform_get_sound_samples);

struct platform_button_state
{
    bool32 EndedDown;
    bool32 NewEndedDown;
    bool32 NewEndedUp;
};
inline bool KeyDown(platform_button_state *Button)
{
    return Button->EndedDown;
}
inline bool OnKeyDown(platform_button_state *Button)
{
    if (Button->NewEndedDown) {
        printf("OnKeyDown\n");
        //Button->NewEndedDown = false;
        return true;
    }
    return false;
}
inline bool OnKeyUp(platform_button_state *Button)
{
    if (Button->NewEndedUp) {
        Button->NewEndedUp = false;
        return true;
    }
    return false;
}

enum struct
platform_cursor_mode
{
    Arrow,
    Hand,
};
struct
platform_mouse_input
{
    union
    {
        platform_button_state Buttons[5];
        struct
        {
            platform_button_state Left;
            platform_button_state Right;
            platform_button_state Middle;
            platform_button_state Four;
            platform_button_state Five;
        };
    };
    int32 X, Y, Z;
    bool32 Moved;
    platform_cursor_mode Cursor;
    bool32 NewCursor;
};

struct platform_controller_input
{
    bool32 IsConnected;
    bool32 IsAnalog;
    real32 StickAverageX;
    real32 StickAverageY;
    
    bool32 IgnoreInputs;
    
    union
    {
        platform_button_state Buttons[12];
        struct
        {
            platform_button_state MoveUp;
            platform_button_state MoveDown;
            platform_button_state MoveLeft;
            platform_button_state MoveRight;
            
            platform_button_state ActionUp;
            platform_button_state Enter;
            platform_button_state ActionLeft;
            platform_button_state ActionRight;
            
            platform_button_state LeftShoulder;
            platform_button_state RightShoulder;
            
            platform_button_state Back;
            platform_button_state Start;
            
            // NOTE(casey): All buttons must be added above this line
            
            platform_button_state Terminator;
        };
    };
};

struct platform_keyboard_input
{
    bool32 IsConnected;
    char Clipboard[1000];
    
    platform_controller_input *ControllerInput;
    
    union
    {
        platform_button_state Buttons[24];
        struct
        {
            platform_button_state W;
            platform_button_state A;
            platform_button_state S;
            platform_button_state D;
            
            platform_button_state CtrlV;
            platform_button_state Escape;
            platform_button_state Period;
            platform_button_state Backspace;
            platform_button_state Tab;
            platform_button_state F5;
            platform_button_state F6;
            union
            {
                platform_button_state Numbers[10];
                struct
                {
                    platform_button_state Zero;
                    platform_button_state One;
                    platform_button_state Two;
                    platform_button_state Three;
                    platform_button_state Four;
                    platform_button_state Five;
                    platform_button_state Six;
                    platform_button_state Seven;
                    platform_button_state Eight;
                    platform_button_state Nine;
                };
            };
            union
            {
                platform_button_state ArrowKeys[4];
                struct
                {
                    platform_button_state Right;
                    platform_button_state Up;
                    platform_button_state Left;
                    platform_button_state Down;
                };
            };
        };
    };
};

enum struct platform_input_mode
{
    Mouse,
    Controller,
    Keyboard,
};
struct platform_input_info
{
    platform_input_mode InputMode;
    platform_controller_input *Controller;
};

struct
platform_input
{
    real32 WorkSecondsElapsed;
    real32 TriggerCount;
    
    platform_controller_input Controllers[5];
    platform_keyboard_input Keyboard;
    platform_mouse_input Mouse;
    
    arr NewEndedDownButtons;
    
    union
    {
        platform_input_info InputInfo[2];
        struct 
        {
            platform_input_info CurrentInputInfo;
            platform_input_info PreviousInputInfo;
        };
    };
    
    int Quit;
};
inline platform_controller_input *GetController(platform_input *Input, int unsigned ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    
    platform_controller_input *Result = &Input->Controllers[ControllerIndex];
    return(Result);
}
inline platform_keyboard_input *GetKeyboard(platform_input *Input, int unsigned KeyboardIndex)
{
    platform_keyboard_input *Result = &Input->Keyboard;
    return Result;
}
inline void PlatformSetCursorMode(platform_mouse_input *Mouse, platform_cursor_mode CursorMode)
{
    if (Mouse->Cursor != CursorMode) {
        Mouse->Cursor = CursorMode;
        Mouse->NewCursor = true;
    }
    else
        Mouse->NewCursor = false;
}
inline void UpdateInputInfo(platform_input *Input)
{
    platform_input_info InputInfo = Input->PreviousInputInfo;
    
    for (int i = 0; i < ArrayCount(Input->Controllers); i++) {
        platform_controller_input *Input2 = &Input->Controllers[i];
        for (int j = 0; j < ArrayCount(Input2->Buttons); j++) {
            if (Input2->Buttons[j].EndedDown != 0) {
                if (i == 0) 
                    InputInfo.InputMode = platform_input_mode::Keyboard;
                else 
                    InputInfo.InputMode = platform_input_mode::Controller;
                InputInfo.Controller = &Input->Controllers[i];
            }
        }
    }
    platform_keyboard_input *Input2 = &Input->Keyboard;
    for (int i = 0; i < ArrayCount(Input2->Buttons); i++) {
        if (Input2->Buttons[i].EndedDown != 0) {
            InputInfo.InputMode = platform_input_mode::Keyboard;
            InputInfo.Controller = &Input->Controllers[0];
        }
    }
    
    if (Input->Mouse.Moved) {
        InputInfo.InputMode = platform_input_mode::Mouse;
        InputInfo.Controller = &Input->Controllers[0];
    }
    
    Input->PreviousInputInfo = Input->CurrentInputInfo;
    Input->CurrentInputInfo = InputInfo;
}
inline bool KeyPressed(platform_button_state *Button, platform_input *Input)
{
    if (OnKeyDown(Button)) {
        Input->TriggerCount = -0.2f;
        return true;
    }
    else if (KeyDown(Button)) {
        //fprintf(stderr, "%d %d %f\n", Button->EndedDown, Button->HalfTransitionCount, Input->TriggerCount);
        Input->TriggerCount += Input->WorkSecondsElapsed;
        if (Input->TriggerCount >= 0.05f) {
            Input->TriggerCount = 0.0f;
            return true;
        }
    }
    
    return false;
}

// Multithreading
struct platform_work_queue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(platform_work_queue *Queue, void *Data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);

typedef void platform_add_entry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data);
typedef void platform_complete_all_work(platform_work_queue *Queue);

struct platform_work_queue_entry
{
    platform_work_queue_callback *Callback;
    void *Data;
};

struct platform_work_queue
{
    uint32 volatile CompletionGoal;
    uint32 volatile CompletionCount;
    
    uint32 volatile NextEntryToWrite;
    uint32 volatile NextEntryToRead;
    HANDLE SemaphoreHandle;
    //u32 SemaphoreHandle;
    
    platform_work_queue_entry Entries[256];
};
// End of Multithreading

struct
platform
{
    platform_input Input;
    platform_sound_output_buffer SoundOutputBuffer;
    
    platform_memory Memory;
    platform_window_dimension Dimension;
    platform_work_queue Queue;
    
    audio_state AudioState;
};
inline v2 GetDim(platform *p)
{
    return v2((real32)p->Dimension.Width, (real32)p->Dimension.Height);
}
inline v2 GetTopLeftCornerCoords(platform *p)
{
    return v2(-p->Dimension.Width/2, -p->Dimension.Height/2);
}

// PrintqDebug
#define OUTPUTBUFFER_SIZE 1000
struct platform_debug_buffer
{
    int MaxSize = OUTPUTBUFFER_SIZE;
    int Size = 0;
    char Data[OUTPUTBUFFER_SIZE];
    char* Next;
    
    HANDLE Mutex;
    //u32 Mutex;
};
global_variable platform_debug_buffer GlobalDebugBuffer = {};

#define BEGIN_BLOCK(Name)
#define END_BLOCK(Name)

#endif //PLATFORM_H