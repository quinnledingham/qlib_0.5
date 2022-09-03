#ifndef FONT_H
#define FONT_H

#ifndef MEMORYMANAGER_H
//#pragma message ("text.h requires memorymanager.h")
#endif
#ifndef RENDERER_H
//#pragma message ("text.h requires renderer.h")
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

// Info about font at PixelHeight
struct font_scale
{
    real32 PixelHeight;
    
    real32 Scale;
    int Ascent;
    int Descent;
    int LineGap;
    real32 ScaledAscent;
    real32 ScaledDescent;
};

struct font_char
{
    int Codepoint;
    real32 Scale;
    uint32 Color;
    
    int AX;
    int LSB;
    int C_X1;
    int C_Y1;
    int C_X2;
    int C_Y2;
    
    loaded_bitmap Bitmap;
};

#define FONTCACHEMAXSIZE 200 
struct font
{
    stbtt_fontinfo Info;
    entire_file TTFFile;
    
    int FontScalesCached;
    font_scale FontScales[10];
    
    int FontCharsCached = 0;
    font_char FontChars[FONTCACHEMAXSIZE];
};
typedef font loaded_font;

#define FONT_STRING_MAX_LENGTH 200
struct font_string
{
    font *Font;
    font_id FontID;
    
    font_char *Memory[FONT_STRING_MAX_LENGTH];
    real32 Advances[FONT_STRING_MAX_LENGTH];
    
    char Text[FONT_STRING_MAX_LENGTH];
    bool32 NewText;
    int Length;
    int MaxLength;
    
    v2 Dim;
    real32 PixelHeight;
    real32 Scale;
    real32 OldScale;
    uint32 Color;
};

internal loaded_font LoadFont(const char *FileName);

struct string_draw
{
    string Text;
    font_char *FontChars[MAX_STRING_LENGTH];
    real32 Advances[MAX_STRING_LENGTH];
    
    // Draw Settings
    font_id FontID;
    u32 Color;
    real32 PixelHeight;
    
    real32 LastPixelHeight; // The pixel height last time it got the dim
    real32 Scale;
    v2 Dim;
    
    string_draw();
    string_draw(string String);
    string_draw(char *Text);
};

#endif //FONT_H