#include "gui.h"
#include "memorymanager.h"
#include "snake.h"

internal int
MouseInRect(v2 Coords, v2 Dim, v2 MouseCoords)
{
    if (Coords.x < MouseCoords.x && MouseCoords.x < (Coords.x + Dim.x) &&
        Coords.y < MouseCoords.y && MouseCoords.y < (Coords.y + Dim.y))
        return 1;
    
    return 0;
}


internal void
AddCharTextBoxText(GUI* G, char* Char)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0) {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->ShowCursor == 1) {
            TB->Text = Concat(TB->Text, Char);
        }
        Cursor = Cursor->Next;
    }
}

internal void
RemoveCharTextBoxText(GUI* G)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0) {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->ShowCursor == 1) {
            int Size = Length(TB->Text);
            char* Result = (char*)qalloc(TB->Text, Size);
            Result[Size - 1] = 0;
            TB->Text = Result;
        }
        Cursor = Cursor->Next;
    }
}

internal void
ChangeTextBoxShowCursor(GUI* G, int ID)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0) {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->ID == ID)
            TB->ShowCursor = 1;
        else
            TB->ShowCursor = 0;
        
        Cursor = Cursor->Next;
    }
}

internal bool32
CheckButtonsHover(GUI* G, v2 MouseCoords)
{
    int InRect = 0;
    
    GUIComponent* Cursor = G->Buttons;
    while(Cursor != 0) {
        Button* B = (Button*)Cursor->Data;
        
        if (MouseInRect(Cursor->Coords, Cursor->Dim, MouseCoords)) {
            B->Color = B->HoverColor;
            InRect = 1;
        }
        else
            B->Color = B->RegularColor;
        
        Cursor = Cursor->Next;
    }
    
    if (InRect == 1)
        return true;
    
    return false;
}

// Returns what button was pressed if one was.
internal int
CheckButtonsClick(GUI* G, v2 MouseCoords)
{
    GUIComponent* Cursor = G->Buttons;
    while(Cursor != 0) {
        if (MouseInRect(Cursor->Coords, Cursor->Dim, MouseCoords)) {
            Button* btn = (Button*)Cursor->Data;
            return btn->ID;
        }
        
        Cursor = Cursor->Next;
    }
    
    return -1;
}

internal int
CheckTextBoxes(GUI* G, v2 MouseCoords)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0) {
        TextBox* btn = (TextBox*)Cursor->Data;
        if (MouseInRect(Cursor->Coords, Cursor->Dim, MouseCoords))
            btn->ShowCursor = 1;
        else
            btn->ShowCursor = 0;
        
        Cursor = Cursor->Next;
    }
    
    return -1;
}


internal int
AddGUIComponentAll(GUIComponent* Head, GUIComponent* NewComponent)
{
    // Start linked list
    if(Head == 0)
        return 0;
    
    GUIComponent* Cursor = Head;
    while(Cursor->All != 0)
        Cursor = Cursor->All;
    
    Cursor->All = NewComponent;
    return 1;
}

internal void
AddButton(GUI* G, int GridX, int GridY, int Width, int Height, Button* B)
{
    B->Color = B->RegularColor;
    GUIComponent NewTemp = {};
    GUIComponent* NewComponent = (GUIComponent*)qalloc((void*)&NewTemp, sizeof(GUIComponent));
    
    NewComponent->GridCoords = v2(GridX, GridY);
    NewComponent->Dim = v2(Width, Height);
    NewComponent->PaddingDim = v2(Width + (G->Padding * 2), Height + (G->Padding * 2));
    NewComponent->DefaultDim = NewComponent->Dim;
    NewComponent->Data = qalloc((void*)B, sizeof(Button));
    
    *B = {}; // Resets struct used to pass this function information
    
    // Start linked list
    if(G->Buttons == 0)
        G->Buttons = NewComponent;
    else {
        GUIComponent* Cursor = G->Buttons;
        while(Cursor->Next != 0)
            Cursor = Cursor->Next;
        Cursor->Next = NewComponent;
    }
    
    if (!AddGUIComponentAll(G->All, NewComponent))
        G->All = NewComponent;
    
}

internal void
AddText(GUI* G, int GridX, int GridY,  Text* T)
{
    GUIComponent NewTemp = {};
    GUIComponent* NewComponent = (GUIComponent*)qalloc((void*)&NewTemp, sizeof(GUIComponent));
    
    v2 StringDimension = GetStringDimensions(&T->FontType, T->Text);
    
    NewComponent->GridCoords = v2(GridX, GridY);
    NewComponent->Dim = v2(StringDimension.x,StringDimension.y);
    NewComponent->DefaultDim = NewComponent->Dim;
    NewComponent->PaddingDim = v2(StringDimension.x + (G->Padding * 2), StringDimension.y + (G->Padding * 2));
    NewComponent->Data = qalloc((void*)T, sizeof(Text));
    
    *T = {}; // Resets struct used to pass this function information
    
    // Start linked list
    if(G->Texts == 0)
        G->Texts = NewComponent;
    else {
        GUIComponent* Cursor = G->Texts;
        while(Cursor->Next != 0)
            Cursor = Cursor->Next;
        
        Cursor->Next = NewComponent;
    }
    
    if (!AddGUIComponentAll(G->All, NewComponent))
        G->All = NewComponent;
}

internal void
AddTextBox(GUI* G, int GridX, int GridY, int Width, int Height, TextBox* TB)
{
    GUIComponent NewTemp = {};
    GUIComponent* NewComponent = (GUIComponent*)qalloc((void*)&NewTemp, sizeof(GUIComponent));
    
    NewComponent->GridCoords = v2(GridX, GridY);
    NewComponent->Dim = v2(Width, Height);
    NewComponent->PaddingDim = v2(Width + (G->Padding * 2), Height + (G->Padding * 2));
    NewComponent->DefaultDim = v2(Width, Height);
    NewComponent->Data = qalloc((void*)TB, sizeof(TextBox));
    
    *TB = {}; // Resets struct used to pass this function information
    
    // Start linked list
    if(G->TextBoxes == 0)
        G->TextBoxes = NewComponent;
    else {
        GUIComponent* Cursor = G->TextBoxes;
        while(Cursor->Next != 0)
            Cursor = Cursor->Next;
        
        Cursor->Next = NewComponent;
    }
    
    if (!AddGUIComponentAll(G->All, NewComponent))
        G->All = NewComponent;
}

internal void
ResizeButton(GUI* G, GUIComponent* C, Button* B, int Width, int Height)
{
    float Scale = stbtt_ScaleForPixelHeight(&B->FontType.Info,
                                            (float)B->FontType.ScaleIn * ((float)Height / G->DefaultDim.y));
    B->FontType.Scale = Scale;
    C->Dim = v2(C->DefaultDim.x * ((float)Width / G->DefaultDim.x),
                C->DefaultDim.y * ((float)Height / G->DefaultDim.y));
    C->PaddingDim = v2(C->Dim.x  + (G->Padding * 2), C->Dim.y  + (G->Padding * 2));
}

internal void
ResizeTextBox(GUI* G,  GUIComponent* C, TextBox* Tb, int Width, int Height)
{
    float Scale = stbtt_ScaleForPixelHeight(&Tb->FontType.Info,
                                            (float)Tb->FontType.ScaleIn * ((float)Height / G->DefaultDim.y));
    Tb->FontType.Scale = Scale;
    C->Dim = v2(C->DefaultDim.x * ((float)Width / G->DefaultDim.x),
                C->DefaultDim.y * ((float)Height / G->DefaultDim.y));
    C->PaddingDim = v2(C->Dim.x  + (G->Padding * 2), C->Dim.y  + (G->Padding * 2));
}

internal void
ResizeText(GUI* G, GUIComponent* C, Text* T, int Height)
{
    float Scale = stbtt_ScaleForPixelHeight(&T->FontType.Info,
                                            (float)T->FontType.ScaleIn * ((float)Height / G->DefaultDim.y));
    T->FontType.Scale = Scale;
    
    v2 StringDimension = GetStringDimensions(&T->FontType, T->Text);
    C->PaddingDim = v2(StringDimension.x + (G->Padding * 2), StringDimension.y + (G->Padding * 2));
}

internal void
UpdateRow(Row* R, real32 Width, real32 Height, real32 GridX)
{
    R->ColumnWidths[(int)GridX] = (int)Width;
    R->Width += (int)Width;
    if (R->Height < Height)
        R->Height = (int)Height;
}

internal void
InitializeGUI(GUI* G)
{
    memset(G->Rows, 0, sizeof(Row) * 10);
    
    GUIComponent* Cursor = G->All;
    while(Cursor != 0) {
        UpdateRow(&G->Rows[(int)Cursor->GridCoords.y], Cursor->PaddingDim.x, Cursor->PaddingDim.y, Cursor->GridCoords.x);
        Cursor = Cursor->All;
    }
    
    G->Dim.y = 0;
    for (int i = 0; i < 10; i++)
        G->Dim.y += G->Rows[i].Height;
}

internal void
UpdateGUI(GUI* G, int BufferWidth, int BufferHeight)
{
    // Resize GUI if screen size changed
    //if (G->ClientWidth != BufferWidth || G->ClientHeight != BufferHeight) 
    {
        G->Padding = (int)((float)G->DefaultPadding * ((float)BufferHeight / G->DefaultDim.y));
        
        GUIComponent* Cursor = G->Buttons;
        while(Cursor != 0) {
            ResizeButton(G, Cursor, (Button*)Cursor->Data, BufferWidth, BufferHeight);
            Cursor = Cursor->Next;
        }
        Cursor = 0;
        
        Cursor = G->TextBoxes;
        while(Cursor != 0) {
            ResizeTextBox(G, Cursor, (TextBox*)Cursor->Data, BufferWidth, BufferHeight);
            Cursor = Cursor->Next;
        }
        Cursor = 0;
        
        Cursor = G->Texts;
        while(Cursor != 0) {
            ResizeText(G, Cursor, (Text*)Cursor->Data, BufferHeight);
            Cursor = Cursor->Next;
        }
        Cursor = 0;
        
        InitializeGUI(G);
    }
    
    G->Dim.x = 0;
    
    GUIComponent* Cursor = G->All;
    while(Cursor != 0)
    {
        Row* R = &G->Rows[(int)Cursor->GridCoords.y];
        
        // Calculate column location to center
        Cursor->Coords.x = (real32)((-R->Width)/2);
        for (int  i = 0; i < Cursor->GridCoords.x; i++)
            Cursor->Coords.x += G->Rows[(int)Cursor->GridCoords.y].ColumnWidths[i];
        
        // Find biggest row
        if (G->Dim.x < R->Width)
            G->Dim.x = (real32)R->Width;
        
        // Calculate row location to center
        Cursor->Coords.y = ((-G->Dim.y)/2) + ((R->Height - Cursor->Dim.y)/2);
        for (int i = 0; i < Cursor->GridCoords.y; i++) {
            Row* tempR = &G->Rows[i];
            Cursor->Coords.y += tempR->Height;
        }
        
        Cursor = Cursor->All;
    }
}

internal void
RenderGUI(GUI* G)
{
    // Render buttons
    GUIComponent* Cursor = G->Buttons;
    while(Cursor != 0)
    {
        Button* b = (Button*)Cursor->Data;
        
        v2 SDim = GetStringDimensions(&b->FontType, b->Text);
        v2 TextCoords = v2(Cursor->Coords.x + (int)((Cursor->Dim.x - SDim.x)/2),
                           Cursor->Coords.y + (int)((Cursor->Dim.y - SDim.y)/2));
        
        Push(RenderGroup, v3(Cursor->Coords, 0.0f), Cursor->Dim, b->Color, 0.0f);
        PrintOnScreen(&b->FontType, b->Text, TextCoords, b->TextColor);
        Cursor = Cursor->Next;
    }
    
    Cursor = 0;
    
    // Render TextBox
    Cursor = G->TextBoxes;
    while(Cursor != 0)
    {
        TextBox* b = (TextBox*)Cursor->Data;
        
        v2 SDim = GetStringDimensions(&b->FontType, b->Text);
        v2 TextCoords = v2(Cursor->Coords.x + (int)((Cursor->Dim.x - SDim.x)/2),
                           Cursor->Coords.y + (int)((Cursor->Dim.y - SDim.y)/2));
        
        Push(RenderGroup, v3(Cursor->Coords, 0.0f), Cursor->Dim,b->Color, 0.0f);
        PrintOnScreen(&b->FontType, b->Text, TextCoords, b->TextColor);
        if (b->ShowCursor == 1)
            Push(RenderGroup, v3(TextCoords.x + SDim.x, Cursor->Coords.y, 10.0f), 
                 v2(5.0f, Cursor->Dim.y), 0xFF000000, 0.0f);
        
        Cursor = Cursor->Next;
    }
    
    
    Cursor = G->Texts;
    while(Cursor != 0)
    {
        Text* b = (Text*)Cursor->Data;
        
        PrintOnScreen(&b->FontType, b->Text, Cursor->Coords, b->TextColor);
        Cursor = Cursor->Next;
    }
    Cursor = 0;
}

internal void
SetCursorMode(platform_input *Input, CursorMode CursorM)
{
    if (Input->Cursor != CursorM) {
        Input->Cursor = CursorM;
        Input->NewCursor = true;
    }
    else
        Input->NewCursor = false;
}

internal GUIEvents
HandleGUIEvents(GUI* G, platform_input *Input)
{
    GUIEvents Events = {};
    Events.BtnPressID = -1;
    Events.TbPressID = -1;
    
    if(Input->MouseButtons[0].NewEndedDown) {
        Events.BtnPressID = CheckButtonsClick(G, v2(Input->MouseX, Input->MouseY));
        Events.TbPressID = CheckTextBoxes(G, v2(Input->MouseX, Input->MouseY));
    }
    
    for (int i = 0; i < 10; i++) {
        if (Input->Keyboard.Numbers[i].NewEndedDown) {
            AddCharTextBoxText(G, IntToString(i));
        }
    }
    if (Input->Keyboard.Backspace.NewEndedDown)
        RemoveCharTextBoxText(G);
    if (Input->Keyboard.Period.NewEndedDown)
        AddCharTextBoxText(G, ".");
    
    if (CheckButtonsHover(G, v2(Input->MouseX, Input->MouseY)))
        SetCursorMode(Input, Hand);
    else
        SetCursorMode(Input, Arrow);
    
    return Events;
}