#include "gui.h"
#include "memorymanager.h"
#include "snake.h"

internal int
MouseInRect(int x, int y, int Width, int Height, int32 MouseX, int32 MouseY)
{
    if (x < MouseX && MouseX < (x + Width) &&
        y < MouseY && MouseY < (y + Height))
    {
        return 1;
    }
    return 0;
}


internal void
AddCharTextBoxText(GUI* G, char* Char)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0)
    {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->ShowCursor == 1)
        {
            TB->Text = StringConcat(TB->Text, Char);
        }
        
        Cursor = Cursor->Next;
    }
}

internal void
RemoveCharTextBoxText(GUI* G)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0)
    {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->ShowCursor == 1)
        {
            int Size = StringLength(TB->Text);
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
    while(Cursor != 0)
    {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->ID == ID)
        {
            TB->ShowCursor = 1;
        }
        else
        {
            TB->ShowCursor = 0;
        }
        
        Cursor = Cursor->Next;
    }
}

internal void
RenderLine(platform_offscreen_buffer *Buffer, v2 Point1, v2 Point2)
{
    real32 Slope = (Point2.y - Point1.y) / (Point2.x - Point1.x);
    real32 B = Point1.y - (Slope * Point1.x);
    
    for (int X = (int)Point1.x; X <= Point2.x; X++)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory + X * Buffer->BytesPerPixel + (int)Point1.y*Buffer->Pitch);
        
        for (int Y = (int)Point1.y; Y <= Point2.y; Y++)
        {
            
            *(uint32 *)Pixel = 0xFF000000;
            
            Pixel += Buffer->Pitch;
        }
    }
}

internal bool32
CheckButtonsHover(GUI* G, int32 MouseX, int32 MouseY)
{
    int InRect = 0;
    
    GUIComponent* Cursor = G->Buttons;
    while(Cursor != 0)
    {
        Button* B = (Button*)Cursor->Data;
        
        if (MouseInRect(Cursor->X, Cursor->Y, Cursor->Width, Cursor->Height, MouseX, MouseY))
        {
            B->Color = B->HoverColor;
            InRect = 1;
        }
        else
        {
            B->Color = B->RegularColor;
        }
        
        Cursor = Cursor->Next;
    }
    
    if (InRect == 1)
    {
        return true;
    }
    return false;
}

// Returns what button was pressed if one was.
internal int
CheckButtonsClick(GUI* G, int32 MouseX, int32 MouseY)
{
    GUIComponent* Cursor = G->Buttons;
    while(Cursor != 0)
    {
        if (MouseInRect(Cursor->X, Cursor->Y, Cursor->Width, Cursor->Height, MouseX, MouseY))
        {
            Button* btn = (Button*)Cursor->Data;
            return btn->ID;
        }
        
        Cursor = Cursor->Next;
    }
    
    return -1;
}

internal int
CheckTextBoxes(GUI* G, int32 MouseX, int32 MouseY)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0)
    {
        TextBox* btn = (TextBox*)Cursor->Data;
        btn->ShowCursor = 0;
        if (MouseInRect(Cursor->X, Cursor->Y, Cursor->Width, Cursor->Height, MouseX, MouseY))
        {
            return btn->ID;
        }
        
        Cursor = Cursor->Next;
    }
    
    return -1;
}

internal char*
IntToString(int Value)
{
    // TODO(quinn): Implement my own int to string? 
    // https://guide.handmadehero.org/code/day328/ casey does do it
    
    char Temp[10];
    sprintf(Temp, "%d", Value);
    char* Result = (char*)qalloc(Temp, 10);
    
    return Result;
}

internal bool
CompareStrings(char *c1, char* c2)
{
    int i = 0;
    while(c1[i] != 0 && c2[i] != 0)
    {
        if (c1[i] != c2[i])
        {
            return false;
        }
        
        i++;
    }
    
    return true;
}


internal int
StringLength(char* String)
{
    if(String == 0)
        return 0;
    
    int i = 0;
    while(String[i] != 0)
    {
        i++;
    }
    
    return i;
}

internal char*
StringConcat(char* Source, char* Add)
{
    int Size = StringLength(Source) + StringLength(Add) + 1;
    char* Result = (char*)qalloc(Source, Size);
    
    int i = StringLength(Source);
    int j = 0;
    while(Add[j] != 0)
    {
        Result[i] = Add[j];
        i++;
        j++;
    }
    Result[i] = 0;
    return Result;
}

internal int
AddGUIComponentAll(GUIComponent* Head, GUIComponent* NewComponent)
{
    // Start linked list
    if(Head == 0)
    {
        return 0;
    }
    
    GUIComponent* Cursor = Head;
    while(Cursor->All != 0)
    {
        Cursor = Cursor->All;
    }
    
    Cursor->All = NewComponent;
    return 1;
}

internal void
AddButton(GUI* G, int GridX, int GridY, int Width, int Height, Button* B)
{
    B->Color = B->RegularColor;
    GUIComponent NewTemp = {};
    GUIComponent* NewComponent = (GUIComponent*)qalloc((void*)&NewTemp, sizeof(GUIComponent));
    
    NewComponent->GridX = GridX;
    NewComponent->GridY = GridY;
    NewComponent->Width = Width;
    NewComponent->Height = Height;
    NewComponent->WidthP = Width + (G->Padding * 2);
    NewComponent->HeightP = Height + (G->Padding * 2);
    NewComponent->DefaultWidth = Width;
    NewComponent->DefaultHeight = Height;
    NewComponent->Data = qalloc((void*)B, sizeof(Button));
    
    *B = {}; // Resets struct used to pass this function information
    
    // Start linked list
    if(G->Buttons == 0)
    {
        G->Buttons = NewComponent;
    }
    else 
    {
        GUIComponent* Cursor = G->Buttons;
        while(Cursor->Next != 0)
        {
            Cursor = Cursor->Next;
        }
        Cursor->Next = NewComponent;
    }
    
    if (!AddGUIComponentAll(G->All, NewComponent))
    {
        G->All = NewComponent;
    }
}

internal void
AddText(GUI* G, int GridX, int GridY,  Text* T)
{
    GUIComponent NewTemp = {};
    GUIComponent* NewComponent = (GUIComponent*)qalloc((void*)&NewTemp, sizeof(GUIComponent));
    
    v2 StringDimension = GetStringDimensions(&T->FontType, T->Text);
    
    NewComponent->GridX = GridX;
    NewComponent->GridY = GridY;
    NewComponent->Width = (int)StringDimension.x;
    NewComponent->Height = (int)StringDimension.y;
    NewComponent->WidthP = (int)StringDimension.x + (G->Padding * 2);
    NewComponent->HeightP = (int)StringDimension.y + (G->Padding * 2);
    NewComponent->Data = qalloc((void*)T, sizeof(Text));
    
    *T = {}; // Resets struct used to pass this function information
    
    // Start linked list
    if(G->Texts == 0)
    {
        G->Texts = NewComponent;
    }
    else 
    {
        GUIComponent* Cursor = G->Texts;
        while(Cursor->Next != 0)
        {
            Cursor = Cursor->Next;
        }
        Cursor->Next = NewComponent;
    }
    
    if (!AddGUIComponentAll(G->All, NewComponent))
    {
        G->All = NewComponent;
    }
}

internal void
AddTextBox(GUI* G, int GridX, int GridY, int Width, int Height, TextBox* TB)
{
    GUIComponent NewTemp = {};
    GUIComponent* NewComponent = (GUIComponent*)qalloc((void*)&NewTemp, sizeof(GUIComponent));
    
    
    NewComponent->GridX = GridX;
    NewComponent->GridY = GridY;
    NewComponent->Width = Width;
    NewComponent->Height = Height;
    NewComponent->WidthP = Width + (G->Padding * 2);
    NewComponent->HeightP = Height + (G->Padding * 2);
    NewComponent->Data = qalloc((void*)TB, sizeof(TextBox));
    
    *TB = {}; // Resets struct used to pass this function information
    
    // Start linked list
    if(G->TextBoxes == 0)
    {
        G->TextBoxes = NewComponent;
    }
    else 
    {
        GUIComponent* Cursor = G->TextBoxes;
        while(Cursor->Next != 0)
        {
            Cursor = Cursor->Next;
        }
        Cursor->Next = NewComponent;
    }
    
    if (!AddGUIComponentAll(G->All, NewComponent))
    {
        G->All = NewComponent;
    }
}

internal char*
GetTextBoxText(GUI *G, int ID)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0)
    {
        
        
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->ID == ID)
        {
            return TB->Text; 
        }
        
        Cursor = Cursor->Next;
    }
    
    return 0;
}

internal void
ResizeButton(GUI* G, GUIComponent* C, Button* B, int Width, int Height)
{
    float Scale = stbtt_ScaleForPixelHeight(&B->FontType.Info,
                                            (float)B->FontType.ScaleIn * ((float)Height / (float)G->ClientHeight));
    B->FontType.Scale = Scale;
    
    C->Width = (int)((float)C->DefaultWidth * ((float)Width / (float)G->ClientWidth));
    C->WidthP = C->Width  + (G->Padding * 2);
    
    C->Height = (int)((float)C->DefaultHeight * ((float)Height / (float)G->ClientHeight));
    C->HeightP = C->Height  + (G->Padding * 2);
}

internal void
ResizeText(GUI* G, GUIComponent* C, Text* T, int Height)
{
    float Scale = stbtt_ScaleForPixelHeight(&T->FontType.Info,
                                            (float)T->FontType.ScaleIn * ((float)Height / (float)G->ClientHeight));
    T->FontType.Scale = Scale;
    
    v2 StringDimension = GetStringDimensions(&T->FontType, T->Text);
    
    C->WidthP = (int)StringDimension.x + (G->Padding * 2);
    C->HeightP = (int)StringDimension.y + (G->Padding * 2);
}

internal void
UpdateRow(Row* R, int Width, int Height, int GridX)
{
    R->Columns[GridX].Width = Width;
    R->Width += Width;
    if (R->Height < Height)
        R->Height = Height;
}

internal void
InitializeGUI(GUI* G)
{
    for (int i = 0; i < 10; i++)
    {
        G->Rows[i].Width = 0;
        G->Rows[i].Height = 0;
    }
    
    GUIComponent* Cursor = G->All;
    while(Cursor != 0)
    {
        UpdateRow(&G->Rows[Cursor->GridY], Cursor->WidthP, Cursor->HeightP, Cursor->GridX);
        Cursor = Cursor->All;
    }
    
    G->Height = 0;
    for (int i = 0; i < 10; i++)
    {
        G->Height += G->Rows[i].Height;
    }
    
}

internal void
UpdateGUI(GUI* G, int BufferWidth, int BufferHeight)
{
    // Resize GUI if screen size changed
    //if (G->ClientWidth != BufferWidth || G->ClientHeight != BufferHeight) 
    {
        G->Padding = (int)((float)G->DefaultPadding * ((float)BufferHeight / (float)G->ClientHeight));
        
        GUIComponent* Cursor = G->Buttons;
        while(Cursor != 0)
        {
            ResizeButton(G, Cursor, (Button*)Cursor->Data, BufferWidth, BufferHeight);
            Cursor = Cursor->Next;
        }
        Cursor = 0;
        
        Cursor = G->Texts;
        while(Cursor != 0)
        {
            ResizeText(G, Cursor, (Text*)Cursor->Data, BufferHeight);
            Cursor = Cursor->Next;
        }
        Cursor = 0;
        
        InitializeGUI(G);
    }
    
    G->Width = 0;
    
    GUIComponent* Cursor = G->All;
    while(Cursor != 0)
    {
        Row* R = &G->Rows[Cursor->GridY];
        
        // Calculate column location to center
        Cursor->X = ((-R->Width)/2);
        for (int  i = 0; i < Cursor->GridX; i++)
        {
            Column* tempC = &G->Rows[Cursor->GridY].Columns[i];
            Cursor->X += tempC->Width;
        }
        
        // Find biggest row
        if (G->Width < R->Width)
        {
            G->Width = R->Width;
        }
        
        // Calculate row location to center
        Cursor->Y = ((-G->Height)/2) + ((R->Height - Cursor->Height)/2);
        for (int i = 0; i < Cursor->GridY; i++)
        {
            Row* tempR = &G->Rows[i];
            Cursor->Y += tempR->Height;
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
        b->TextX = Cursor->X + (int)((Cursor->Width - SDim.x)/2);
        b->TextY = Cursor->Y + (int)((Cursor->Height - SDim.y)/2);
        //(int)(Cursor->Height / 2) + (int)(SDim.y / 2)
        
        //RenderRect(&R, FILL, b->Color);
        DrawRect(Cursor->X, Cursor->Y, Cursor->Width, Cursor->Height, b->Color);
        PrintOnScreen(&b->FontType, b->Text, b->TextX, b->TextY, b->TextColor);
        Cursor = Cursor->Next;
    }
    
    Cursor = 0;
    
    // Render TextBox
    Cursor = G->TextBoxes;
    while(Cursor != 0)
    {
        TextBox* b = (TextBox*)Cursor->Data;
        
        v2 SDim = GetStringDimensions(b->FontType, b->Text);
        b->TextX = Cursor->X + (int)((Cursor->Width - SDim.x)/2);
        b->TextY = Cursor->Y + (int)((Cursor->Height - SDim.y)/2);
        
        DrawRect(Cursor->X, Cursor->Y, Cursor->Width, Cursor->Height, b->Color);
        PrintOnScreen(b->FontType, b->Text, b->TextX, b->TextY, b->TextColor);
        if (b->ShowCursor == 1)
        {
            //v2 p1 = {EndOfText.Top.x, EndOfText.Top.y};
            //v2 p2 = {EndOfText.Top.x, EndOfText.Top.y + EndOfText.Height};
            //RenderLine(Buffer,p1, p2);
        }
        
        Cursor = Cursor->Next;
    }
    
    
    Cursor = G->Texts;
    while(Cursor != 0)
    {
        Text* b = (Text*)Cursor->Data;
        
        PrintOnScreen(&b->FontType, b->Text, Cursor->X, Cursor->Y, b->TextColor);
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
    
    if(Input->MouseButtons[0].EndedDown) {
        Events.BtnPressID = CheckButtonsClick(G, Input->MouseX, Input->MouseY);
        Events.TbPressID = CheckTextBoxes(G, Input->MouseX, Input->MouseY);
    }
    
    if (CheckButtonsHover(G, Input->MouseX, Input->MouseY))
        SetCursorMode(Input, Hand);
    else
        SetCursorMode(Input, Arrow);
    
    return Events;
}