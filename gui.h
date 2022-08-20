#ifndef GUI_H
#define GUI_H

struct gui_component_button
{
    font_string FontString;
    real32 DesignTextPixelHeight;
    
    u32 Color;
    
    u32 DefaultColor;
    u32 HoverColor;
    u32 DefaultTextColor;
    u32 HoverTextColor;
};

struct gui_component_text
{
    font_string FontString;
    u32 Color;
};

enum struct gui_component_type
{
    Button,
    Text,
};

struct gui_component
{
    v2 Coords;
    v2 Dim;
    v4 Margins; // Right, Top, Left, Bottom
    
    v2 DesignCoords;
    v2 DesignDim;
    
    gui_component_type Type;
    union
    {
        gui_component_text Text;
        gui_component_button Button;
    };
};

struct gui
{
    bool32 Initialized;
    
    v2 Coords; // Top Left Coords of GUI
    v2 Dim; // Dim of GUI
    
    // gui is designed with a screen size which is used to resize
    // to other screen sizes.
    v2 DesignScreenDim;
    v2 DesignDim;
    v4 DesignMargins;
    
    u32 NumOfComponents;
    gui_component Components[20];
};

struct gui_controller
{
    platform_button_state *Forward[2];
    platform_button_state Backward;
    
    v2 MouseCoords;
};

internal void
GUIInit(gui *GUI)
{
    
}

internal bool32
GUIRun(gui *GUI)
{
    
    
    return true;
}

#endif //GUI_H
