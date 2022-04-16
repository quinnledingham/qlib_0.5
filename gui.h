#ifndef GUI_H
#define GUI_H


#if !defined(RAYLIB_H)
struct Texture2D
{
    
};
#endif

internal int
StringLength(char* String);

internal char*
StringConcat(char* Source, char* Add);

enum ComponentIDs
{
    Btn1,
    Btn2,
    Btn3,
    Btn4,
    PORT,
    IP,
    Restart,
    JOIN,
    GameStart,
    Quit
};

struct Button
{
    int TextX;
    int TextY;
    char* Text;
    Font* FontType;
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
    Font* FontType;
    uint32 TextColor;
};

struct TextBox
{
    int TextX;
    int TextY;
    char* Text;
    Font* FontType;
    int ID;
    int ShowCursor;
    
    uint32 Color;
    uint32 TextColor;
};

struct GUIComponent
{
    int X;
    int Y;
    
    int GridX;
    int GridY;
    int Width;
    int Height;
    
    int WidthP;
    int HeightP;
    
    GUIComponent* Next;
    GUIComponent* All;
    void* Data;
};

struct Column
{
    int Width;
};

struct Row
{
    int Width = 0;
    int Height = 0;
    Column Columns[10];
};

struct GUI
{
    int Initialized;
    Row Rows[10];
    
    int Width = 0;
    int Height = 0;
    int Padding = 0;
    GUIComponent* All;
    GUIComponent* Buttons;
    GUIComponent* TextBoxes;
    GUIComponent* Texts;
};


#endif //GUI_H
