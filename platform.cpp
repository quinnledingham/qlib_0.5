internal void
PlatformProcessKeyboardMessage(platform_button_state *State, platform_button_message Msg)
{
    if (Msg.IsDown) {
        if (!Msg.Repeat) {
            State->Repeat = false;
            State->NewEndedDown = true;
            ArrPushPointer(Msg.ButtonsToClear, State, sizeof(platform_button_state*));
        }
        else {
            State->Repeat = true;
            State->NewEndedDown = false;
            ArrPushPointer(Msg.ButtonsToClear, State, sizeof(platform_button_state*));
        }
        
        State->EndedDown = true;
    }
    else if (!Msg.IsDown) {
        State->NewEndedDown = false;
        State->EndedDown = false;
    }
}

internal void
PlatformClearButtons(platform_input *Input, arr *ButtonsToClear)
{
    if (ButtonsToClear->CurrentSize != ButtonsToClear->MaxSize)
    {
        for (int i = 0; i < ButtonsToClear->CurrentSize; i++) {
            platform_button_state **Button = (platform_button_state**)Input->ButtonsToClear[i];
            ClearKeyState((*Button));
        }
    }
    else
    {
        for (int i = 0; i < ArrayCount(Input->Keyboard.Buttons); i++)
            ClearKeyState(&Input->Keyboard.Buttons[i]);
        
        for (int i = 0; i < ArrayCount(Input->Mouse.Buttons); i++) 
            ClearKeyState(&Input->Mouse.Buttons[i]);
    }
    ArrClear(ButtonsToClear, sizeof(platform_button_state*));
}

// https://stackoverflow.com/questions/1735236/how-to-write-my-own-printf-in-c
void Log(char * fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    
    char *Traverse;
    for (Traverse = fmt; *Traverse != '\0'; Traverse++)
    {
        
    }
    
#ifdef QLIB_SDL
    SDL_Log(fmt, arg);
#endif
    
    va_end(arg);
}