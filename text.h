#ifndef TEXT_H
#define TEXT_H

#ifndef MEMORYMANAGER_H
//#pragma message ("text.h requires memorymanager.h")
#endif
#ifndef RENDERER_H
//#pragma message ("text.h requires renderer.h")
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

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
    int AX;
    int LSB;
    int C_X1;
    int C_Y1;
    int C_X2;
    int C_Y2;
    real32 Scale;
    
    texture Tex;
    
    uint32 Color;
    int32 Width;
    int32 Height;
    int32 Pitch;
    void* Memory;
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

#endif //TEXT_H
