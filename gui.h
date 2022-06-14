#ifndef GUI_H
#define GUI_H

struct Button
{
    char* Text;
    Font FontType;
    int ID;
    uint32 Color;
    uint32 RegularColor;
    uint32 HoverColor;
    uint32 TextColor;
};

struct Text
{
    char* Text;
    int ID;
    Font FontType;
    uint32 TextColor;
};

struct TextBox
{
    char* Text;
    Font FontType;
    int ID;
    int ShowCursor;
    
    uint32 Color;
    uint32 TextColor;
};

struct GUIComponent
{
    v2 Coords;
    v2 GridCoords;
    v2 Dim;
    v2 PaddingDim;
    v2 DefaultDim;
    
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

struct GUI
{
    int Initialized;
    Row Rows[10];
    
    v2 Dim;
    int Padding = 0;
    
    GUIComponent* All;
    GUIComponent* Buttons;
    GUIComponent* TextBoxes;
    GUIComponent* Texts;
    Arr Components;
    
    int Screen;
    
    v2 DefaultDim;
    int DefaultPadding = 0;
};

struct GUIEvents
{
    int BtnPressID;
    int TbPressID;
    bool32 BtnHoverFlag;
};
#endif //GUI_H
