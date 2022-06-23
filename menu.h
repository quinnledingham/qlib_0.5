#ifndef MENU_H
#define MENU_H

enum struct menu_direction
{
    Right,
    Up,
    Left,
    Down,
};

struct menu_button
{
    font_string FontString;
    real32 DefaultTextPixelHeight;
    
    uint32 CurrentColor;
    uint32 DefaultColor;
    uint32 HoverColor;
    
    //uint32 CurrentTextColor;
    uint32 DefaultTextColor;
    uint32 HoverTextColor;
};

struct menu_logo
{
    Texture *Tex;
};

struct menu_text
{
    font_string FontString;
    uint32 DefaultTextColor;
};

struct menu_textbox
{
    font_string FontString;
    v2 TextCoords;
    
    bool Active;
    int MaxTextLength = 100;
    real32 XAdvances[100];
    int CursorPosition;
    
    uint32 CurrentColor;
    uint32 CurrentTextColor;
};

//enum menu_component_id;
enum struct menu_component_type
{
    Button,
    Text,
    TextBox,
    Logo,
};
struct menu_component
{
    int ID;
    
    v2 Coords;
    v2 GridCoords;
    v2 Dim;
    v2 DefaultDim;
    real32 DefaultTextPixelHeight;
    v2 PaddingDim;
    
    bool Active;
    
    menu_component *AlignWith;
    
    menu_component *ActivePrevious;
    menu_component *ActiveNext;
    
    menu_component *NextSameType;
    menu_component_type Type;
    void *Data;
};

#define GetComponentData(d, t) ((t*)d->Data)

enum struct menu_input_mode
{
    Controller,
    Keyboard,
    Mouse,
};
struct menu_events
{
    int ButtonClicked;
    bool ButtonHoverFlag;
};

struct menu_grid_row
{
    v2 Dim;
    real32 MenuGridColumnWidth[10];
};
struct menu
{
    bool Initialized;
    menu_grid_row Rows[10];
    
    v2 Coords;
    v2 Dim;
    
    v2 DefaultDim;
    v2 ScreenDim;
    
    int Padding;
    int DefaultPadding;
    
    int NumOfComponents;
    int MaxNumOfComponents;
    menu_component Components[20];
    menu_component *Buttons;
    menu_component *TextBoxes;
    menu_component *Texts;
    menu_component *Logos;
    
    menu_events Events;
    
    menu_component *Active;
    menu_component *ActiveStart;
    
    menu_input_mode InputMode;
    
    uint32 BackgroundColor;
};

inline bool CoordsInRect( v2 Coords, v2 RectCoords, v2 RectDim)
{
    if (RectCoords.x < Coords.x && Coords.x < (RectCoords.x + RectDim.x) &&
        RectCoords.y < Coords.y && Coords.y < (RectCoords.y + RectDim.y))
        return true;
    
    return false;
}

internal menu_component*
MenuAddToComponentList(menu_component *Start, menu_component *NewComponent)
{
    if(Start == 0)
        Start = NewComponent;
    else {
        menu_component* Cursor = Start;
        while(Cursor->NextSameType != 0)
            Cursor = Cursor->NextSameType;
        Cursor->NextSameType = (menu_component*)NewComponent;
    }
    
    return Start;
}

internal menu_component*
MenuGetNextComponent(menu *Menu)
{
    Assert(Menu->NumOfComponents + 1 < Menu->MaxNumOfComponents);
    return &Menu->Components[Menu->NumOfComponents++];
}

// menu_button
internal int
MenuButtonClicked(menu *Menu, v2 MouseCoords)
{
    menu_component* Cursor = Menu->Buttons;
    while(Cursor != 0) {
        if (CoordsInRect(MouseCoords, Cursor->Coords, Cursor->Dim)) {
            return Cursor->ID;
        }
        Cursor = Cursor->NextSameType;
    }
    return -1;
}

internal bool
MenuButtonHovered(menu *Menu, v2 MouseCoords)
{
    bool ButtonHovered = false;
    
    menu_component* Cursor = Menu->Buttons;
    while(Cursor != 0) {
        menu_button *Button = (menu_button*)Cursor->Data;
        if (CoordsInRect(MouseCoords, Cursor->Coords, Cursor->Dim)) {
            Button->CurrentColor = Button->HoverColor;
            Button->FontString.Color = Button->HoverTextColor;
            ButtonHovered = true;
        }
        else {
            Button->CurrentColor = Button->DefaultColor;
            Button->FontString.Color = Button->DefaultTextColor;
        }
        Cursor = Cursor->NextSameType;
    }
    
    return ButtonHovered;
}

internal void
MenuResizeButton(menu *Menu, menu_component *MenuComponent, v2 ResizeFactors)
{
    menu_button *Button = (menu_button*)MenuComponent->Data;
    FontStringResize(&Button->FontString, ResizeEquivalentAmount(MenuComponent->DefaultTextPixelHeight, ResizeFactors.y));
    MenuComponent->Dim = ResizeEquivalentAmount(MenuComponent->DefaultDim, ResizeFactors);
    MenuComponent->PaddingDim = MenuComponent->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddButton(menu *Menu, int ID, v2 GridCoords, v2 Dim, menu_button *Button)
{
    Button->FontString.Color = Button->DefaultTextColor;
    FontStringInit(&Button->FontString);
    
    menu_component *MenuComponent = MenuGetNextComponent(Menu);
    MenuComponent->GridCoords = GridCoords;
    MenuComponent->Dim = Dim;
    MenuComponent->DefaultDim = MenuComponent->Dim;
    MenuComponent->DefaultTextPixelHeight = Button->FontString.PixelHeight;
    MenuComponent->Type = menu_component_type::Button;
    MenuComponent->ID = ID;
    
    Button->CurrentColor = Button->DefaultColor;
    
    MenuComponent->Data = qalloc((void*)Button, sizeof(menu_button));
    MenuResizeButton(Menu, MenuComponent, 0);
    
    
    Menu->Buttons = MenuAddToComponentList(Menu->Buttons, MenuComponent);
    return MenuComponent;
}

// menu_text
internal void
MenuResizeText(menu *Menu, menu_component *MenuComponent, v2 ResizeFactors)
{
    menu_text *Text = (menu_text*)MenuComponent->Data;
    MenuComponent->Dim = FontStringResize(&Text->FontString, ResizeEquivalentAmount(MenuComponent->DefaultTextPixelHeight, ResizeFactors.y));
    MenuComponent->PaddingDim = MenuComponent->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddText(menu *Menu, int ID, v2 GridCoords, menu_text *Text, menu_component *Align)
{
    Text->FontString.Color = Text->DefaultTextColor;
    FontStringInit(&Text->FontString);
    
    menu_component *MenuComponent = MenuGetNextComponent(Menu);
    MenuComponent->GridCoords = GridCoords;
    MenuComponent->Dim = FontStringGetDim(&Text->FontString);
    MenuComponent->DefaultDim = MenuComponent->Dim;
    MenuComponent->DefaultTextPixelHeight = Text->FontString.PixelHeight;
    MenuComponent->AlignWith = Align;
    MenuComponent->Type = menu_component_type::Text;
    MenuComponent->ID = ID;
    
    MenuComponent->Data = qalloc((void*)Text, sizeof(menu_text));
    MenuResizeText(Menu, MenuComponent, 0);
    
    Menu->Texts = MenuAddToComponentList(Menu->Texts, MenuComponent);
    return MenuComponent;
}
inline menu_component* MenuAddText(menu *Menu, int ID, v2 GridCoords, menu_text *Text)
{
    return MenuAddText(Menu, ID, GridCoords, Text, 0);
}
inline menu_component* MenuAddText(menu *Menu, v2 GridCoords, menu_text *Text, menu_component *Align)
{
    return MenuAddText(Menu, -1, GridCoords, Text, Align);
}
inline menu_component* MenuAddText(menu *Menu, v2 GridCoords, menu_text *Text)
{
    return MenuAddText(Menu, -1, GridCoords, Text, 0);
}

// menu_textbox
inline void IncrementCursorPosition(menu_textbox *TextBox)
{
    if (TextBox->CursorPosition < TextBox->FontString.Length)
        TextBox->CursorPosition++;
}
inline void DecrementCursorPosition(menu_textbox *TextBox)
{
    if (TextBox->CursorPosition != 0)
        TextBox->CursorPosition--;
}

internal int
MenuTextBoxClicked(menu *Menu, v2 MouseCoords)
{
    int ID = -1;
    
    menu_component* Cursor = Menu->TextBoxes;
    while(Cursor != 0) {
        menu_textbox *TextBox = (menu_textbox*)Cursor->Data;
        if (CoordsInRect(MouseCoords, Cursor->Coords, Cursor->Dim)) {
            TextBox->Active = true;
            ID = Cursor->ID;
        }
        else
            TextBox->Active = false;
        Cursor = Cursor->NextSameType;
    }
    
    return ID;
}

internal menu_component*
MenuTextBoxGetActive(menu_component* Cursor)
{
    while(Cursor != 0) {
        menu_textbox *TextBox = (menu_textbox*)Cursor->Data;
        if (TextBox->Active)
            return Cursor;
        Cursor = Cursor->NextSameType;
    }
}

internal void
MenuTextBoxArrowKeysMoveCursor(menu_component *MenuComponent,  menu_direction Dir)
{
    menu_textbox* TextBox = (menu_textbox*)MenuComponent->Data;
    if (TextBox->Active) {
        if (Dir == menu_direction::Right)
            IncrementCursorPosition(TextBox);
        else if (Dir == menu_direction::Left)
            DecrementCursorPosition(TextBox);
    }
}

internal void
MenuTextBoxAddChar(menu_component *MenuComponent, const char *Char)
{
    menu_textbox* TextBox = (menu_textbox*)MenuComponent->Data;
    if (TextBox->Active && TextBox->FontString.Length + 1 < TextBox->MaxTextLength) {
        FontStringSetText(&TextBox->FontString, Insert(TextBox->FontString.Text, TextBox->CursorPosition, Char));
        IncrementCursorPosition(TextBox);
    }
}

internal void
MenuTextBoxRemoveChar(menu_component *MenuComponent, const char *Char)
{
    menu_textbox* TextBox = (menu_textbox*)MenuComponent->Data;
    if (TextBox->Active && TextBox->FontString.Length + 1 < TextBox->MaxTextLength) {
        DecrementCursorPosition(TextBox);
        FontStringSetText(&TextBox->FontString, RemoveAt(TextBox->FontString.Text, TextBox->CursorPosition));
    }
}

internal void
MenuResizeTextBox(menu *Menu, menu_component *MenuComponent, v2 ResizeFactors)
{
    menu_textbox *TextBox = (menu_textbox*)MenuComponent->Data;
    FontStringResize(&TextBox->FontString, ResizeEquivalentAmount(MenuComponent->DefaultTextPixelHeight, ResizeFactors.y));
    MenuComponent->Dim = ResizeEquivalentAmount(MenuComponent->DefaultDim, ResizeFactors);
    MenuComponent->PaddingDim = MenuComponent->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddTextBox(menu *Menu, int ID, v2 GridCoords, v2 Dim, menu_textbox *TextBox, menu_component *Align)
{
    menu_component *MenuComponent = MenuGetNextComponent(Menu);
    MenuComponent->GridCoords = GridCoords;
    MenuComponent->Dim = Dim;
    MenuComponent->DefaultDim = MenuComponent->Dim;
    MenuComponent->DefaultTextPixelHeight = TextBox->FontString.PixelHeight;
    MenuComponent->Type = menu_component_type::TextBox;
    MenuComponent->ID = ID;
    MenuComponent->AlignWith = Align;
    
    MenuComponent->Data = qalloc((void*)TextBox, sizeof(menu_textbox));
    MenuResizeTextBox(Menu, MenuComponent, 0);
    
    Menu->TextBoxes = MenuAddToComponentList(Menu->TextBoxes, MenuComponent);
    return MenuComponent;
}


// menu_logo
internal void
MenuResizeLogo(menu *Menu, menu_component *MenuComponent, v2 ResizeFactors)
{
    menu_logo *Logo = (menu_logo*)MenuComponent->Data;
    MenuComponent->Dim = ResizeEquivalentAmount(MenuComponent->DefaultDim, ResizeFactors.y);
    ResizeTexture(Logo->Tex, MenuComponent->Dim);
    MenuComponent->PaddingDim = MenuComponent->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddLogo(menu *Menu, int ID, v2 GridCoords, v2 Dim, menu_logo *Logo)
{
    menu_component *MenuComponent = MenuGetNextComponent(Menu);
    
    MenuComponent->GridCoords = GridCoords;
    MenuComponent->Dim = Dim;
    MenuComponent->DefaultDim = Dim;
    MenuComponent->Type = menu_component_type::Logo;
    MenuComponent->ID = ID;
    
    MenuComponent->Data = qalloc((void*)Logo, sizeof(menu_logo));
    MenuResizeLogo(Menu, MenuComponent, 0);
    
    Menu->Logos = MenuAddToComponentList(Menu->Logos, MenuComponent);
    return MenuComponent;
}
inline menu_component* MenuAddLogo(menu *Menu, v2 GridCoords, v2 Dim, menu_logo *Logo)
{
    return MenuAddLogo(Menu, -1, GridCoords, Dim, Logo);
}

// menu
internal void
MenuSetUpActivePath(menu *Menu)
{
    Menu->Active = &Menu->Components[0];
    for (int i = 1; i < Menu->NumOfComponents; i++) {
        v2 CGridCoords = Menu->Components[0].GridCoords;
        for (int i = 1; i < Menu->NumOfComponents; i++) {
            v2 Temp = Menu->Components[0].GridCoords;
            
        }
    }
}

internal void
MenuInit(menu *Menu, v2 DefaultDim, int Padding)
{
    Menu->DefaultDim = DefaultDim;
    Menu->DefaultPadding = Padding;
    Menu->NumOfComponents = 0;
    Menu->MaxNumOfComponents = ArrayCount(Menu->Components);
    Menu->Initialized = true;
    
    Menu->Active = &Menu->Components[1];
}

internal menu_events*
HandleMenuEvents(menu *Menu, platform_input *Input)
{
    for (int i = 0; i < ArrayCount(Input->Controllers); i++) {
        platform_controller_input *Input2 = &Input->Controllers[i];
        for (int j = 0; j < ArrayCount(Input2->Buttons); j++) {
            if (Input2->Buttons[j].EndedDown != 0) {
                if (i == 0)
                    Menu->InputMode = menu_input_mode::Keyboard;
                else
                    Menu->InputMode = menu_input_mode::Controller;
            }
        }
    }
    platform_keyboard_input *Input2 = &Input->Keyboard;
    for (int i = 0; i < ArrayCount(Input2->Buttons); i++) {
        if (Input2->Buttons[i].EndedDown != 0)
            Menu->InputMode = menu_input_mode::Keyboard;
    }
    
    Menu->Events.ButtonClicked = -1;
    v2 MouseCoords = v2(Input->MouseX, Input->MouseY);
    
    if (Input->MouseButtons[0].HalfTransitionCount) {
        Menu->Events.ButtonClicked = MenuButtonClicked(Menu, MouseCoords);
    }
    
    platform_controller_input *Controller = &Input->Controllers[0];
    if (Controller->MoveUp.NewEndedDown) {
        Menu->Active = Menu->Active->NextSameType;
    }
    if (Controller->MoveDown.NewEndedDown) {
        Menu->Active = Menu->Active->NextSameType;
    }
    
    if (MenuButtonHovered(Menu, v2(Input->MouseX, Input->MouseY)))
        PlatformSetCursorMode(Input, platform_cursor_mode::Hand);
    else
        PlatformSetCursorMode(Input, platform_cursor_mode::Arrow);
    
    if (Menu->InputMode == menu_input_mode::Keyboard || Menu->InputMode == menu_input_mode::Controller) {
        menu_button *Button = GetComponentData(Menu->Active, menu_button);
        Button->CurrentColor = Button->HoverColor;
    }
    
    return &Menu->Events;
}

internal void
UpdateMenu(menu *Menu, v2 BufferDim)
{
    if (Menu->ScreenDim != BufferDim) {
        Menu->ScreenDim = BufferDim;
        
        v2 ResizeFactors = GetResizeFactor(Menu->DefaultDim, BufferDim);
        Menu->Padding = (int)ResizeEquivalentAmount((real32)Menu->DefaultPadding, ResizeFactors.y);
        
        for (int i = 0; i < Menu->NumOfComponents; i++) {
            menu_component *MenuComponent = &Menu->Components[i];
            if (MenuComponent->Type == menu_component_type::Button)
                MenuResizeButton(Menu, MenuComponent, ResizeFactors);
            if (MenuComponent->Type == menu_component_type::Text)
                MenuResizeText(Menu, MenuComponent, ResizeFactors);
            if (MenuComponent->Type == menu_component_type::TextBox)
                MenuResizeTextBox(Menu, MenuComponent, ResizeFactors);
            if (MenuComponent->Type == menu_component_type::Logo)
                MenuResizeLogo(Menu, MenuComponent, ResizeFactors);
        }
        
        // Setting up rows
        memset(Menu->Rows, 0, sizeof(menu_grid_row) * 10);
        for (int i = 0; i < Menu->NumOfComponents; i++) {
            menu_component *C = &Menu->Components[i];
            menu_grid_row* R = &Menu->Rows[(int)C->GridCoords.y];
            
            real32 Width = 0;
            if (C->AlignWith == 0) 
                Width = C->PaddingDim.x;
            else if (C->AlignWith != 0)
                Width = C->AlignWith->PaddingDim.x;
            
            R->MenuGridColumnWidth[(int)C->GridCoords.x] = Width;
            R->Dim.x += Width;
            if (R->Dim.y < C->PaddingDim.y)
                R->Dim.y = C->PaddingDim.y;
        }
        
        // Height of GUI
        Menu->Dim.y = 0;
        for (int i = 0; i < 10; i++)
            Menu->Dim.y += Menu->Rows[i].Dim.y;
        
        Menu->Dim.x = 0;
        for (int i = 0; i <  Menu->NumOfComponents; i++) {
            menu_component *C = &Menu->Components[i];
            menu_grid_row *R = &Menu->Rows[(int)C->GridCoords.y];
            
            // Calculate column location to center
            C->Coords.x = (-R->Dim.x)/2;
            for (int  i = 0; i < C->GridCoords.x; i++)
                C->Coords.x += Menu->Rows[(int)C->GridCoords.y].MenuGridColumnWidth[i];
            
            // Find biggest row
            if (Menu->Dim.x < R->Dim.x) {
                Menu->Dim.x = R->Dim.x;
                Menu->Coords.x = C->Coords.x - (Menu->Padding);
            }
            // Calculate row location to center
            C->Coords.y = ((-Menu->Dim.y)/2) + ((R->Dim.y - C->PaddingDim.y)/2);
            for (int i = 0; i < C->GridCoords.y; i++) {
                menu_grid_row* tempR = &Menu->Rows[i];
                C->Coords.y += tempR->Dim.y;
            }
        }
        
        menu_component* C = &Menu->Components[0];
        Menu->Coords.y = C->Coords.y - (Menu->Padding);
    }
}

internal void
DrawMenu(menu *Menu, real32 Z)
{
    for (int i = 0; i < Menu->NumOfComponents; i++) {
        menu_component *MenuComponent = &Menu->Components[i];
        
        v2 Padding = (MenuComponent->Dim - MenuComponent->PaddingDim)/2;
        
        if (MenuComponent->Type == menu_component_type::Button) {
            menu_button *Button = (menu_button*)MenuComponent->Data;
            
            v2 SDim = FontStringGetDim(&Button->FontString);
            v2 TextCoords = MenuComponent->Coords + ((MenuComponent->Dim - SDim)/2);
            Push(RenderGroup, v3(MenuComponent->Coords - Padding, Z), MenuComponent->Dim, Button->CurrentColor, 0.0f);
            FontStringPrint(&Button->FontString, TextCoords - Padding);
        }
        else if (MenuComponent->Type == menu_component_type::TextBox) {
            menu_textbox *TextBox = (menu_textbox*)MenuComponent->Data;
            
            v2 SDim = FontStringGetDim(&TextBox->FontString);
            
            TextBox->TextCoords = MenuComponent->Coords;
            TextBox->TextCoords.y += (MenuComponent->Dim.y - SDim.y)/2;
            
            real32 OutOfBounds = (SDim.x + (MenuComponent->Dim.y/4)) - (MenuComponent->Dim.x);
            if (OutOfBounds >= 0) {
                TextBox->TextCoords.x -= SDim.x - (MenuComponent->Dim.x);
                TextBox->TextCoords.x -= (MenuComponent->Dim.y)/4;
            }
            else {
                TextBox->TextCoords.x += (MenuComponent->Dim.y)/4;
            }
            
            Push(RenderGroup, v3(MenuComponent->Coords - Padding, 50.0f), MenuComponent->Dim, TextBox->CurrentColor, 0.0f);
            
            if (TextBox->Active) {
                real32 CursorX = TextBox->TextCoords.x;
                for (int i = 0; i <  TextBox->CursorPosition; i++) {
                    CursorX += TextBox->FontString.Advances[i];
                }
                
                if (CursorX < MenuComponent->Coords.x) {
                    real32 TempCursorX = CursorX;
                    int i = 0;
                    while (TempCursorX < MenuComponent->Coords.x) {
                        TempCursorX += TextBox->FontString.Advances[i];
                    }
                    real32 Diff = TempCursorX - CursorX;
                    CursorX = TempCursorX;
                    TextBox->TextCoords.x += Diff;
                }
                Push(RenderGroup, v3(CursorX - Padding.x, MenuComponent->Coords.y - Padding.y, 100.0f), v2(5.0f, MenuComponent->Dim.y), 0xFF000000, 0.0f);
            }
            FontStringPrint(&TextBox->FontString, TextBox->TextCoords - Padding, MenuComponent->Coords - Padding, MenuComponent->Dim);
        }
        else if (MenuComponent->Type == menu_component_type::Text) {
            menu_text *Text = (menu_text*)MenuComponent->Data;
            v2 TextCoords = MenuComponent->Coords;
            if (MenuComponent->AlignWith != 0)
                TextCoords.x = MenuComponent->Coords.x + MenuComponent->AlignWith->Dim.x - MenuComponent->Dim.x;
            FontStringPrint(&Text->FontString, TextCoords - Padding);
        }
        else if (MenuComponent->Type == menu_component_type::Logo) {
            menu_logo *Logo = (menu_logo*)MenuComponent->Data;
            Push(RenderGroup, v3(MenuComponent->Coords - Padding, 100.0f), MenuComponent->Dim, 
                 Logo->Tex, 0.0f, BlendMode::gl_src_alpha);
        }
    }
    
    Push(RenderGroup, v3(Menu->Coords, 50.0f), Menu->Dim + (Menu->Padding * 2), Menu->BackgroundColor, 0.0f);
}
inline void DrawMenu(menu *Menu) { DrawMenu(Menu, 0); }

#endif //MENU_H