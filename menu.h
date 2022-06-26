#ifndef MENU_H
#define MENU_H

enum struct menu_direction
{
    Right,
    Up,
    Left,
    Down,
};

struct menu_component_button
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

struct menu_component_logo
{
    Texture *Tex;
    Texture *Alt;
};

struct menu_component_text
{
    font_string FontString;
    uint32 DefaultTextColor;
};

struct menu_component_textbox
{
    font_string FontString;
    v2 TextCoords;
    
    bool Active;
    int MaxTextLength = 100;
    real32 XAdvances[100];
    
    int CursorPosition;
    real32 DisplayLeft;
    real32 DisplayRight;
    bool PaddingRight;
    
    uint32 CurrentColor;
    uint32 CurrentTextColor;
};

struct menu_component_checkbox
{
    bool32 Clicked;
    platform_controller_input *Controller;
    
    uint32 CurrentColor;
    uint32 DefaultColor;
    uint32 HoverColor;
    uint32 DefaultTextColor;
    uint32 HoverTextColor;
    
    Texture *CurrentTexture;
    Texture *DefaultTexture;
    Texture *ClickedTexture;
    Texture *ActiveTexture;
    Texture *ActiveClickedTexture;
};

//enum menu_component_id;
enum struct menu_component_type
{
    Button,
    Text,
    TextBox,
    Logo,
    CheckBox,
};
struct menu_component
{
    int ID;
    //char* ID;
    
    v2 Coords;
    v2 GridCoords;
    v2 Dim;
    v2 DefaultDim;
    real32 DefaultTextPixelHeight;
    v2 PaddingDim;
    
    bool Active;
    
    menu_component *AlignWith;
    
    menu_component *NextSameType;
    menu_component_type Type;
    void *Data;
};

#define GetComponentData(d, t) ((t*)d->Data)

struct menu_events
{
    int ButtonClicked;
    int CheckBoxClicked;
    int TextBoxClicked;
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
    bool Reset;
    
    bool Edit;
    
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
    
    arr MenuButtons; // menu_component_button
    
    menu_component *Buttons;
    menu_component *TextBoxes;
    menu_component *Texts;
    menu_component *Logos;
    menu_component *CheckBoxes;
    
    menu_events Events;
    
    int ActiveIndex;
    int NumOfActiveComponents;
    menu_component *ActiveComponents[20];
    
    uint32 BackgroundColor;
};
inline void IncrActive(menu *Menu)
{
    Menu->ActiveComponents[Menu->ActiveIndex]->Active = false;
    Menu->ActiveIndex++;
    if (Menu->ActiveIndex == Menu->NumOfActiveComponents)
        Menu->ActiveIndex = 0;
}
inline void DecrActive(menu *Menu)
{
    Menu->ActiveComponents[Menu->ActiveIndex]->Active = false;
    Menu->ActiveIndex--;
    if (Menu->ActiveIndex < 0)
        Menu->ActiveIndex = Menu->NumOfActiveComponents - 1;
}
inline void UpdateActiveIndex(menu *Menu)
{
    for (int i = 0; i < Menu->NumOfActiveComponents; i++) {
        menu_component *C = Menu->ActiveComponents[i];
        if (C->Active)
            Menu->ActiveIndex = i;
    }
}


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

internal menu_component*
MenuGetComponent(menu *Menu, pair_int_string *IDs, int NumOfIDs, const char *ID)
{
    int intID = -1;
    for (int i = 0; i < NumOfIDs; i++) {
        if (Equal(IDs[i].String, ID))
        {
            intID = IDs[i].Int;
        }
    }
    
    for (int i = 0; i < Menu->NumOfComponents; i++) {
        if (Menu->Components[i].ID == intID)
            return &Menu->Components[i];
    }
    
    return 0;
}

// menu_component_button
internal int
MenuButtonClicked(menu *Menu, v2 MouseCoords)
{
    menu_component* Cursor = Menu->Buttons;
    while(Cursor != 0) {
        if (CoordsInRect(MouseCoords, Cursor->Coords, Cursor->PaddingDim)) {
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
        menu_component_button *Button = (menu_component_button*)Cursor->Data;
        if (CoordsInRect(MouseCoords, Cursor->Coords, Cursor->PaddingDim)) {
            //Button->CurrentColor = Button->HoverColor;
            //Button->FontString.Color = Button->HoverTextColor;
            Cursor->Active = true;
            ButtonHovered = true;
        }
        else {
            Cursor->Active = false;
            //Button->CurrentColor = Button->DefaultColor;
            //Button->FontString.Color = Button->DefaultTextColor;
        }
        Cursor = Cursor->NextSameType;
    }
    
    return ButtonHovered;
}

internal void
MenuResizeButton(menu *Menu, menu_component *MComp, v2 ResizeFactors)
{
    menu_component_button *Button = (menu_component_button*)MComp->Data;
    FontStringResize(&Button->FontString, ResizeEquivalentAmount(MComp->DefaultTextPixelHeight, ResizeFactors.y));
    MComp->Dim = ResizeEquivalentAmount(MComp->DefaultDim, ResizeFactors);
    MComp->PaddingDim = MComp->Dim + (Menu->Padding * 2);
}
/*
internal void
MenuUpdateButton(menu *Menu, menu_component *MComp, int ID, v2 GridCoords, v2 Dim, menu_component_button *Updated)
{
    menu_component_button *Button = (menu_component_button*)MComp->Data;
    Button->DefaultTextColor = Updated->DefaultTextColor;
    Button->HoverTextColor = Updated->HoverTextColor;
    Button->FontString = Updated->FontString;
    Button->FontString.Color = Button->DefaultTextColor;
    FontStringInit(&Button->FontString);
    
    MComp->GridCoords = GridCoords;
    MComp->Dim = Dim;
    MComp->DefaultDim = MComp->Dim;
    MComp->DefaultTextPixelHeight = Button->FontString.PixelHeight;
    MComp->ID = ID;
    
    Button->DefaultColor = Updated->DefaultColor;
    Button->HoverColor = Updated->HoverColor;
    
    Button->CurrentColor = Updated->DefaultColor;
    
    MenuResizeButton(Menu, MComp, 0);
}
*/
internal menu_component*
MenuAddButton(menu *Menu, int ID, v2 GridCoords, v2 Dim, menu_component_button *Button)
{
    Button->FontString.Color = Button->DefaultTextColor;
    FontStringInit(&Button->FontString);
    
    menu_component *MComp = MenuGetNextComponent(Menu);
    MComp->GridCoords = GridCoords;
    MComp->Dim = Dim;
    MComp->DefaultDim = MComp->Dim;
    MComp->DefaultTextPixelHeight = Button->FontString.PixelHeight;
    MComp->Type = menu_component_type::Button;
    MComp->ID = ID;
    
    Button->CurrentColor = Button->DefaultColor;
    
    //MComp->Data = qalloc((void*)Button, sizeof(menu_component_button));
    //MComp->Data = ArrPush(Menu->MenuButtons, Button, menu_component_button);
    
    if (MComp->Data == 0)
        MComp->Data = ArrPush(Menu->MenuButtons, Button, menu_component_button);
    else
        MemCpy(MComp->Data, Button, menu_component_button);
    
    MenuResizeButton(Menu, MComp, 0);
    
    Menu->Buttons = MenuAddToComponentList(Menu->Buttons, MComp);
    return MComp;
}

// menu_component_text
internal void
MenuResizeText(menu *Menu, menu_component *MComp, v2 ResizeFactors)
{
    menu_component_text *Text = (menu_component_text*)MComp->Data;
    MComp->Dim = FontStringResize(&Text->FontString, ResizeEquivalentAmount(MComp->DefaultTextPixelHeight, ResizeFactors.y));
    MComp->PaddingDim = MComp->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddText(menu *Menu, int ID, v2 GridCoords, menu_component_text *Text, menu_component *Align)
{
    Text->FontString.Color = Text->DefaultTextColor;
    FontStringInit(&Text->FontString);
    
    menu_component *MComp = MenuGetNextComponent(Menu);
    MComp->GridCoords = GridCoords;
    MComp->Dim = FontStringGetDim(&Text->FontString);
    MComp->DefaultDim = MComp->Dim;
    MComp->DefaultTextPixelHeight = Text->FontString.PixelHeight;
    MComp->AlignWith = Align;
    MComp->Type = menu_component_type::Text;
    MComp->ID = ID;
    
    MComp->Data = qalloc((void*)Text, sizeof(menu_component_text));
    MenuResizeText(Menu, MComp, 0);
    
    Menu->Texts = MenuAddToComponentList(Menu->Texts, MComp);
    return MComp;
}
inline menu_component* MenuAddText(menu *Menu, int ID, v2 GridCoords, menu_component_text *Text)
{
    return MenuAddText(Menu, ID, GridCoords, Text, 0);
}
inline menu_component* MenuAddText(menu *Menu, v2 GridCoords, menu_component_text *Text, menu_component *Align)
{
    return MenuAddText(Menu, -1, GridCoords, Text, Align);
}
inline menu_component* MenuAddText(menu *Menu, v2 GridCoords, menu_component_text *Text)
{
    return MenuAddText(Menu, -1, GridCoords, Text, 0);
}

// menu_component_textbox
inline void IncrementCursorPosition(menu_component_textbox *TextBox)
{
    if (TextBox->CursorPosition < TextBox->FontString.Length) {
        TextBox->CursorPosition++;
    }
}
inline void DecrementCursorPosition(menu_component_textbox *TextBox)
{
    if (TextBox->CursorPosition != 0) {
        TextBox->CursorPosition--;
    }
}

internal char*
MenuGetTextBoxText(menu *Menu, int ID)
{
    menu_component* Cursor = Menu->TextBoxes;
    while(Cursor != 0) {
        menu_component_textbox *TextBox = (menu_component_textbox*)Cursor->Data;
        if (Cursor->ID == ID ) {
            return TextBox->FontString.Text;
        }
        
        Cursor = Cursor->NextSameType;
    }
    
    return 0;
}

internal int
MenuTextBoxClicked(menu *Menu, v2 MouseCoords)
{
    int ID = -1;
    
    menu_component* Cursor = Menu->TextBoxes;
    while(Cursor != 0) {
        menu_component_textbox *TextBox = (menu_component_textbox*)Cursor->Data;
        if (CoordsInRect(MouseCoords, Cursor->Coords, Cursor->PaddingDim)) {
            Cursor->Active = true;
            ID = Cursor->ID;
        }
        else
            Cursor->Active = false;
        Cursor = Cursor->NextSameType;
    }
    
    return ID;
}

internal menu_component*
MenuTextBoxGetActive(menu_component* Cursor)
{
    while(Cursor != 0) {
        menu_component_textbox *TextBox = (menu_component_textbox*)Cursor->Data;
        if (TextBox->Active)
            return Cursor;
        Cursor = Cursor->NextSameType;
    }
    return 0;
}

internal void
MenuTextBoxArrowKeysMoveCursor(menu_component *MComp,  menu_direction Dir)
{
    menu_component_textbox* TextBox = (menu_component_textbox*)MComp->Data;
    if (Dir == menu_direction::Right) 
        IncrementCursorPosition(TextBox);
    else if (Dir == menu_direction::Left) 
        DecrementCursorPosition(TextBox);
}

internal void
MenuTextBoxMouseMoveCursor(menu_component *MComp, v2 MouseCoords)
{
    menu_component_textbox* TextBox = (menu_component_textbox*)MComp->Data;
    
    v2 Coords = MComp->Coords;
    
    int StringLength = GetLength(TextBox->FontString.Text);
    
    v2 CharCoords[100];
    CharCoords[0] = v2(TextBox->TextCoords.x, TextBox->TextCoords.y);
    
    real32 ClosestX = TextBox->TextCoords.x;
    real32 CharX = TextBox->TextCoords.x;
    int Closest = 0;
    for (int i = 0; i < StringLength; i++) {
        CharX += TextBox->FontString.Advances[i];
        real32 DiffClosest = fabsf(ClosestX - MouseCoords.x);
        real32 DiffCurrent = fabsf(CharX - MouseCoords.x);
        if (DiffCurrent < DiffClosest) {
            ClosestX = CharX;
            Closest = i + 1;
        }
    }
    
    TextBox->CursorPosition = Closest;
}

internal void
MenuTextBoxAddChar(menu_component *MComp, const char *Char)
{
    menu_component_textbox* TextBox = (menu_component_textbox*)MComp->Data;
    if (TextBox->FontString.Length + 1 < TextBox->MaxTextLength) {
        FontStringSetText(&TextBox->FontString, Insert(TextBox->FontString.Text, TextBox->CursorPosition, Char));
        IncrementCursorPosition(TextBox);
    }
}

internal void
MenuTextBoxRemoveChar(menu_component *MComp)
{
    menu_component_textbox* TextBox = (menu_component_textbox*)MComp->Data;
    if (TextBox->FontString.Length + 1 < TextBox->MaxTextLength) {
        DecrementCursorPosition(TextBox);
        FontStringSetText(&TextBox->FontString, RemoveAt(TextBox->FontString.Text, TextBox->CursorPosition));
    }
}

internal void
MenuTextBoxReplaceText(menu_component *MComp, char *NewText)
{
    menu_component_textbox* TextBox = (menu_component_textbox*)MComp->Data;
    if (TextBox->FontString.Length + GetLength(NewText) < TextBox->MaxTextLength) {
        FontStringSetText(&TextBox->FontString, Insert(TextBox->FontString.Text, TextBox->CursorPosition, NewText));
        TextBox->CursorPosition += GetLength(NewText);
    }
}

internal void
MenuResizeTextBox(menu *Menu, menu_component *MComp, v2 ResizeFactors)
{
    menu_component_textbox *TextBox = (menu_component_textbox*)MComp->Data;
    FontStringResize(&TextBox->FontString, ResizeEquivalentAmount(MComp->DefaultTextPixelHeight, ResizeFactors.y));
    MComp->Dim = ResizeEquivalentAmount(MComp->DefaultDim, ResizeFactors);
    MComp->PaddingDim = MComp->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddTextBox(menu *Menu, int ID, v2 GridCoords, v2 Dim, menu_component_textbox *TextBox)
{
    TextBox->FontString.Color = TextBox->CurrentTextColor;
    FontStringInit(&TextBox->FontString);
    
    menu_component *MComp = MenuGetNextComponent(Menu);
    MComp->GridCoords = GridCoords;
    MComp->Dim = Dim;
    MComp->DefaultDim = MComp->Dim;
    MComp->DefaultTextPixelHeight = TextBox->FontString.PixelHeight;
    MComp->Type = menu_component_type::TextBox;
    MComp->ID = ID;
    //MComp->AlignWith = Align;
    
    MComp->Data = qalloc((void*)TextBox, sizeof(menu_component_textbox));
    MenuResizeTextBox(Menu, MComp, 0);
    
    Menu->TextBoxes = MenuAddToComponentList(Menu->TextBoxes, MComp);
    return MComp;
}


// menu_component_logo
internal void
MenuResizeLogo(menu *Menu, menu_component *MComp, v2 ResizeFactors)
{
    menu_component_logo *Logo = (menu_component_logo*)MComp->Data;
    MComp->Dim = ResizeEquivalentAmount(MComp->DefaultDim, ResizeFactors.y);
    ResizeTexture(Logo->Tex, MComp->Dim);
    MComp->PaddingDim = MComp->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddLogo(menu *Menu, int ID, v2 GridCoords, v2 Dim, menu_component_logo *Logo)
{
    menu_component *MComp = MenuGetNextComponent(Menu);
    
    MComp->GridCoords = GridCoords;
    MComp->Dim = Dim;
    MComp->DefaultDim = Dim;
    MComp->Type = menu_component_type::Logo;
    MComp->ID = ID;
    
    MComp->PaddingDim = MComp->Dim + (Menu->Padding * 2);
    MComp->Data = qalloc((void*)Logo, sizeof(menu_component_logo));
    //MenuResizeLogo(Menu, MComp, 0);
    
    
    Menu->Logos = MenuAddToComponentList(Menu->Logos, MComp);
    return MComp;
}
inline menu_component* MenuAddLogo(menu *Menu, v2 GridCoords, v2 Dim, menu_component_logo *Logo)
{
    return MenuAddLogo(Menu, -1, GridCoords, Dim, Logo);
}

// menu_component_checkbox
internal menu_component*
MenuGetComponent(menu_component *Cursor, int ID)
{
    while(Cursor != 0) {
        if (Cursor->ID == ID)
            return Cursor;
        Cursor = Cursor->NextSameType;
    }
    return 0;
}

internal void
MenuResizeCheckBox(menu *Menu, menu_component *MComp, v2 ResizeFactors)
{
    menu_component_button *Button = (menu_component_button*)MComp->Data;
    MComp->Dim = ResizeEquivalentAmount(MComp->DefaultDim, ResizeFactors);
    MComp->PaddingDim = MComp->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddCheckBox(menu *Menu, int ID, v2 GridCoords, v2 Dim, menu_component_checkbox *CheckBox)
{
    menu_component *MComp = MenuGetNextComponent(Menu);
    MComp->GridCoords = GridCoords;
    MComp->Dim = Dim;
    MComp->DefaultDim = MComp->Dim;
    MComp->Type = menu_component_type::CheckBox;
    MComp->ID = ID;
    
    CheckBox->CurrentColor = CheckBox->DefaultColor;
    CheckBox->CurrentTexture = CheckBox->DefaultTexture;
    
    MComp->Data = qalloc((void*)CheckBox, sizeof(menu_component_checkbox));
    MenuResizeCheckBox(Menu, MComp, 0);
    
    Menu->CheckBoxes = MenuAddToComponentList(Menu->CheckBoxes, MComp);
    return MComp;
}

// menu
internal void
MenuSortActiveComponents(menu *Menu)
{
    Menu->NumOfActiveComponents = 0;
    for (int i = 0; i < Menu->NumOfComponents; i++) {
        menu_component *C = &Menu->Components[i];
        if (C->Type == menu_component_type::Button ||
            C->Type == menu_component_type::TextBox ||
            C->Type == menu_component_type::CheckBox) {
            Menu->ActiveComponents[Menu->NumOfActiveComponents++] = C;
        }
    }
    
    // X-Sort using Insertion Sort
    {
        int i = 1;
        while (i < Menu->NumOfActiveComponents) {
            int j = i;
            while (j > 0 && Menu->ActiveComponents[j-1]->GridCoords.x > Menu->ActiveComponents[j]->GridCoords.x) {
                menu_component *Temp = Menu->ActiveComponents[j];
                Menu->ActiveComponents[j] = Menu->ActiveComponents[j-1];
                Menu->ActiveComponents[j-1] = Temp;
                j = j - 1;
            }
            i++;
        }
    }
    
    // Y-Sort using Insertion Sort
    {
        int i = 1;
        while (i < Menu->NumOfActiveComponents) {
            int j = i;
            while (j > 0 && Menu->ActiveComponents[j-1]->GridCoords.y > Menu->ActiveComponents[j]->GridCoords.y) {
                menu_component *Temp = Menu->ActiveComponents[j];
                Menu->ActiveComponents[j] = Menu->ActiveComponents[j-1];
                Menu->ActiveComponents[j-1] = Temp;
                j = j - 1;
            }
            i++;
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
    memset(Menu->Components, 0, Menu->MaxNumOfComponents * sizeof(menu_component));
    
    ArrInit(&Menu->MenuButtons, 10, sizeof(menu_component_button));
    
    Menu->Buttons = 0;
    Menu->TextBoxes = 0;
    Menu->Texts = 0;
    Menu->Logos = 0;
    Menu->CheckBoxes = 0;
    
    Menu->Initialized = true;
}

internal void
MenuInit(menu *Menu)
{
    Menu->NumOfComponents = 0;
    Menu->MaxNumOfComponents = ArrayCount(Menu->Components);
    memset(Menu->Components, 0, Menu->MaxNumOfComponents * sizeof(menu_component));
    //memset(&Menu->MenuButtons, 0, Menu->MenuButtons.MaxSize * Menu->MenuButtons.TypeSize);
    
    ArrInit(&Menu->MenuButtons, 10, sizeof(menu_component_button));
    
    Menu->Buttons = 0;
    Menu->TextBoxes = 0;
    Menu->Texts = 0;
    Menu->Logos = 0;
    Menu->CheckBoxes = 0;
    
    Menu->Initialized = true;
}

internal void
MenuReset(menu *Menu) 
{
    for (int i = 0; i < Menu->NumOfComponents; i++) {
        Menu->Components[i].Active = false;
        
        if (Menu->Components[i].Type == menu_component_type::CheckBox) {
            menu_component_checkbox *CheckBox = (menu_component_checkbox*)Menu->Components[i].Data;
            CheckBox->Clicked = false;
        }
    }
    
    Menu->ActiveIndex = 0;
    Menu->Reset = false;
}

internal menu_events*
HandleMenuEvents(menu *Menu, platform_input *Input)
{
    UpdateInputInfo(Input);
    
    bool KeyboardMode = Input->CurrentInputInfo.InputMode == platform_input_mode::Keyboard;
    bool KeyboardPreviousMode = Input->PreviousInputInfo.InputMode == platform_input_mode::Keyboard;
    
    bool ControllerMode = Input->CurrentInputInfo.InputMode == platform_input_mode::Controller;
    bool ControllerPreviousMode = Input->PreviousInputInfo.InputMode == platform_input_mode::Controller;
    
    bool MouseMode = Input->CurrentInputInfo.InputMode == platform_input_mode::Mouse;
    bool MousePreviousMode = Input->PreviousInputInfo.InputMode == platform_input_mode::Mouse;
    
    Menu->Events.ButtonClicked = -1;
    Menu->Events.TextBoxClicked = -1;
    
    v2 MouseCoords = v2(Input->Mouse.X, Input->Mouse.Y);
    
    if (KeyboardMode || ControllerMode) {
        if (KeyboardPreviousMode || ControllerPreviousMode) {
            platform_controller_input *Controller = 0;
            if (KeyboardMode)
                Controller = Input->CurrentInputInfo.Controller;
            else if (ControllerMode)
                Controller = Input->CurrentInputInfo.Controller;
            
            if (OnKeyDown(&Controller->MoveUp)) {
                DecrActive(Menu);
            }
            if (OnKeyDown(&Controller->MoveDown)) {
                IncrActive(Menu);
            }
            if(OnKeyDown(&Input->Keyboard.Tab)) {
                IncrActive(Menu);
            }
            
            if (OnKeyDown(&Controller->Enter)) {
                menu_component *C = Menu->ActiveComponents[Menu->ActiveIndex];
                if (C->Type == menu_component_type::Button)
                    Menu->Events.ButtonClicked = C->ID;
                else if (C->Type == menu_component_type::CheckBox) {
                    menu_component_checkbox *CheckBox = (menu_component_checkbox*)C->Data;
                    
                    if (CheckBox->Clicked && Controller == CheckBox->Controller) {
                        CheckBox->Clicked = false;
                        Controller->IgnoreInputs = false;
                    }
                    else if (!CheckBox->Clicked && !Controller->IgnoreInputs) {
                        CheckBox->Controller = Controller;
                        Controller->IgnoreInputs = true;
                        CheckBox->Clicked = true;
                    }
                    Menu->Events.CheckBoxClicked = C->ID;
                }
            }
        }
        
        Menu->ActiveComponents[Menu->ActiveIndex]->Active = true;
    }
    else if (MouseMode) {
        if (Menu->ActiveComponents[Menu->ActiveIndex]->Type == menu_component_type::Button)
            Menu->ActiveComponents[Menu->ActiveIndex]->Active = false;
        
        if (OnKeyDown(&Input->Mouse.Left)) {
            Menu->Events.ButtonClicked = MenuButtonClicked(Menu, MouseCoords);
            Menu->Events.TextBoxClicked = MenuTextBoxClicked(Menu, MouseCoords);
            UpdateActiveIndex(Menu);
        }
        
        if (MenuButtonHovered(Menu, v2(Input->Mouse.X, Input->Mouse.Y)))
            PlatformSetCursorMode(&Input->Mouse, platform_cursor_mode::Hand);
        else
            PlatformSetCursorMode(&Input->Mouse, platform_cursor_mode::Arrow);
    }
    
    //menu_component *ActiveTextBox = MenuTextBoxGetActive(Menu->TextBoxes);
    menu_component *ActiveTextBox = Menu->ActiveComponents[Menu->ActiveIndex];
    if (ActiveTextBox->Type == menu_component_type::TextBox)
    {
        for (int i = 0; i < 10; i++) {
            if (OnKeyDown(&Input->Keyboard.Numbers[i]))
                MenuTextBoxAddChar(ActiveTextBox, IntToString(i));
        }
        
        if (KeyDown(&Input->Mouse.Left)) {
            MenuTextBoxMouseMoveCursor(ActiveTextBox, MouseCoords);
        }
        
        if (KeyPressed(&Input->Keyboard.Left, Input))
            MenuTextBoxArrowKeysMoveCursor(ActiveTextBox, menu_direction::Left);
        if (KeyPressed(&Input->Keyboard.Right, Input))
            MenuTextBoxArrowKeysMoveCursor(ActiveTextBox, menu_direction::Right);
        if (KeyDown(&Input->Keyboard.CtrlV))
            MenuTextBoxReplaceText(ActiveTextBox, Input->Keyboard.Clipboard);
        
        if (KeyPressed(&Input->Keyboard.Backspace, Input))
            MenuTextBoxRemoveChar(ActiveTextBox);
        if (OnKeyDown(&Input->Keyboard.Period))
            MenuTextBoxAddChar(ActiveTextBox, ".");
    }
    
    return &Menu->Events;
}

internal void
ResizeMenu(menu *Menu, v2 BufferDim)
{
    Menu->ScreenDim = BufferDim;
    
    v2 ResizeFactors = GetResizeFactor(Menu->DefaultDim, BufferDim);
    Menu->Padding = (int)ResizeEquivalentAmount((real32)Menu->DefaultPadding, ResizeFactors.y);
    
    for (int i = 0; i < Menu->NumOfComponents; i++) {
        menu_component *MComp = &Menu->Components[i];
        if (MComp->Type == menu_component_type::Button)
            MenuResizeButton(Menu, MComp, ResizeFactors);
        if (MComp->Type == menu_component_type::Text)
            MenuResizeText(Menu, MComp, ResizeFactors);
        if (MComp->Type == menu_component_type::TextBox)
            MenuResizeTextBox(Menu, MComp, ResizeFactors);
        if (MComp->Type == menu_component_type::Logo)
            MenuResizeLogo(Menu, MComp, ResizeFactors);
        if (MComp->Type == menu_component_type::CheckBox)
            MenuResizeCheckBox(Menu, MComp, ResizeFactors);
    }
}

internal void
UpdateMenu(menu *Menu)
{
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
inline void UpdateMenu(menu *Menu, v2 BufferDim) 
{
    ResizeMenu(Menu, BufferDim);
    UpdateMenu(Menu);
}

internal void
DrawMenu(menu *Menu, real32 Z)
{
    for (int i = 0; i < Menu->NumOfComponents; i++) {
        menu_component *MComp = &Menu->Components[i];
        
        v2 Padding = (MComp->Dim - MComp->PaddingDim)/2;
        
        if (MComp->Type == menu_component_type::Button) {
            menu_component_button *Button = (menu_component_button*)MComp->Data;
            
            if (MComp->Active == true) {
                //fprintf(stderr, "BtnDraw %d %d\n", Menu->ActiveIndex, Menu->NumOfComponents);
                Button->CurrentColor = Button->HoverColor;
                Button->FontString.Color = Button->HoverTextColor;
            }
            else {
                Button->CurrentColor = Button->DefaultColor;
                Button->FontString.Color = Button->DefaultTextColor;
            }
            v2 SDim = FontStringGetDim(&Button->FontString);
            v2 TextCoords = MComp->Coords + ((MComp->Dim - SDim)/2);
            Push(RenderGroup, v3(MComp->Coords - Padding, Z), MComp->Dim, Button->CurrentColor, 0.0f);
            
            FontStringPrint(&Button->FontString, TextCoords - Padding);
        }
        else if (MComp->Type == menu_component_type::TextBox) {
            menu_component_textbox *TextBox = (menu_component_textbox*)MComp->Data;
            Push(RenderGroup, v3(MComp->Coords - Padding, 50.0f), MComp->Dim, TextBox->CurrentColor, 0.0f);
            
            if (MComp->Active) {
                v2 SDim = FontStringGetDim(&TextBox->FontString);
                TextBox->TextCoords = MComp->Coords;
                TextBox->TextCoords.y += (MComp->Dim.y - SDim.y)/2;
                real32 CursorPadding = (MComp->Dim.y)/4;
                real32 CursorX = 0;
                real32 LeftX = MComp->Coords.x;
                real32 RightX = MComp->Coords.x + MComp->Dim.x;
                
                if (TextBox->DisplayRight == 0) {
                    TextBox->DisplayRight = MComp->Dim.x;
                    TextBox->DisplayLeft = 0;
                }
                
                for (int i = 0; i <  TextBox->CursorPosition; i++)
                    CursorX += TextBox->FontString.Advances[i];
                
                if (TextBox->DisplayRight < CursorX + CursorPadding) {
                    TextBox->DisplayLeft = TextBox->DisplayLeft + (CursorX - TextBox->DisplayRight);
                    TextBox->DisplayRight = CursorX;
                    TextBox->PaddingRight = true;
                }
                if (TextBox->DisplayLeft > CursorX - CursorPadding) {
                    TextBox->DisplayRight = TextBox->DisplayRight - (TextBox->DisplayLeft - CursorX);
                    TextBox->DisplayLeft = CursorX;
                    TextBox->PaddingRight = false;
                }
                
                
                if (!TextBox->PaddingRight) {
                    TextBox->TextCoords.x +=  CursorPadding;
                }
                else if (TextBox->PaddingRight) {
                    TextBox->TextCoords.x -=  CursorPadding;
                }
                
                
                TextBox->TextCoords.x -= TextBox->DisplayLeft;
                CursorX += TextBox->TextCoords.x;
                Push(RenderGroup, v3(CursorX - Padding.x, MComp->Coords.y - Padding.y, 100.0f), v2(5.0f, MComp->Dim.y), 0xFF000000, 0.0f);
            }
            
            FontStringPrint(&TextBox->FontString, TextBox->TextCoords - Padding, MComp->Coords - Padding, MComp->Dim);
        }
        else if (MComp->Type == menu_component_type::Text) {
            menu_component_text *Text = (menu_component_text*)MComp->Data;
            v2 TextCoords = MComp->Coords;
            if (MComp->AlignWith != 0)
                TextCoords.x = MComp->Coords.x + MComp->AlignWith->Dim.x - MComp->Dim.x;
            FontStringPrint(&Text->FontString, TextCoords - Padding);
        }
        else if (MComp->Type == menu_component_type::Logo) {
            menu_component_logo *Logo = (menu_component_logo*)MComp->Data;
            Push(RenderGroup, v3(MComp->Coords - Padding, 100.0f), MComp->Dim, 
                 Logo->Tex, 0.0f, BlendMode::gl_src_alpha);
        }
        else if (MComp->Type == menu_component_type::CheckBox) {
            menu_component_checkbox *CheckBox = (menu_component_checkbox*)MComp->Data;
            
            if (MComp->Active == true && !CheckBox->Clicked) {
                CheckBox->CurrentColor = CheckBox->HoverColor;
                CheckBox->CurrentTexture = CheckBox->ActiveTexture;
            }
            else if (MComp->Active == false && CheckBox->Clicked) {
                CheckBox->CurrentColor = CheckBox->HoverColor;
                CheckBox->CurrentTexture = CheckBox->ClickedTexture;
            }
            else if (MComp->Active == true && CheckBox->Clicked) {
                CheckBox->CurrentColor = CheckBox->HoverColor;
                CheckBox->CurrentTexture = CheckBox->ActiveClickedTexture;
            }
            else {
                CheckBox->CurrentColor = CheckBox->DefaultColor;
                CheckBox->CurrentTexture = CheckBox->DefaultTexture;
            }
            
            Push(RenderGroup, v3(MComp->Coords - Padding, 100.0f), MComp->Dim, 
                 CheckBox->CurrentTexture, 0.0f, BlendMode::gl_src_alpha);
        }
    }
    
    Push(RenderGroup, v3(Menu->Coords, 50.0f), Menu->Dim + (Menu->Padding * 2), Menu->BackgroundColor, 0.0f);
}
inline void DrawMenu(menu *Menu) { DrawMenu(Menu, 0); }

struct menu_token
{
    char ID[100];
    char Value[100];
};

struct menu_token_collection
{
    char ID[50];
    char Type[50];
    int NumOfTokens = 0;
    menu_token Tokens[30];
};

struct string_copy_buffer
{
    int Index;
    char Value[100];
};
inline void Clear(string_copy_buffer *Buffer)
{
    Buffer->Index = 0;
    memset(Buffer->Value, 0, sizeof(Buffer->Value));
}
inline void NextChar(string_copy_buffer *Buffer, char Next)
{
    Buffer->Value[Buffer->Index++] = Next;
}

struct v2_string
{
    union
    {
        struct
        {
            string_copy_buffer x;
            string_copy_buffer y;
        };
        char *v[2];
    };
};
internal v2_string
Getv2StringFromChar(char *Input)
{
    v2_string Result = {};
    
    bool FirstNum = true;
    int i = 0;
    while(Input[i] != 0)
    {
        if (FirstNum)
        {
            if (Input[i] != ',')
            {
                NextChar(&Result.x, Input[i]);
            }
            else
            {
                FirstNum = false;
            }
        }
        else
        {
            NextChar(&Result.y, Input[i]);
        }
        
        i++;
    }
    
    
    return Result;
}

internal v2
GetCoordsFromChar(char *Input)
{
    string_copy_buffer N1;
    Clear(&N1);
    string_copy_buffer N2;
    Clear(&N2);
    
    bool FirstNum = true;
    int i = 0;
    while(Input[i] != 0)
    {
        if (FirstNum)
        {
            if (Input[i] != ',')
            {
                NextChar(&N1, Input[i]);
            }
            else
            {
                FirstNum = false;
            }
        }
        else
        {
            NextChar(&N2, Input[i]);
        }
        
        i++;
    }
    
    return v2(atoi(N1.Value), atoi(N2.Value));
}

internal v2
GetCoordsFromChar(char *Input, int N2)
{
    string_copy_buffer N1;
    Clear(&N1);
    
    bool FirstNum = true;
    int i = 0;
    while(Input[i] != 0)
    {
        if (FirstNum)
        {
            if (Input[i] != ',')
            {
                NextChar(&N1, Input[i]);
            }
            else
            {
                break;
            }
        }
        i++;
    }
    
    return v2(atoi(N1.Value), N2);
}



#endif //MENU_H