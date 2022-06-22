#ifndef GUI_H
#define GUI_H

#ifndef TEXT_H
#pragma message ("gui.h requires text.h")
#endif

/*
struct Button
{
    char* Text;
    Font *FontType;
    
    int ID;
    uint32 Color;
    uint32 RegularColor;
    uint32 HoverColor;
    
    uint32 TextColor;
    uint32 RegularTextColor;
    uint32 HoverTextColor;
};

struct gui_logo
{
    Texture *Tex;
};

struct Text
{
    char* Text;
    int ID;
    Font *FontType;
    uint32 TextColor;
};

struct TextBox
{
    char* Text;
    Font *FontType;
    v2 TextCoords;
    int ID;
    
    real32 Active;
    int MaxTextLength = 100;
    real32 XAdvances[100];
    int CursorPosition; // What char to print the cursor after (0 is front, 1 after first char)
    
    uint32 Color;
    uint32 TextColor;
};
inline void IncrementCursorPosition(TextBox *TB)
{
    if (TB->CursorPosition < GetLength(TB->Text))
        TB->CursorPosition++;
}
inline void DecrementCursorPosition(TextBox *TB)
{
    if (TB->CursorPosition != 0)
        TB->CursorPosition--;
}

//typedef int type;
struct GUIComponent
{
    enum type
    {
        button,
        text,
        textbox,
        logo,
    };
    type Type;
    
    v2 Coords;
    v2 GridCoords;
    v2 Dim;
    v2 PaddingDim;
    v2 DefaultDim;
    
    GUIComponent *AlignWith;
    
    GUIComponent* Next;
    GUIComponent* All;
    void* Data;
};

struct Row
{
    int Width = 0;
    int Height = 0;
    int ColumnWidths[10];
};
struct GUIEvents
{
    int BtnPressID;
    int TbPressID;
    bool32 BtnHoverFlag;
};
struct GUI
{
    int Initialized;
    Row Rows[10];
    
    v2 Coords;
    v2 Dim;
    int Padding = 0;
    
    Arr Components; // GUIComponent
    GUIComponent *Buttons;
    GUIComponent *TextBoxes;
    GUIComponent *Texts;
    GUIComponent *Logos;
    
    GUIEvents Events;
    
    int Screen;
    
    uint32 BackgroundColor = 0;
    
    v2 DefaultDim;
    int DefaultPadding = 0;
};
*/


inline int MouseInRect(v2 RectCoords, v2 RectDim, v2 MouseCoords)
{
    if (RectCoords.x < MouseCoords.x && MouseCoords.x < (RectCoords.x + RectDim.x) &&
        RectCoords.y < MouseCoords.y && MouseCoords.y < (RectCoords.y + RectDim.y))
        return 1;
    
    return 0;
}

internal void
SetCursorMode(platform_input *Input, platform_cursor_mode CursorM)
{
    if (Input->Cursor != CursorM) {
        Input->Cursor = CursorM;
        Input->NewCursor = true;
    }
    else
        Input->NewCursor = false;
}

internal void
TextBoxMoveCursorArrowKeys(GUI *G, menu_direction Dir)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0) {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->Active) {
            if (Dir == Right)
                IncrementCursorPosition(TB);
            else if (Dir == Left)
                DecrementCursorPosition(TB);
        }
        Cursor = Cursor->Next;
    }
}

internal void
TextBoxLoadXAdvances(TextBox *TB)
{
    for (int i = 0; i < GetLength(TB->Text); i++)
    {
        FontChar *NextChar = LoadFontChar(TB->FontType,  TB->Text[i], 0xFF000000);
        
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&TB->FontType->Info, TB->Text[i], &ax, &lsb);
        
        // add kerning
        int kern = stbtt_GetCodepointKernAdvance(&TB->FontType->Info, TB->Text[i], TB->Text[i + 1]);
        TB->XAdvances[i] = ((kern + ax) * TB->FontType->Scale);
    }
}

internal void
TextBoxMoveCursorMouse(GUI *G, v2 MouseCoords)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0) {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->Active) {
            
            v2 Coords = Cursor->Coords;
            
            int StringLength = GetLength(TB->Text);
            
            v2 CharCoords[100];
            CharCoords[0] = v2(TB->TextCoords.x, TB->TextCoords.y);
            
            TextBoxLoadXAdvances(TB);
            
            real32 ClosestX = TB->TextCoords.x;
            real32 CharX = TB->TextCoords.x;
            int Closest = 0;
            for (int i = 0; i < StringLength; i++) {
                CharX += TB->XAdvances[i];
                real32 DiffClosest = fabsf(ClosestX - MouseCoords.x);
                real32 DiffCurrent = fabsf(CharX - MouseCoords.x);
                if (DiffCurrent < DiffClosest) {
                    ClosestX = CharX;
                    Closest = i + 1;
                }
            }
            
            TB->CursorPosition = Closest;
        }
        Cursor = Cursor->Next;
    }
}

internal void
AddCharTextBoxText(GUI* G, char* Char)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0) {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->Active && GetLength(TB->Text) + 1 < TB->MaxTextLength) {
            TB->Text = Insert(TB->Text, TB->CursorPosition, Char);
            IncrementCursorPosition(TB);
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
        if (TB->Active && TB->CursorPosition != 0) {
            DecrementCursorPosition(TB);
            TB->Text = RemoveAt(TB->Text, TB->CursorPosition);
        }
        Cursor = Cursor->Next;
    }
}

internal const char*
GetTextBoxText(GUI* G, int ID)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0) {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->ID == ID) {
            return TB->Text;
        }
        Cursor = Cursor->Next;
    }
    
    return 0;
}

internal void
TextBoxAddText(GUI *G, char *NewText)
{
    GUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0) {
        TextBox* TB = (TextBox*)Cursor->Data;
        if (TB->Active && GetLength(TB->Text) + GetLength(NewText) < TB->MaxTextLength) {
            TB->Text = Insert(TB->Text, TB->CursorPosition, NewText);
            TB->CursorPosition += GetLength(NewText);
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
            B->TextColor = B->HoverTextColor;
            InRect = 1;
        }
        else {
            B->Color = B->RegularColor;
            B->TextColor = B->RegularTextColor;
        }
        
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
        TextBox* TB = (TextBox*)Cursor->Data;
        
        if (MouseInRect(Cursor->Coords, Cursor->Dim, MouseCoords))
            TB->Active = true;
        else
            TB->Active = false;
        
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
/*
internal void
GUIAddButton(gui *GUI, v2 GridCoords, v2 Dim, gui_button *Button)
{
    //
}
*/
internal GUIComponent*
AddText(GUI* G, v2 GridCoords,  Text T, GUIComponent *Align)
{
    GUIComponent* NewComponent = (GUIComponent*)qalloc(sizeof(GUIComponent));
    v2 StringDimension = GetStringDimensions(T.FontType, T.Text);
    NewComponent->GridCoords = GridCoords;
    NewComponent->Dim = StringDimension;
    NewComponent->DefaultDim = NewComponent->Dim;
    NewComponent->PaddingDim = v2(StringDimension.x + (G->Padding * 2), StringDimension.y + (G->Padding * 2));
    NewComponent->Type = GUIComponent::type::text;
    NewComponent->Data = qalloc((void*)&T, sizeof(Text));
    
    NewComponent->AlignWith = Align;
    
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
    
    return (GUIComponent*)G->Components.Last();
}

inline GUIComponent* AddText(GUI* G, v2 GridCoords,  Text T) { return AddText(G, GridCoords, T, 0); }

internal GUIComponent*
AddTextBox(GUI* G, v2 GridCoords, v2 Dim, TextBox TB, GUIComponent *Align)
{
    GUIComponent* NewComponent = (GUIComponent*)qalloc(sizeof(GUIComponent));
    
    NewComponent->GridCoords = GridCoords;
    NewComponent->Dim = Dim;
    NewComponent->PaddingDim = v2(Dim.x + (G->Padding * 2), Dim.y + (G->Padding * 2));
    NewComponent->DefaultDim = Dim;
    NewComponent->Type = GUIComponent::type::textbox;
    NewComponent->Data = qalloc((void*)&TB, sizeof(TextBox));
    
    NewComponent->AlignWith = Align;
    
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
    
    return (GUIComponent*)G->Components.Last();
}

internal void
GUIAddLogo(GUI *G, v2 GridCoords, v2 Dim, gui_logo Logo)
{
    GUIComponent* NewComponent = (GUIComponent*)qalloc(sizeof(GUIComponent));
    
    NewComponent->GridCoords = GridCoords;
    NewComponent->Dim = Dim;
    NewComponent->PaddingDim = v2(Dim.x + (G->Padding * 2), Dim.y + (G->Padding * 2));
    NewComponent->DefaultDim = Dim;
    NewComponent->Type = GUIComponent::type::logo;
    NewComponent->Data = qalloc((void*)&Logo, sizeof(TextBox));
    
    G->Components.Push(NewComponent);
    
    // Start linked list
    if(G->Logos == 0)
        G->Logos = (GUIComponent*)G->Components.Last();
    else {
        GUIComponent* Cursor = G->TextBoxes;
        while(Cursor->Next != 0)
            Cursor = Cursor->Next;
        
        Cursor->Next = (GUIComponent*)G->Components.Last();
    }
}

internal GUIEvents*
HandleGUIEvents(GUI* G, platform_input *Input)
{
    //GUIEvents Events = {};
    G->Events.BtnPressID = -1;
    G->Events.TbPressID = -1;
    
    v2 MouseCoords = v2(Input->MouseX, Input->MouseY);
    if(Input->MouseButtons[0].NewEndedDown) {
        G->Events.BtnPressID = CheckButtonsClick(G, MouseCoords);
        G->Events.TbPressID = CheckTextBoxes(G, MouseCoords);
    }
    if (Input->MouseButtons[0].EndedDown) {
        TextBoxMoveCursorMouse(G, MouseCoords);
    }
    
    for (int i = 0; i < 10; i++) {
        if (Input->Keyboard.Numbers[i].NewEndedDown)
            AddCharTextBoxText(G, IntToString(i));
    }
    
    if (Input->Keyboard.Left.NewEndedDown)
        TextBoxMoveCursorArrowKeys(G, Left);
    if (Input->Keyboard.Right.NewEndedDown)
        TextBoxMoveCursorArrowKeys(G, Right);
    if (Input->Keyboard.CtrlV.NewEndedDown)
        TextBoxAddText(G, Input->Keyboard.Clipboard);
    
    if (Input->Keyboard.Backspace.NewEndedDown)
        RemoveCharTextBoxText(G);
    if (Input->Keyboard.Period.NewEndedDown)
        AddCharTextBoxText(G, ".");
    
    if (CheckButtonsHover(G, v2(Input->MouseX, Input->MouseY)))
        SetCursorMode(Input, Hand);
    else
        SetCursorMode(Input, Arrow);
    
    return &G->Events;
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
        else if (C->Type == GUIComponent::type::logo) {
            C->Dim = C->DefaultDim * v2(BufferDim.y / G->DefaultDim.y, BufferDim.y / G->DefaultDim.y);
            C->PaddingDim = C->Dim + v2(G->Padding * 2, G->Padding * 2);
        }
    }
    
    // Setting up rows
    memset(G->Rows, 0, sizeof(Row) * 10);
    for (int i = 0; i < G->Components.Size; i++) {
        GUIComponent* C = (GUIComponent*)G->Components[i];
        Row* R = &G->Rows[(int)C->GridCoords.y];
        int Width = 0;
        
        if (C->AlignWith == 0) 
            Width = (int)C->PaddingDim.x;
        else if (C->AlignWith != 0)
            Width = (int)C->AlignWith->PaddingDim.x;
        
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
        if (G->Dim.x < R->Width) {
            G->Dim.x = (real32)R->Width;
            G->Coords.x = C->Coords.x - (G->Padding);
        }
        // Calculate row location to center
        C->Coords.y = ((-G->Dim.y)/2) + ((R->Height - C->PaddingDim.y)/2);
        for (int i = 0; i < C->GridCoords.y; i++) {
            Row* tempR = &G->Rows[i];
            C->Coords.y += tempR->Height;
        }
    }
    
    GUIComponent* C = (GUIComponent*)G->Components[0];
    G->Coords.y = C->Coords.y - (G->Padding);
}

internal void
RenderGUI(GUI* G)
{
    for (int i = 0; i < G->Components.Size; i++) {
        GUIComponent* Component = (GUIComponent*)G->Components[i];
        
        v2 Padding = (Component->Dim - Component->PaddingDim)/2;
        
        if (Component->Type == GUIComponent::type::button) {
            Button* b = (Button*)Component->Data;
            
            v2 SDim = GetStringDimensions(b->FontType, b->Text);
            v2 TextCoords = Component->Coords + v2((Component->Dim.x - SDim.x)/2, (Component->Dim.y - SDim.y)/2);
            
            Push(RenderGroup, v3(Component->Coords - Padding, 100.0f), Component->Dim, b->Color, 0.0f);
            PrintOnScreen(b->FontType, b->Text, TextCoords - Padding, b->TextColor);
        }
        else if (Component->Type == GUIComponent::type::textbox) {
            TextBox* b = (TextBox*)Component->Data;
            
            v2 SDim = GetStringDimensions(b->FontType, b->Text);
            //b->TextCoords = Component->Coords + v2((Component->Dim.x - SDim.x)/2, (Component->Dim.y - SDim.y)/2);
            
            b->TextCoords = Component->Coords + v2(0.0f ,(Component->Dim.y - SDim.y)/2);
            real32 OutOfBounds = (SDim.x + (Component->Dim.y/4)) - (Component->Dim.x);
            if (OutOfBounds >= 0) {
                b->TextCoords.x -= SDim.x - (Component->Dim.x);
                b->TextCoords.x -= (Component->Dim.y)/4;
            }
            else {
                b->TextCoords.x += (Component->Dim.y)/4;
            }
            
            Push(RenderGroup, v3(Component->Coords - Padding, 50.0f), Component->Dim, b->Color, 0.0f);
            if (b->Active) {
                TextBoxLoadXAdvances(b);
                real32 CursorX = b->TextCoords.x;
                for (int i = 0; i <  b->CursorPosition; i++) {
                    CursorX += b->XAdvances[i];
                }
                
                if (CursorX < Component->Coords.x) {
                    real32 TempCursorX = CursorX;
                    int i = 0;
                    while (TempCursorX < Component->Coords.x) {
                        TempCursorX += b->XAdvances[i];
                    }
                    real32 Diff = TempCursorX - CursorX;
                    CursorX = TempCursorX;
                    b->TextCoords.x += Diff;
                }
                
                Push(RenderGroup, v3(CursorX - Padding.x, Component->Coords.y - Padding.y, 100.0f), v2(5.0f, Component->Dim.y), 0xFF000000, 0.0f);
            }
            PrintOnScreen(b->FontType, b->Text, b->TextCoords - Padding, b->TextColor, Component->Coords - Padding, Component->Dim);
        }
        else if (Component->Type == GUIComponent::type::text) {
            Text* b = (Text*)Component->Data;
            if (Component->AlignWith != 0)
                Component->Coords.x = Component->Coords.x + Component->AlignWith->Dim.x - Component->Dim.x;
            PrintOnScreen(b->FontType, b->Text, Component->Coords - Padding, b->TextColor);
        }
        else if (Component->Type == GUIComponent::type::logo) {
            gui_logo* Logo = (gui_logo*)Component->Data;
            Push(RenderGroup, v3(Component->Coords - Padding, 100.0f), Component->Dim, Logo->Tex, 0.0f, BlendMode::gl_src_alpha);
        }
    }
    
    Push(RenderGroup, v3(G->Coords, 50.0f), G->Dim + (G->Padding * 2), G->BackgroundColor, 0.0f);
}

#endif //GUI_H
