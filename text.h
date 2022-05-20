#ifndef TEXT_H
#define TEXT_H

struct FontChar
{
    int codepoint;
    int32 Width;
    int32 Height;
    int32 Pitch;
    int AX;
    int C_X1;
    int C_Y1;
    int C_X2;
    int C_Y2;
    real32 Advance;
    Texture Tex;
    uint32 Color;
    float Scale;
    void* Memory;
};

#define FONTCACHEMAXSIZE 200 
struct Font
{
    stbtt_fontinfo Info;
    int Size = 0;
    int Ascent;
    float Scale;
    float ScaleIn;
    FontChar Memory[FONTCACHEMAXSIZE];
    entire_file TTFFile;
};


//internal PrintOnScreenReturn
//PrintOnScreen(game_offscreen_buffer *Buffer, char* text, int xin, int yin, float scalein, uint32 color, Rect* alignRect);
//void PrintOnScreen(Font* SrcFont, char* SrcText, int InputX, int InputY, uint32 Color);

#endif //TEXT_H
