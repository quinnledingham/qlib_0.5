#ifndef PLATFORM_H
#define PLATFORM_H

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

struct platform_sound_output_buffer
{
    uint32 SamplesPerSecond;
    uint32 SampleCount;
    uint32 MaxSampleCount;
    int16 *Samples;
};

struct platform_button_state
{
    bool32 EndedDown;
    bool32 NewEndedDown;
    bool32 Repeat;
};
inline bool KeyDown(platform_button_state *Button) { return Button->EndedDown; }
inline bool OnKeyDown(platform_button_state *Button) { return Button->NewEndedDown; }
inline bool OnMessage(platform_button_state *Button) 
{ 
    if (!Button->NewEndedDown)
        return Button->Repeat;
    else
        return Button->NewEndedDown;
}
inline void ClearKeyState(platform_button_state *Button)
{
    Button->NewEndedDown = false;
    Button->Repeat = false;
}

struct platform_button_message
{
    bool32 IsDown;
    bool32 Repeat;
    arr *ButtonsToClear;
};

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
    
    union
    {
        platform_button_state Buttons[28];
        struct
        {
            platform_button_state W;
            platform_button_state A;
            platform_button_state S;
            platform_button_state D;
            
            platform_button_state Ctrl;
            platform_button_state V;
            
            platform_button_state Enter;
            platform_button_state Esc;
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

enum struct platform_input_index
{
    keyboard,
    mouse,
    controller1,
    controller2,
    controller3,
    controller4,
};

struct
platform_input
{
    real32 WorkSecondsElapsed;
    real32 MillisecondsElapsed;
    real32 TriggerCount;
    arr ButtonsToClear;
    
    platform_input_index ActiveInput;
    
    platform_keyboard_input Keyboard;
    platform_mouse_input Mouse;
    platform_controller_input Controllers[4];
    
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
inline void PlatformSetCursorMode(platform_mouse_input *Mouse, platform_cursor_mode CursorMode)
{
    if (Mouse->Cursor != CursorMode) {
        Mouse->Cursor = CursorMode;
        Mouse->NewCursor = true;
    }
    else
        Mouse->NewCursor = false;
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

#endif //PLATFORM_H