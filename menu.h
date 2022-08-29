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
    bitmap_id Regular;
    bitmap_id Alternative;
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
    uint32 ControllerIndex;
    
    uint32 CurrentColor;
    uint32 DefaultColor;
    uint32 HoverColor;
    uint32 DefaultTextColor;
    uint32 HoverTextColor;
    
    bitmap_id CurrentTexture;
    
    bitmap_id DefaultTexture;
    bitmap_id ClickedTexture;
    bitmap_id ActiveTexture;
    bitmap_id ActiveClickedTexture;
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
    v2 DefaultCoords;
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
    bool MenuClicked;
    bool ButtonHoverFlag;
    
    platform_cursor_mode CursorMode;
};

struct menu_controller
{
    uint32 Index;
    bool32 IgnoreInputs;
    
    bool32 MouseControls;
    platform_button_state *Enter;
    platform_button_state *Edit;
    
    // Controller Controls
    platform_button_state *ForwardActive[2];
    platform_button_state *BackwardActive;
    
    // Textbox Inputting
    platform_button_state *Left;
    platform_button_state *Right;
    platform_button_state *Backspace;
    platform_button_state *Period;
    platform_button_state *Numbers[10];
    platform_button_state *Paste;
    char *Clipboard;
    
    // Mouse Controls
    v2 MouseCoords;
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
    
    qlib_bool Edit;
    
    menu_grid_row Rows[10];
    
    v2 Coords; // Default at origin. Used for moving around menu
    v2 DefaultCoords; // Default top left coords for background color
    v2 Dim;
    
    v2 DefaultDim;
    v2 ScreenDim;
    
    int Padding;
    int DefaultPadding;
    int OldPadding;
    v2 LastMouseCoords;
    
    int NumOfComponents;
    int MaxNumOfComponents;
    menu_component Components[20];
    
    arr ButtonsStorage; // menu_component_button
    arr TextBoxesStorage; // menu_component_button
    arr TextsStorage; // menu_component_button
    arr LogosStorage; // menu_component_button
    arr CheckBoxesStorage; // menu_component_button
    
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
    bitmap_id BackgroundBitmap;
    
    menu_controller Controller;
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

//
//  Menu Tags
//

struct menu_tag
{
    u32 id;
    char *string;
};

struct menu_tags
{
    u32 NumOfTags;
    menu_tag Tags[10];
};



// End of tags

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
        menu_component_button *Button = (menu_component_button*)Cursor->Data;
        if (CoordsInRect(MouseCoords, Cursor->Coords, Cursor->Dim)) {
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
    
    MComp->Data = ArrUseNext(Menu->ButtonsStorage, Button, MComp->Data, menu_component_button);
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
    
    //MComp->Data = qalloc((void*)Text, sizeof(menu_component_text));
    MComp->Data = ArrUseNext(Menu->TextsStorage, Text, MComp->Data, menu_component_text);
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
    
    MComp->Data = ArrUseNext(Menu->TextBoxesStorage, TextBox, MComp->Data, menu_component_textbox);
    //MComp->Data = qalloc((void*)TextBox, sizeof(menu_component_textbox));
    MenuResizeTextBox(Menu, MComp, 0);
    
    Menu->TextBoxes = MenuAddToComponentList(Menu->TextBoxes, MComp);
    return MComp;
}


// menu_component_logo
internal void
MenuUpdateLogoPadding(menu *Menu, menu_component *MComp)
{
    MComp->PaddingDim = MComp->Dim + (Menu->Padding * 2);
}

internal void
MenuResizeLogo(menu *Menu, menu_component *MComp, v2 ResizeFactors, assets *Assets)
{
    menu_component_logo *Logo = (menu_component_logo*)MComp->Data;
    MComp->Dim = ResizeEquivalentAmount(MComp->DefaultDim, ResizeFactors.y);
    //TextureResize(&Logo->Tex, Assets, MComp->Dim);
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
    MComp->Data = ArrUseNext(Menu->LogosStorage, Logo, MComp->Data, menu_component_logo);
    //MComp->Data = qalloc((void*)Logo, sizeof(menu_component_logo));
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
MenuResizeCheckBox(menu *Menu, menu_component *MComp, v2 ResizeFactors, assets *Assets)
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
    
    MComp->Data = ArrUseNext(Menu->CheckBoxesStorage, CheckBox, MComp->Data, menu_component_checkbox);
    //MComp->Data = qalloc((void*)CheckBox, sizeof(menu_component_checkbox));
    //MenuResizeCheckBox(Menu, MComp, 0, 0);
    
    Menu->CheckBoxes = MenuAddToComponentList(Menu->CheckBoxes, MComp);
    return MComp;
}

// menu
internal bool
MenuMenuClicked(menu *Menu, v2 MouseCoords)
{
    if (CoordsInRect(MouseCoords, 
                     Menu->Coords - (Menu->Dim/2 + Menu->Padding) + (Menu->ScreenDim/2),
                     Menu->Dim + (Menu->Padding * 2))) 
        return true;
    return false;
}

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
MenuInit(menu *Menu)
{
    Menu->NumOfComponents = 0;
    Menu->MaxNumOfComponents = ArrayCount(Menu->Components);
    memset(Menu->Components, 0, Menu->MaxNumOfComponents * sizeof(menu_component));
    //memset(&Menu->MenuButtons, 0, Menu->MenuButtons.MaxSize * Menu->MenuButtons.TypeSize);
    
    ArrInit(&Menu->ButtonsStorage, 10, sizeof(menu_component_button));
    ArrInit(&Menu->TextBoxesStorage, 10, sizeof(menu_component_textbox));
    ArrInit(&Menu->TextsStorage, 10, sizeof(menu_component_text));
    ArrInit(&Menu->LogosStorage, 10, sizeof(menu_component_logo));
    ArrInit(&Menu->CheckBoxesStorage, 10, sizeof(menu_component_checkbox));
    
    Menu->Buttons = 0;
    Menu->TextBoxes = 0;
    Menu->Texts = 0;
    Menu->Logos = 0;
    Menu->CheckBoxes = 0;
    Menu->BackgroundBitmap = 0;
    Menu->BackgroundColor = 0;
    
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
            CheckBox->ControllerIndex = 0;
        }
    }
    
    Menu->ActiveIndex = 0;
    Menu->Reset = false;
}

internal void
PlatformKeyboardToMenuController(platform_keyboard_input *Keyboard, menu_controller *Controller)
{
    Controller->ForwardActive[0] = &Keyboard->S;
    Controller->ForwardActive[1] = &Keyboard->Tab;
    Controller->BackwardActive = &Keyboard->W;
    Controller->Enter = &Keyboard->Enter;
    
    Controller->Left = &Keyboard->Left;
    Controller->Right = &Keyboard->Right;
    Controller->Backspace = &Keyboard->Backspace;
    Controller->Period = &Keyboard->Period;
    
    for (int i = 0; i < 10; i++)
        Controller->Numbers[i] = &Keyboard->Numbers[i];
    
    if (KeyDown(&Keyboard->Ctrl) && OnKeyDown(&Keyboard->V))
        Controller->Paste = &Keyboard->V;
    else
        Controller->Paste = &Keyboard->Ctrl;
    Controller->Clipboard = Keyboard->Clipboard;
    
    Controller->Edit = &Keyboard->F6;
    
    Controller->MouseControls = false;
}

internal void
PlatformMouseToMenuController(platform_mouse_input *Mouse, menu_controller *Controller)
{
    Controller->MouseCoords = v2(Mouse->X, Mouse->Y);
    Controller->Enter = &Mouse->Left;
    Controller->MouseControls = true;
}

internal void
PlatformControllerToMenuController(platform_controller_input *PlatformController, menu_controller *Controller)
{
    Controller->ForwardActive[0] = &PlatformController->DPadDown;
    Controller->BackwardActive = &PlatformController->DPadUp;
    Controller->Enter = &PlatformController->A;
    Controller->MouseControls = false;
}

internal menu_events*
HandleMenuEvents(menu *Menu, menu_controller *Controller)
{
    Menu->Events.ButtonClicked = -1;
    Menu->Events.TextBoxClicked = -1;
    
    if (!Controller->MouseControls) {
        if (OnKeyDown(Controller->ForwardActive[0]))
            IncrActive(Menu);
        if (OnKeyDown(Controller->ForwardActive[1]))
            IncrActive(Menu);
        if (OnKeyDown(Controller->BackwardActive))
            DecrActive(Menu);
        
        if (OnKeyDown(Controller->Enter)) {
            menu_component *C = Menu->ActiveComponents[Menu->ActiveIndex];
            if (C->Type == menu_component_type::Button)
                Menu->Events.ButtonClicked = C->ID;
            else if (C->Type == menu_component_type::CheckBox) {
                menu_component_checkbox *CheckBox = (menu_component_checkbox*)C->Data;
                
                bool32 IgnoreInputs = false;
                menu_component *OtherC = Menu->CheckBoxes;
                while(OtherC != 0)
                {
                    menu_component_checkbox *OtherCheckBox = (menu_component_checkbox*)OtherC->Data;
                    if (OtherCheckBox->ControllerIndex == Controller->Index && 
                        CheckBox != OtherCheckBox &&
                        OtherCheckBox->Clicked)
                        IgnoreInputs = true;
                    OtherC = OtherC->NextSameType;
                }
                
                if (!IgnoreInputs) {
                    if (CheckBox->Clicked && Controller->Index == CheckBox->ControllerIndex) {
                        CheckBox->ControllerIndex = 0;
                        CheckBox->Clicked = false;
                    }
                    else if (!CheckBox->Clicked) {
                        CheckBox->ControllerIndex = Controller->Index;
                        CheckBox->Clicked = true;
                    }
                }
                Menu->Events.CheckBoxClicked = C->ID;
            }
        }
        
        menu_component *ActiveTextBox = Menu->ActiveComponents[Menu->ActiveIndex];
        if (ActiveTextBox->Type == menu_component_type::TextBox)
        {
            for (int i = 0; i < 10; i++) {
                if (OnMessage(Controller->Numbers[i]))
                    MenuTextBoxAddChar(ActiveTextBox, IntToString(i));
            }
            
            if (OnMessage(Controller->Left))
                MenuTextBoxArrowKeysMoveCursor(ActiveTextBox, menu_direction::Left);
            
            if (OnMessage(Controller->Right))
                MenuTextBoxArrowKeysMoveCursor(ActiveTextBox, menu_direction::Right);
            if (KeyDown(Controller->Paste))
                MenuTextBoxReplaceText(ActiveTextBox, Controller->Clipboard);
            
            if (OnMessage(Controller->Backspace))
                MenuTextBoxRemoveChar(ActiveTextBox);
            if (OnMessage(Controller->Period))
                MenuTextBoxAddChar(ActiveTextBox, ".");
        }
        
        Menu->ActiveComponents[Menu->ActiveIndex]->Active = true;
    }
    else if (Controller->MouseControls) {
        if (Menu->ActiveComponents[Menu->ActiveIndex]->Type == menu_component_type::Button)
            Menu->ActiveComponents[Menu->ActiveIndex]->Active = false;
        
        if (OnKeyDown(Controller->Enter)) {
            Menu->Events.ButtonClicked = MenuButtonClicked(Menu, Controller->MouseCoords);
            Menu->Events.TextBoxClicked = MenuTextBoxClicked(Menu, Controller->MouseCoords);
            UpdateActiveIndex(Menu);
            if (Menu->Events.ButtonClicked == -1 && Menu->Events.TextBoxClicked -1) {
                Menu->Events.MenuClicked = MenuMenuClicked(Menu, Controller->MouseCoords);
                Menu->LastMouseCoords = Controller->MouseCoords;
            }
            else
                Menu->Events.MenuClicked = false;
        }
        
        menu_component *ActiveTextBox = Menu->ActiveComponents[Menu->ActiveIndex];
        if (ActiveTextBox->Type == menu_component_type::TextBox)
        {
            if (KeyDown(Controller->Enter))
                MenuTextBoxMouseMoveCursor(ActiveTextBox, Controller->MouseCoords);
        }
        
        if (KeyDown(Controller->Enter)) {
            if (Menu->Events.MenuClicked) {
                v2 MouseChange = Controller->MouseCoords - Menu->LastMouseCoords;
                Menu->Coords = Menu->Coords + MouseChange;
                Menu->LastMouseCoords = Controller->MouseCoords;
            }
        }
        
        if (MenuButtonHovered(Menu, Controller->MouseCoords))
            Menu->Events.CursorMode = platform_cursor_mode::Hand;
        else
            Menu->Events.CursorMode = platform_cursor_mode::Arrow;
    }
    
    return &Menu->Events;
}

internal void
ResizeMenu(menu *Menu, v2 BufferDim, assets *Assets)
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
            MenuResizeLogo(Menu, MComp, ResizeFactors, Assets);
        if (MComp->Type == menu_component_type::CheckBox)
            MenuResizeCheckBox(Menu, MComp, ResizeFactors, Assets);
    }
}
internal void
PaddingResizeMenu(menu *Menu, assets *Assets)
{
    v2 ResizeFactors = v2(1, 1);
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
            MenuUpdateLogoPadding(Menu, MComp);
        if (MComp->Type == menu_component_type::CheckBox)
            MenuResizeCheckBox(Menu, MComp, ResizeFactors, Assets);
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
        C->DefaultCoords.x = (-R->Dim.x)/2 + (Menu->ScreenDim.x/2);
        for (int  i = 0; i < C->GridCoords.x; i++)
            C->DefaultCoords.x += Menu->Rows[(int)C->GridCoords.y].MenuGridColumnWidth[i];
        
        // Find biggest row
        if (Menu->Dim.x < R->Dim.x) {
            Menu->Dim.x = R->Dim.x;
            Menu->DefaultCoords.x = C->DefaultCoords.x - (Menu->Padding);
        }
        // Calculate row location to center
        C->DefaultCoords.y = ((-Menu->Dim.y)/2) + ((R->Dim.y - C->PaddingDim.y)/2) + (Menu->ScreenDim.y/2);
        for (int i = 0; i < C->GridCoords.y; i++) {
            menu_grid_row* tempR = &Menu->Rows[i];
            C->DefaultCoords.y += tempR->Dim.y;
        }
    }
    
    menu_component* C = &Menu->Components[0];
    Menu->DefaultCoords.y = C->DefaultCoords.y - (Menu->Padding);
}
inline void UpdateMenu(menu *Menu, v2 BufferDim, assets *Assets) 
{
    ResizeMenu(Menu, BufferDim, Assets);
    UpdateMenu(Menu);
}

internal void
DrawMenu(menu *Menu, v2 TopLeftCornerCoords, v2 PlatformDim, real32 Z)
{
    qlibCoordSystem(QLIB_TOP_RIGHT);
    qlibBlendMode(BLEND_MODE_GL_SRC_ALPHA);
    
    for (int i = 0; i < Menu->NumOfComponents; i++) {
        menu_component *MComp = &Menu->Components[i];
        v2 Padding = (MComp->Dim - MComp->PaddingDim)/2;
        MComp->Coords = MComp->DefaultCoords - Padding + Menu->Coords;
        v2 CompCoords = MComp->Coords;
        
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
            v2 TextCoords = CompCoords + ((MComp->Dim - SDim)/2);
            
            Push(v3(CompCoords, Z), MComp->Dim, Button->CurrentColor);
            FontStringPrint(&Button->FontString, TextCoords);
            
        }
        else if (MComp->Type == menu_component_type::TextBox) {
            menu_component_textbox *TextBox = (menu_component_textbox*)MComp->Data;
            Push(v3(CompCoords, 50.0f), MComp->Dim, TextBox->CurrentColor);
            
            if (MComp->Active) {
                v2 SDim = FontStringGetDim(&TextBox->FontString);
                TextBox->TextCoords = CompCoords;
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
                Push(v3(CursorX, MComp->Coords.y, 100.0f), v2(5.0f, MComp->Dim.y), 0xFF000000);
            }
            
            FontStringPrint(&TextBox->FontString, TextBox->TextCoords, MComp->Coords, MComp->Dim);
            //FontStringPrint(&TextBox->FontString, TextBox->TextCoords);
        }
        else if (MComp->Type == menu_component_type::Text) {
            menu_component_text *Text = (menu_component_text*)MComp->Data;
            v2 TextCoords = MComp->Coords;
            if (MComp->AlignWith != 0)
                TextCoords.x = MComp->Coords.x + MComp->AlignWith->Dim.x - MComp->Dim.x;
            FontStringPrint(&Text->FontString, TextCoords);
        }
        else if (MComp->Type == menu_component_type::Logo) {
            menu_component_logo *Logo = (menu_component_logo*)MComp->Data;
            Push(v3(CompCoords, 100.0f), MComp->Dim, Logo->Regular);
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
            
            Push(v3(CompCoords, 100.0f), MComp->Dim, CheckBox->CurrentTexture);
        }
    }
    
    if (Menu->BackgroundBitmap.id == 0)
        Push(v3(Menu->Coords - (Menu->Dim/2 + Menu->Padding) + (Menu->ScreenDim/2), 50.0f), Menu->Dim + (Menu->Padding * 2), Menu->BackgroundColor);
    else {
        v2 BackgroundCoords = TopLeftCornerCoords - 5;
        v2 BackgroundDim = 0;
        if (PlatformDim.x >= PlatformDim.y)
            BackgroundDim = v2(PlatformDim.x + 5, PlatformDim.x + 5);
        else
            BackgroundDim = v2(PlatformDim.y + 5, PlatformDim.y + 5);
        
        BackgroundCoords.y -= ((BackgroundDim.y - (PlatformDim.y + 5)) / 2);
        
        Push(v3(BackgroundCoords, 0.0f), BackgroundDim, Menu->BackgroundBitmap);
    }
    
    qlibCoordSystem(QLIB_CENTER);
}
inline void DrawMenu(menu *Menu, real32 Z) { DrawMenu(Menu, 0,0, Z); }

internal v2_string
Getv2StringFromChar(char *Input)
{
    v2_string Result = {};
    
    bool FirstNum = true;
    int i = 0;
    while(Input[i] != 0) {
        if (FirstNum) {
            if (Input[i] != ',')
                NextChar(&Result.x, Input[i]);
            else
                FirstNum = false;
        }
        else
            NextChar(&Result.y, Input[i]);
        
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
    while(Input[i] != 0) {
        if (FirstNum) {
            if (Input[i] != ',')
                NextChar(&N1, Input[i]);
            else
                FirstNum = false;
        }
        else
            NextChar(&N2, Input[i]);
        
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
    while(Input[i] != 0) {
        if (FirstNum) {
            if (Input[i] != ',')
                NextChar(&N1, Input[i]);
            else
                break;
        }
        i++;
    }
    
    return v2(atoi(N1.Value), N2);
}

internal v2
GetGridCoords(char *Input, int *X, int *Y)
{
    v2_string Coords = Getv2StringFromChar(Input);
    
    if (Equal(Coords.y.Value, "Y++"))
        return v2(atoi(Coords.x.Value), (*Y)++);
    else if (Equal(Coords.y.Value, "Y"))
        return v2(atoi(Coords.x.Value), (*Y));
    else if (Equal(Coords.y.Value, "Y+1"))
        return v2(atoi(Coords.x.Value), (*Y)+1);
    else if (Equal(Coords.y.Value, "Y+2"))
        return v2(atoi(Coords.x.Value), (*Y)+2);
    else if (Equal(Coords.y.Value, "Y+3"))
        return v2(atoi(Coords.x.Value), (*Y)+3);
    else
        return v2(atoi(Coords.x.Value), atoi(Coords.y.Value));
}

internal void
ReadMenuFromFile(menu *Menu, const char* FileName, assets *Assets, pair_int_string *IDs, int NumOfIDs)
{
    entire_file EntireFile = ReadEntireFile(FileName);
    const char* Cursor = (const char*)EntireFile.Contents;
    
    if (Cursor == 0)
        return;
    
    bool FillCollection = false;
    
    int NumOfReplaceTokens = 0;
    menu_token ReplaceTokens[10];
    memset(ReplaceTokens, 0, ArrayCount(ReplaceTokens) * sizeof(menu_token));
    
    int NumOfCollections = 0;
    menu_token_collection Collections[10];
    memset(Collections, 0, ArrayCount(Collections) * sizeof(menu_token_collection));
    
    menu_token_collection *Collection = 0;
    
    int i = 0;
    while(1)
    {
        // Read Line
        char Line[100];
        memset(Line, 0, 100);
        int j = 0;
        while (Cursor[i] != 0x0A && Cursor[i] != 0x0D && i < (int)EntireFile.ContentsSize)
        {
            Line[j] = Cursor[i];
            j++;
            i++;
        }
        
        if (j != 0)
        {
            if (Line[0] == '#')
            {
                //fprintf(stderr, "%s\n", Line);
                
                if (Equal(Line + 1, "end"))
                    break;
                else
                {
                    Collection = &Collections[NumOfCollections++];
                    CopyBuffer(Collection->Type, Line+1, GetLength(Line+1));
                    FillCollection = true;
                }
            }
            else if (FillCollection)
            {
                //fprintf(stderr, "%s\n", Line);
                
                int CursorIndex = 0;
                int TokenIndex = 0;
                menu_token *Token = &Collection->Tokens[Collection->NumOfTokens++];
                while (Line[CursorIndex] != ':')
                {
                    if (Line[CursorIndex] != 0x20) {
                        Token->ID[TokenIndex] = Line[CursorIndex];
                        TokenIndex++;
                    }
                    CursorIndex++;
                }
                
                CursorIndex++;
                TokenIndex = 0;
                while (CursorIndex != j)
                {
                    if (Line[CursorIndex] != 0x20 || Equal(Token->ID, "Text")) {
                        Token->Value[TokenIndex] = Line[CursorIndex];
                        TokenIndex++;
                    }
                    CursorIndex++;
                }
            }
        }
        
        if (i >= (int)EntireFile.ContentsSize)
            break;
        
        i++;
    }
    DestroyEntireFile(EntireFile);
    
    // Replace
    for (int i = 1; i < NumOfCollections; i++)
    {
        menu_token_collection *Collection = &Collections[i];
        for (int j = 0; j < Collection->NumOfTokens; j++)
        {
            menu_token *Token = &Collection->Tokens[j];
            for (int k = 0; k < Collections[0].NumOfTokens; k++)
            {
                menu_token *ReplaceToken = &Collections[0].Tokens[k];
                
                if (Equal(Token->Value, ReplaceToken->ID))
                {
                    memset(Token->Value, 0, 100);
                    CopyBuffer(Token->Value, ReplaceToken->Value, GetLength(ReplaceToken->Value));
                }
            }
        }
    }
    
    int Y = 0;
    int X = 0;
    for (int i = 1; i < NumOfCollections; i++)
    {
        menu_token_collection *Collection = &Collections[i];
        if (Equal(Collection->Type, "Menu"))
        {
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "BackgroundColor"))
                    Menu->BackgroundColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "BackgroundTexture"))
                    Menu->BackgroundBitmap = GetIndexBitmap(Assets, Asset_MenuBackground, 0);
                else if (Equal(Token->ID,  "DefaultDim"))
                    Menu->DefaultDim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "DefaultPadding"))
                    Menu->DefaultPadding = atoi(Token->Value);
            }
        }
        else if (Equal(Collection->Type, "Logo"))
        {
            menu_component_logo Logo = {};
            v2 Dim;
            v2 GridCoords;
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "Texture"))
                    Logo.Regular = GetIndexBitmap(Assets, Asset_MenuLogo, atoi(Token->Value));
                else if (Equal(Token->ID,  "Dim")) 
                    Dim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "GridCoords")) 
                    GridCoords = GetGridCoords(Token->Value, &X, &Y);
            }
            MenuAddLogo(Menu, GridCoords, Dim, &Logo);
        }
        else if (Equal(Collection->Type, "Button"))
        {
            menu_component_button Button = {};
            v2 GridCoords;
            v2 Dim;
            int ID = 0;
            
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "ID")) 
                    ID = GetInt(IDs, NumOfIDs, Token->Value);
                else if (Equal(Token->ID,  "DefaultColor"))
                    Button.DefaultColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "HoverColor"))
                    Button.HoverColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "DefaultTextColor"))
                    Button.DefaultTextColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "HoverTextColor"))
                    Button.HoverTextColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "Text"))
                    FontStringSetText(&Button.FontString, Token->Value);
                else if (Equal(Token->ID,  "Font"))
                    Button.FontString.Font = GetFont(Assets, GetFirstFont(Assets, Asset_Font));
                else if (Equal(Token->ID,  "PixelHeight"))
                    Button.FontString.PixelHeight = (real32)atoi(Token->Value);
                else if (Equal(Token->ID,  "Dim")) 
                    Dim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "GridCoords")) 
                    GridCoords = GetGridCoords(Token->Value, &X, &Y);
            }
            
            MenuAddButton(Menu, ID, GridCoords, Dim, &Button);
        }
        else if (Equal(Collection->Type, "Text"))
        {
            menu_component_text Text = {};
            v2 GridCoords;
            v2 Dim;
            int ID = 0;
            menu_component *Align = 0;
            
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "ID")) 
                    ID = GetInt(IDs, NumOfIDs, Token->Value);
                else if (Equal(Token->ID,  "DefaultTextColor"))
                    Text.DefaultTextColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "Text"))
                    FontStringSetText(&Text.FontString, Token->Value);
                else if (Equal(Token->ID,  "Font"))
                    Text.FontString.Font = GetFont(Assets, GetFirstFont(Assets, Asset_Font));
                else if (Equal(Token->ID,  "PixelHeight"))
                    Text.FontString.PixelHeight = (real32)atoi(Token->Value);
                else if (Equal(Token->ID,  "Dim")) 
                    Dim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "Align")) 
                    Align = MenuGetComponent(Menu, IDs, NumOfIDs, Token->Value);
                else if (Equal(Token->ID,  "GridCoords")) 
                    GridCoords = GetGridCoords(Token->Value, &X, &Y);
            }
            
            MenuAddText(Menu, ID, GridCoords, &Text, Align);
        }
        else if (Equal(Collection->Type, "TextBox"))
        {
            menu_component_textbox TextBox = {};
            v2 GridCoords;
            v2 Dim;
            int ID = 0;
            
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "ID")) 
                    ID = GetInt(IDs, NumOfIDs, Token->Value);
                else if (Equal(Token->ID,  "DefaultTextColor"))
                    TextBox.CurrentTextColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "DefaultColor"))
                    TextBox.CurrentColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "Text"))
                    FontStringSetText(&TextBox.FontString, Token->Value);
                else if (Equal(Token->ID,  "Font"))
                    TextBox.FontString.Font = GetFont(Assets, GetFirstFont(Assets, Asset_Font));
                else if (Equal(Token->ID,  "PixelHeight"))
                    TextBox.FontString.PixelHeight = (real32)atoi(Token->Value);
                else if (Equal(Token->ID,  "Dim")) 
                    Dim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "GridCoords")) 
                    GridCoords = GetGridCoords(Token->Value, &X, &Y);
            }
            
            MenuAddTextBox(Menu, ID, GridCoords, Dim, &TextBox);
        }
        else if (Equal(Collection->Type, "CheckBox"))
        {
            menu_component_checkbox CheckBox = {};
            v2 GridCoords;
            v2 Dim;
            int ID = 0;
            
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "ID")) 
                    ID = GetInt(IDs, NumOfIDs, Token->Value);
                else if (Equal(Token->ID,  "DefaultTexture"))
                    CheckBox.DefaultTexture = GetIndexBitmap(Assets, Asset_DefaultCheckBox, atoi(Token->Value));
                else if (Equal(Token->ID,  "ActiveTexture"))
                    CheckBox.ActiveTexture = GetIndexBitmap(Assets, Asset_ActiveCheckBox, atoi(Token->Value));
                else if (Equal(Token->ID,  "ClickedTexture"))
                    CheckBox.ClickedTexture = GetIndexBitmap(Assets, Asset_HighlightCheckBox, atoi(Token->Value));
                else if (Equal(Token->ID,  "ActiveClickedTexture"))
                    CheckBox.ActiveClickedTexture = GetIndexBitmap(Assets, Asset_ActiveHighlightCheckBox, atoi(Token->Value));
                else if (Equal(Token->ID,  "Dim")) 
                    Dim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "GridCoords")) 
                    GridCoords = GetGridCoords(Token->Value, &X, &Y);
            }
            
            MenuAddCheckBox(Menu, ID, GridCoords, Dim, &CheckBox);
        }
        else if (Equal(Collection->Type, "GridCoords"))
        {
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "Y")) 
                    Y = atoi(Token->Value);
            }
        }
    }
}

// Returns true if events should be processed
internal bool
DoMenu(menu *Menu, const char *FileName, 
       platform *p, assets *Assets, 
       pair_int_string *IDs, int NumOfIDs)
{
    switch (ActiveInputType(p->Input.ActiveInput))
    {
        case Keyboard: {
            PlatformKeyboardToMenuController(&p->Input.Keyboard, &Menu->Controller);
        } break;
        case Mouse: {
            PlatformMouseToMenuController(&p->Input.Mouse, &Menu->Controller);
        } break;
        case Controller: {
            PlatformControllerToMenuController(GetActiveController(&p->Input), &Menu->Controller);
        } break;
    }
    Menu->Controller.Index = p->Input.ActiveInput;
    
    if (!Menu->Controller.MouseControls && OnKeyDown(Menu->Controller.Edit))
        Toggle(&Menu->Edit);
    
    if (!Menu->Initialized)
    {
        MenuInit(Menu);
        ReadMenuFromFile(Menu, FileName, Assets, IDs, NumOfIDs);
        MenuSortActiveComponents(Menu);
    }
    else if (Menu->Reset)
    {
        PlatformSetCursorMode(&p->Input.Mouse, platform_cursor_mode::Arrow);
        MenuReset(Menu);
        //MenuSortActiveComponents(Menu);
    }
    
    if (QlibBoolTrue(&Menu->Edit))
    {
        font_string EditMenuString = {};
        const char* EM = "Edit Menu";
        FontStringInit(&EditMenuString, GetFont(Assets, GetFirstFont(Assets, Asset_Font)), EM, 50, 0xFFFFFFFF);
        v2 SDim = FontStringGetDim(&EditMenuString);
        FontStringPrint(&EditMenuString, v2((p->Dimension.Width/2)-(int)SDim.x-10, -p->Dimension.Height/2 + 10));
        
        //*Menu = {};
        MenuInit(Menu);
        ReadMenuFromFile(Menu, FileName, Assets, IDs, NumOfIDs);
        
        PaddingResizeMenu(Menu, Assets);
        UpdateMenu(Menu);
        return false;
    }
    else
    {
        HandleMenuEvents(Menu, &Menu->Controller);
        
        PlatformSetCursorMode(&p->Input.Mouse, Menu->Events.CursorMode);
        
        if (Menu->ScreenDim != GetDim(p)) 
            UpdateMenu(Menu, GetDim(p), Assets);
        
        return true;
    }
}

#endif //MENU_H