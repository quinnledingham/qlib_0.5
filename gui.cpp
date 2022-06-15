#include "gui.h"
#include "memorymanager.h"

internal int
MouseInRect(v2 RectCoords, v2 RectDim, v2 MouseCoords)
{
    if (RectCoords.x < MouseCoords.x && MouseCoords.x < (RectCoords.x + RectDim.x) &&
        RectCoords.y < MouseCoords.y && MouseCoords.y < (RectCoords.y + RectDim.y))
        return 1;
    
    return 0;
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

internal void
AddCharTextBoxText(GUI* G, char* Char)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0) {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->Active) {
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
        if (TB->Active) {
            int Size = Length(TB->Text);
            char* Result = (char*)qalloc(TB->Text, Size);
            Result[Size - 1] = 0;
            TB->Text = Result;
        }
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
            btn->Active = true;
        else
            btn->Active = false;
        
        Cursor = Cursor->Next;
    }
    
    return -1;
}

internal void
AddButton(GUI* G, v2 GridCoords, v2 Dim, Button B)
{
    GUIComponent* NewComponent = (GUIComponent*)qalloc(sizeof(GUIComponent));
    NewComponent->GridCoords = GridCoords;
    NewComponent->Dim = Dim;
    NewComponent->PaddingDim = v2(Dim.x + (G->Padding * 2), Dim.y + (G->Padding * 2));
    NewComponent->DefaultDim = NewComponent->Dim;
    NewComponent->Type = GUIComponent::type::button;
    
    B.Color = B.RegularColor;
    NewComponent->Data = qalloc((void*)&B, sizeof(Button));
    
    G->Components.Push(NewComponent);
    
    // Start linked list
    if(G->Buttons == 0)
        G->Buttons = (GUIComponent*)G->Components.Last();
    else {
        GUIComponent* Cursor = G->Buttons;
        while(Cursor->Next != 0)
            Cursor = Cursor->Next;
        Cursor->Next = (GUIComponent*)G->Components.Last();
    }
}

internal void
AddText(GUI* G, v2 GridCoords,  Text T)
{
    GUIComponent* NewComponent = (GUIComponent*)qalloc(sizeof(GUIComponent));
    v2 StringDimension = GetStringDimensions(T.FontType, T.Text);
    NewComponent->GridCoords = GridCoords;
    NewComponent->Dim = StringDimension;
    NewComponent->DefaultDim = NewComponent->Dim;
    NewComponent->PaddingDim = v2(StringDimension.x + (G->Padding * 2), StringDimension.y + (G->Padding * 2));
    NewComponent->Type = GUIComponent::type::text;
    NewComponent->Data = qalloc((void*)&T, sizeof(Text));
    
    G->Components.Push(NewComponent);
    
    // Start linked list
    if(G->Texts == 0)
        G->Texts = (GUIComponent*)G->Components.Last();
    else {
        GUIComponent* Cursor = G->Texts;
        while(Cursor->Next != 0)
            Cursor = Cursor->Next;
        
        Cursor->Next = (GUIComponent*)G->Components.Last();
    }
}

internal void
AddTextBox(GUI* G, v2 GridCoords, v2 Dim, TextBox TB)
{
    GUIComponent* NewComponent = (GUIComponent*)qalloc(sizeof(GUIComponent));
    
    NewComponent->GridCoords = GridCoords;
    NewComponent->Dim = Dim;
    NewComponent->PaddingDim = v2(Dim.x + (G->Padding * 2), Dim.y + (G->Padding * 2));
    NewComponent->DefaultDim = Dim;
    NewComponent->Type = GUIComponent::type::textbox;
    NewComponent->Data = qalloc((void*)&TB, sizeof(TextBox));
    
    G->Components.Push(NewComponent);
    
    // Start linked list
    if(G->TextBoxes == 0)
        G->TextBoxes = (GUIComponent*)G->Components.Last();
    else {
        GUIComponent* Cursor = G->TextBoxes;
        while(Cursor->Next != 0)
            Cursor = Cursor->Next;
        
        Cursor->Next = (GUIComponent*)G->Components.Last();
    }
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

internal void
UpdateGUI(GUI* G, v2 BufferDim)
{
    // Resize GUI if screen size changed
    //if (G->ClientWidth != BufferWidth || G->ClientHeight != BufferHeight) 
    G->Padding = (int)((float)G->DefaultPadding * (BufferDim.y / G->DefaultDim.y));
    
    // Resizing components
    for (int i = 0; i < G->Components.Size; i++) {
        GUIComponent* C = (GUIComponent*)G->Components[i];
        
        if (C->Type == GUIComponent::type::button) {
            Button *B = (Button*)C->Data;
            real32 FontHeight = B->FontType->ScaleIn * (BufferDim.y / G->DefaultDim.y);
            B->FontType->Scale = stbtt_ScaleForPixelHeight(&B->FontType->Info, FontHeight);
            C->Dim = C->DefaultDim * v2(BufferDim.x / G->DefaultDim.x, BufferDim.y / G->DefaultDim.y);
            C->PaddingDim = C->Dim + v2(G->Padding * 2, G->Padding * 2);
        }
        else if (C->Type == GUIComponent::type::textbox) {
            TextBox *Tb = (TextBox*)C->Data;
            real32 FontHeight = Tb->FontType->ScaleIn * (BufferDim.y / G->DefaultDim.y);
            Tb->FontType->Scale = stbtt_ScaleForPixelHeight(&Tb->FontType->Info, FontHeight);
            C->Dim = C->DefaultDim * v2(BufferDim.x / G->DefaultDim.x, BufferDim.y / G->DefaultDim.y);
            C->PaddingDim = C->Dim + v2(G->Padding * 2, G->Padding * 2);
        }
        else if (C->Type == GUIComponent::type::text) {
            Text *T = (Text*)C->Data;
            real32 FontHeight = T->FontType->ScaleIn * (BufferDim.y / G->DefaultDim.y);
            T->FontType->Scale = stbtt_ScaleForPixelHeight(&T->FontType->Info, FontHeight);
            v2 StringDimension = GetStringDimensions(T->FontType, T->Text);
            C->Dim = StringDimension;
            C->PaddingDim = StringDimension + v2(G->Padding * 2, G->Padding * 2);
        }
    }
    
    // Setting up rows
    memset(G->Rows, 0, sizeof(Row) * 10);
    for (int i = 0; i < G->Components.Size; i++) {
        GUIComponent* C = (GUIComponent*)G->Components[i];
        Row* R = &G->Rows[(int)C->GridCoords.y];
        int Width = (int)C->PaddingDim.x;
        int Height = (int)C->PaddingDim.y;
        int GridX = (int)C->GridCoords.x;
        
        R->ColumnWidths[GridX] = Width;
        R->Width += Width;
        if (R->Height < Height)
            R->Height = Height;
    }
    
    // Height of GUI
    G->Dim.y = 0;
    for (int i = 0; i < 10; i++)
        G->Dim.y += G->Rows[i].Height;
    
    G->Dim.x = 0;
    for (int i = 0; i < G->Components.Size; i++) {
        GUIComponent* C = (GUIComponent*)G->Components[i];
        Row* R = &G->Rows[(int)C->GridCoords.y];
        
        // Calculate column location to center
        C->Coords.x = (real32)((-R->Width)/2);
        for (int  i = 0; i < C->GridCoords.x; i++)
            C->Coords.x += G->Rows[(int)C->GridCoords.y].ColumnWidths[i];
        
        // Find biggest row
        if (G->Dim.x < R->Width)
            G->Dim.x = (real32)R->Width;
        
        // Calculate row location to center
        C->Coords.y = ((-G->Dim.y)/2) + ((R->Height - C->Dim.y)/2);
        for (int i = 0; i < C->GridCoords.y; i++) {
            Row* tempR = &G->Rows[i];
            C->Coords.y += tempR->Height;
        }
    }
}

internal void
RenderGUI(GUI* G)
{
    for (int i = 0; i < G->Components.Size; i++) {
        GUIComponent* Component = (GUIComponent*)G->Components[i];
        if (Component->Type == GUIComponent::type::button) {
            Button* b = (Button*)Component->Data;
            
            v2 SDim = GetStringDimensions(b->FontType, b->Text);
            v2 TextCoords = Component->Coords + v2((Component->Dim.x - SDim.x)/2, (Component->Dim.y - SDim.y)/2);
            
            Push(RenderGroup, v3(Component->Coords, 0.0f), Component->Dim, b->Color, 0.0f);
            PrintOnScreen(b->FontType, b->Text, TextCoords, b->TextColor);
        }
        else if (Component->Type == GUIComponent::type::textbox) {
            TextBox* b = (TextBox*)Component->Data;
            
            v2 SDim = GetStringDimensions(b->FontType, b->Text);
            v2 TextCoords = Component->Coords + v2((Component->Dim.x - SDim.x)/2, (Component->Dim.y - SDim.y)/2);
            
            Push(RenderGroup, v3(Component->Coords, 0.0f), Component->Dim,b->Color, 0.0f);
            PrintOnScreen(b->FontType, b->Text, TextCoords, b->TextColor);
            if (b->Active)
                Push(RenderGroup, v3(TextCoords.x + SDim.x, Component->Coords.y, 10.0f), 
                     v2(5.0f, Component->Dim.y), 0xFF000000, 0.0f);
        }
        else if (Component->Type == GUIComponent::type::text) {
            Text* b = (Text*)Component->Data;
            PrintOnScreen(b->FontType, b->Text, Component->Coords, b->TextColor);
        }
    }
}