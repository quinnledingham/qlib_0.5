#ifndef TEXT_H
#define TEXT_H

#ifndef MEMORYMANAGER_H
#pragma message ("text.h requires memorymanager.h")
#endif
#ifndef RENDERER_H
#pragma message ("text.h requires renderer.h")
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

struct FontChar
{
    int codepoint;
    int32 Width;
    int32 Height;
    int32 Pitch;
    int AX;
    int LSB;
    int C_X1;
    int C_Y1;
    int C_X2;
    int C_Y2;
    Texture Tex;
    uint32 Color;
    real32 Scale;
    void* Memory;
};

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

#define FONTCACHEMAXSIZE 200 
struct Font
{
    stbtt_fontinfo Info;
    real32 Ascent;
    real32 Descent;
    real32 Scale;
    real32 ScaleIn;
    
    font_scale FontScales[10];
    
    int Size = 0;
    FontChar Memory[FONTCACHEMAXSIZE];
    entire_file TTFFile;
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
    
    Texture Tex;
    
    uint32 Color;
    int32 Width;
    int32 Height;
    int32 Pitch;
    void* Memory;
};

struct font
{
    stbtt_fontinfo Info;
    entire_file TTFFile;
    
    int FontScalesCached;
    font_scale FontScales[10];
    
    int FontCharsCached = 0;
    font_char FontChars[FONTCACHEMAXSIZE];
};

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

internal void
FilenameSearchModify(char* filename, char* result)
{
    int j = 0;
    char* cursor = filename;
    while (*cursor != 0) {
        if (*cursor == '.')
            result[j] = '_';
        else
            result[j] = *cursor;
        cursor++;
        j++;
    }
    result[j] = 0;
}

internal void
FilenameCapitalize(char* filename, char* result)
{
    int j = 0;
    char* cursor = filename;
    while (*cursor != 0) {
        if (*cursor == '_' || *cursor == '.')
            result[j] = '_';
        else
        {
            char c = *cursor;
            result[j] = c - 32;
        }
        cursor++;
        j++;
    }
    result[j] = 0;
}

internal loaded_bitmap
LoadGlyphBitmap(Font *font, u32 Codepoint, uint32 Color)
{
    loaded_bitmap Result = {};
    if(font->TTFFile.ContentsSize != 0)
    {
        int Width, Height, XOffset, YOffset;
        u8 *MonoBitmap = stbtt_GetCodepointBitmap(&font->Info, 0, font->Scale,
                                                  Codepoint, &Width, &Height, &XOffset, &YOffset);
        
        Result.Width = Width;
        Result.Height = Height;
        Result.Pitch = Result.Width * BITMAP_BYTES_PER_PIXEL;
        Result.Memory = malloc(Height*Result.Pitch);
        Result.Free = Result.Memory;
        
        u8 *Source = MonoBitmap;
        u8 *DestRow = (u8 *)Result.Memory;
        for(s32 Y = 0; Y < Height; ++Y)
        {
            u32 *Dest = (u32 *)DestRow;
            for(s32 X = 0; X < Width; ++X)
            {
                u8 Gray = *Source++;
                u32 Alpha = ((Gray << 24) |
                             (Gray << 16) |
                             (Gray <<  8) |
                             (Gray << 0));
                Color &= 0x00FFFFFF;
                Alpha &= 0xFF000000;
                Color += Alpha;
                *Dest++ = Color;
            }
            
            DestRow += Result.Pitch;
        }
        
        stbtt_FreeBitmap(MonoBitmap, 0);
    }
    
    return (Result);
}

internal void
LoadFont(Font *F, char* FileName, real32 ScaleIn)
{
    entire_file File = ReadEntireFile(FileName);
    
    stbtt_fontinfo Info;
    stbtt_InitFont(&Info, (u8 *)File.Contents, stbtt_GetFontOffsetForIndex((u8 *)File.Contents, 0));
    
    float Scale = stbtt_ScaleForPixelHeight(&Info, ScaleIn);
    
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&Info, &ascent, &descent, &lineGap);
    
    F->Info = Info;
    F->Scale = Scale;
    F->ScaleIn = ScaleIn;
    F->TTFFile = File;
}

internal void
ResizeFont(Font *Font, real32 ScaleIn)
{
    float Scale = stbtt_ScaleForPixelHeight(&Font->Info, ScaleIn);
    
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&Font->Info, &ascent, &descent, &lineGap);
    
    Font->Ascent = roundf((real32)ascent * Scale);
    Font->Descent = roundf((real32)descent * Scale);
    Font->Scale = Scale;
    Font->ScaleIn = ScaleIn;
    
}

internal Font*
LoadFont(const char* FileName, real32 ScaleIn)
{
    Font *F = (Font*)qalloc(sizeof(Font));
    
    F->TTFFile = ReadEntireFile(FileName);
    stbtt_InitFont(&F->Info, (u8 *)F->TTFFile.Contents, stbtt_GetFontOffsetForIndex((u8 *)F->TTFFile.Contents, 0));
    ResizeFont(F, ScaleIn);
    
    return F;
}

internal void
UnLoadFont(Font DelFont)
{
    
}

internal FontChar*
LoadFontChar(Font* font, int codepoint, uint32 Color)
{
    // If codepoint is already loaded
    for (int i = 0; i < font->Size; i++)
    {
        if (font->Memory[i].codepoint == codepoint &&
            font->Memory[i].Color == Color &&
            font->Memory[i].Scale == font->Scale)
        {
            return &font->Memory[i];
        }
    }
    
    // If the codepoint has to be loaded
    
    // how wide is this character
    int ax;
    int lsb;
    stbtt_GetCodepointHMetrics(&font->Info, codepoint, &ax, &lsb);
    
    // get bounding box for character (may be offset to account for chars that dip above or below the line
    int c_x1, c_y1, c_x2, c_y2;
    stbtt_GetCodepointBitmapBox(&font->Info, codepoint, font->Scale, font->Scale, &c_x1, &c_y1, &c_x2, &c_y2);
    
    // render character
    loaded_bitmap Temp = LoadGlyphBitmap(font, codepoint, Color);
    
    FontChar NewFontChar = {};
    NewFontChar.codepoint = codepoint;
    NewFontChar.Width = Temp.Width;
    NewFontChar.Height = Temp.Height;
    NewFontChar.Pitch = Temp.Pitch;
    NewFontChar.Memory = Temp.Memory;
    NewFontChar.AX = ax;
    NewFontChar.LSB = lsb;
    NewFontChar.C_X1 = c_x1;
    NewFontChar.C_Y1 = c_y1;
    NewFontChar.C_X2 = c_x2;
    NewFontChar.C_Y2 = c_y2;
    NewFontChar.Color = Color;
    NewFontChar.Scale = font->Scale;
    
#if QLIB_OPENGL
    Image SrcImage = {};
    SrcImage.x = NewFontChar.Width;
    SrcImage.y = NewFontChar.Height;
    SrcImage.n = NewFontChar.Pitch;
    SrcImage.data = (unsigned char*)NewFontChar.Memory;
    NewFontChar.Tex.Init(&SrcImage);
    stbi_image_free(SrcImage.data);
#endif
    
    font->Memory[font->Size] = NewFontChar;
    font->Size++;
    return &font->Memory[font->Size - 1];
}

internal v2
GetStringDimensions(Font* SrcFont, char* SrcText)
{
    real32 X = 0;
    int StrLength = GetLength(SrcText);
    int BiggestY = 0;
    
    for (int i = 0; i < StrLength; i++)
    {
        FontChar *NextChar = LoadFontChar(SrcFont,  SrcText[i], 0xFF000000);
        
        int Y = -1 *  NextChar->C_Y1;
        if(BiggestY < Y)
            BiggestY = Y;
        
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&SrcFont->Info, SrcText[i], &ax, &lsb);
        
        // add kerning
        int kern = stbtt_GetCodepointKernAdvance(&SrcFont->Info, SrcText[i], SrcText[i + 1]);
        
        X += ((kern + ax) * SrcFont->Scale);
    }
    
    int StringWidth = (int)X;
    
    v2 Dimension = v2((real32)StringWidth, (real32)BiggestY);
    return(Dimension);
}

internal v2
GetStringDimensions(Font *SrcFont, strinq *SrcText)
{
    if ((SrcText->Dim.x == 0 && SrcText->Dim.y == 0) || (SrcFont->Scale != SrcText->DimScale)) {
        SrcText->DimScale = SrcFont->Scale;
        SrcText->Dim = GetStringDimensions(SrcFont, SrcText->Data);
    }
    return SrcText->Dim;
}

internal void
SaveMemoryToHeaderFile(FILE* File, char* MemoryName, void* Memory, int MemorySize)
{
    fprintf(File,
            "const unsigned char %s[%d] = {",
            MemoryName,
            MemorySize
            );
    
    unsigned char* MemoryCursor = (unsigned char*)Memory;
    for(int i = 0; i < MemorySize; i++)
    {
        fprintf(File,
                "%d ,",
                *MemoryCursor);
        *MemoryCursor++;
    }
    
    fprintf(File,
            "};\n"
            "\n"
            );
}

internal void
SaveInsideFontMemoryToHeaderFile(FILE* File, char* FontName,  Font* SaveFont, int InsideSize)
{
    fprintf(File,
            "const unsigned char %s[%d] = {",
            Concat(FontName, "FontChar"),
            (int)(sizeof(FontChar) * SaveFont->Size)
            );
    for (int i = 0; i < SaveFont->Size; i++)
    {
        //MemorySize += ((SaveFont->Memory[i].Width * SaveFont->Memory[i].Height) * BITMAP_BYTES_PER_PIXEL);
        unsigned char* MemoryCursor = (unsigned char*)&SaveFont->Memory[i];
        for(int j = 0; j < InsideSize; j++)
        {
            fprintf(File,
                    "%d ,",
                    *MemoryCursor);
            *MemoryCursor++;
        }
    }
    fprintf(File,
            "};\n"
            "\n"
            );
}

internal void
SaveIntToHeaderFile(FILE *File, char* IntName, int Value)
{
    fprintf(File, "int %s = %d;\n",
            IntName,
            Value
            );
}

internal void
SaveInt32ToHeaderFile(FILE *File, char* IntName, int32 Value)
{
    fprintf(File, "int %s = %d;\n",
            IntName,
            Value
            );
}

internal void
SaveFloatToHeaderFile(FILE *File, char* FloatName, float Value)
{
    fprintf(File, "float %s = (float)%f;\n",
            FloatName,
            Value
            );
}

internal void
StartHeaderFile(FILE *File, char* FCapital)
{
    fprintf(File,
            "#ifndef %s\n"
            "#define %s\n",
            FCapital,
            FCapital);
}

internal void
SaveFontToHeaderFile(char* FontName, char* FileName, char* FullFilePath, Font* SaveFont)
{
    // Header file
    FILE *File = fopen(FullFilePath, "w");
    
    char f[20];
    FilenameSearchModify(FileName, f);
    char FCapital[20];
    FilenameCapitalize(FileName, FCapital);
    
    StartHeaderFile(File, FCapital);
    
    SaveMemoryToHeaderFile(File, Concat(FontName, "stbtt"),
                           (void*)&SaveFont->Info, sizeof(stbtt_fontinfo));
    SaveMemoryToHeaderFile(File, Concat(FontName, "stbttuserdata"), 
                           (void*)&SaveFont->Info.userdata, 100);
    SaveMemoryToHeaderFile(File, Concat(FontName, "stbttdata"), (void*)SaveFont->Info.data, 100);
    SaveIntToHeaderFile(File, Concat(FontName, "Size"), SaveFont->Size);
    SaveIntToHeaderFile(File, Concat(FontName, "Ascent"), (int)SaveFont->Ascent);
    SaveFloatToHeaderFile(File, Concat(FontName, "Scale"), SaveFont->Scale);
    
    int MemorySize = 0;
    
    
    fprintf(File,
            "const unsigned char %s[%d] = {",
            Concat(FontName, "FontChar"),
            (int)(sizeof(FontChar) * SaveFont->Size)
            );
    for (int i = 0; i < SaveFont->Size; i++)
    {
        MemorySize += ((SaveFont->Memory[i].Width * SaveFont->Memory[i].Height) * BITMAP_BYTES_PER_PIXEL);
        unsigned char* MemoryCursor = (unsigned char*)&SaveFont->Memory[i];
        for(int j = 0; j < sizeof(FontChar); j++)
        {
            fprintf(File,
                    "%d ,",
                    *MemoryCursor);
            *MemoryCursor++;
        }
    }
    fprintf(File,
            "};\n"
            "\n"
            );
    
    //SaveInsideFontMemoryToHeaderFile(File, FontName, SaveFont, sizeof(FontChar));
    //SaveInsideFontMemoryToHeaderFile(File, FontName, SaveFont, ((SaveFont->Memory[i].Width * SaveFont->Memory[i].Height) * BITMAP_BYTES_PER_PIXEL));
    
    fprintf(File,
            "const unsigned char %s[%d] = {",
            Concat(FontName, "FontCharMemory"),
            MemorySize
            );
    for (int i = 0; i < SaveFont->Size; i++)
    {
        unsigned char* MemoryCursor = (unsigned char*)SaveFont->Memory[i].Memory;
        for(int j = 0; j < ((SaveFont->Memory[i].Width * SaveFont->Memory[i].Height) * BITMAP_BYTES_PER_PIXEL); j++)
        {
            fprintf(File,
                    "%d ,",
                    *MemoryCursor);
            *MemoryCursor++;
        }
    }
    fprintf(File,
            "};\n"
            "\n"
            );
    
    fprintf(File, 
            "#endif"
            );
    
    fclose(File);
}

internal void
LoadFontFromHeaderFile(Font* LoadFont, 
                       const unsigned char* stbtt,
                       const unsigned char* stbttuserdata,
                       const unsigned char* stbttdata,
                       int Size,
                       int Ascent,
                       float Scale,
                       const unsigned char* FontCharC,
                       const unsigned char* FontCharMemory)
{
    void* Mem = qalloc((void*)stbtt, 160);
    void* Memud = qalloc((void*)stbttuserdata, 100);
    void* Memd = qalloc((void*)stbttdata, 100);
    
    stbtt_fontinfo* LoadInfo = (stbtt_fontinfo*)Mem;
    LoadInfo->userdata = Memud;
    LoadInfo->data = (unsigned char*)Memd;
    
    LoadFont->Info = *LoadInfo;
    LoadFont->Size = Size;
    LoadFont->Ascent = (real32)Ascent;
    LoadFont->Scale = Scale;
    
    const unsigned char* CharCursor = FontCharC;
    const unsigned char* CharMemoryCursor = FontCharMemory;
    for (int i = 0; i < Size; i++)
    {
        MemoryCopy((void*)&LoadFont->Memory[i], (void*)CharCursor, sizeof(FontChar));
        CharCursor += sizeof(FontChar);
        
        if (i == 83)
            i = 83;
        
        int NextSpot = ((LoadFont->Memory[i].Width * LoadFont->Memory[i].Height) * BITMAP_BYTES_PER_PIXEL);
        LoadFont->Memory[i].Memory = qalloc((void*)CharMemoryCursor, NextSpot);
        CharMemoryCursor += NextSpot;
    }
}

/*
 Problems:
 z-sorting - buttons are in front of the text
off by one - texture is off by one in some places so a line is drawn.
fixed right now by turning off tiling. Can't tell if it still makes the font look bad.
*/
void
PrintOnScreen(Font* SrcFont, char* SrcText, v2 Coords, uint32 Color, v2 ScissorCoords, v2 ScissorDim)
{
    int StrLength = GetLength(SrcText);
    int BiggestY = 0;
    
    for (int i = 0; i < StrLength; i++){
        int SrcChar = SrcText[i];
        FontChar *NextChar = LoadFontChar(SrcFont, SrcChar, 0xFF000000);
        int Y = -1 *  NextChar->C_Y1;
        if(BiggestY < Y)
            BiggestY = Y;
    }
    
    real32 X = Coords.x;
    
    for (int i = 0; i < StrLength; i++)
    {
        int SrcChar = SrcText[i];
        
        FontChar *NextChar = LoadFontChar(SrcFont, SrcChar, Color);
        
        int Y = (int)Coords.y + NextChar->C_Y1 + BiggestY;
        
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&SrcFont->Info, SrcText[i], &ax, &lsb);
        
        v2 Scissor = v2(ScissorCoords.x, ScissorCoords.y + ScissorDim.y);
        
        Push(RenderGroup,
             v3(X + (lsb * SrcFont->Scale), (real32)Y, 100.0f), 
             v2((real32)NextChar->Tex.mWidth, (real32)NextChar->Tex.mHeight), 
             Scissor,
             ScissorDim,
             &NextChar->Tex, 0, BlendMode::gl_src_alpha);
        
        int kern = stbtt_GetCodepointKernAdvance(&SrcFont->Info, SrcText[i], SrcText[i + 1]);
        X += ((kern + ax) * SrcFont->Scale);
    }
}
inline void PrintOnScreen(Font* SrcFont, char* SrcText, v2 Coords, uint32 Color)
{
    PrintOnScreen(SrcFont, SrcText, Coords, Color, v2(0,0), v2(0,0));
}

internal loaded_bitmap
LoadGlyphBitmap(font *Font, u32 Codepoint, real32 Scale, uint32 Color)
{
    loaded_bitmap Result = {};
    if(Font->TTFFile.ContentsSize != 0)
    {
        int Width, Height, XOffset, YOffset;
        u8 *MonoBitmap = stbtt_GetCodepointBitmap(&Font->Info, 0, Scale, Codepoint, &Width, &Height, &XOffset, &YOffset);
        
        Result.Width = Width;
        Result.Height = Height;
        Result.Pitch = Result.Width * BITMAP_BYTES_PER_PIXEL;
        Result.Memory = malloc(Height*Result.Pitch);
        Result.Free = Result.Memory;
        
        u8 *Source = MonoBitmap;
        u8 *DestRow = (u8 *)Result.Memory;
        for(s32 Y = 0; Y < Height; ++Y)
        {
            u32 *Dest = (u32 *)DestRow;
            for(s32 X = 0; X < Width; ++X)
            {
                u8 Gray = *Source++;
                u32 Alpha = ((Gray << 24) |
                             (Gray << 16) |
                             (Gray <<  8) |
                             (Gray << 0));
                Color &= 0x00FFFFFF;
                Alpha &= 0xFF000000;
                Color += Alpha;
                *Dest++ = Color;
            }
            
            DestRow += Result.Pitch;
        }
        
        stbtt_FreeBitmap(MonoBitmap, 0);
    }
    
    return (Result);
}

internal font_scale*
LoadFontScale(font *Font, real32 PixelHeight)
{
    for (int i = 0; i < Font->FontScalesCached; i++) {
        if (PixelHeight == Font->FontScales[i].PixelHeight)
            return &Font->FontScales[i];
    }
    
    font_scale *FontScale = &Font->FontScales[Font->FontScalesCached];
    FontScale->Scale = stbtt_ScaleForPixelHeight(&Font->Info, PixelHeight);
    
    stbtt_GetFontVMetrics(&Font->Info, &FontScale->Ascent, &FontScale->Descent, &FontScale->LineGap);
    
    FontScale->ScaledAscent = roundf((real32)FontScale->Ascent * FontScale->Scale);
    FontScale->ScaledDescent = roundf((real32)FontScale->Descent * FontScale->Scale);
    FontScale->PixelHeight = PixelHeight;
    
    if (Font->FontScalesCached + 1 < 10)
        Font->FontScalesCached++;
    else
        Font->FontScalesCached = 0;
    
    return FontScale;
}

internal font*
LoadFont(const char *FileName)
{
    font *F = (font*)qalloc(sizeof(font));
    F->TTFFile = ReadEntireFile(FileName);
    stbtt_InitFont(&F->Info, (u8 *)F->TTFFile.Contents, stbtt_GetFontOffsetForIndex((u8 *)F->TTFFile.Contents, 0));
    return F;
}

internal font_char*
LoadFontChar(font* Font, int Codepoint, real32 Scale, uint32 Color)
{
    // If codepoint is already loaded
    for (int i = 0; i < Font->FontCharsCached; i++)
    {
        font_char *FontChar = &Font->FontChars[i];
        if (FontChar->Codepoint == Codepoint && 
            FontChar->Color == Color &&
            FontChar->Scale == Scale)
            return FontChar;
    }
    
    // If the codepoint has to be loaded
    font_char *FontChar = &Font->FontChars[Font->FontCharsCached];
    
    if (Font->FontCharsCached + 1 < FONTCACHEMAXSIZE)
        Font->FontCharsCached++;
    else
        Font->FontCharsCached = 0;
    
    FontChar->Codepoint = Codepoint;
    FontChar->Scale = Scale;
    FontChar->Color = Color;
    
    // how wide is this character
    stbtt_GetCodepointHMetrics(&Font->Info, FontChar->Codepoint, &FontChar->AX, &FontChar->LSB);
    
    // get bounding box for character (may be offset to account for chars that dip above or below the line
    stbtt_GetCodepointBitmapBox(&Font->Info, FontChar->Codepoint, FontChar->Scale, FontChar->Scale, 
                                &FontChar->C_X1, &FontChar->C_Y1, &FontChar->C_X2, &FontChar->C_Y2);
    
    // render character
    loaded_bitmap Temp = LoadGlyphBitmap(Font, Codepoint, Scale, Color);
    FontChar->Width = Temp.Width;
    FontChar->Height = Temp.Height;
    FontChar->Pitch = Temp.Pitch;
    FontChar->Memory = Temp.Memory;
    
#if QLIB_OPENGL
    Image SrcImage = {};
    SrcImage.x = FontChar->Width;
    SrcImage.y = FontChar->Height;
    SrcImage.n = FontChar->Pitch;
    SrcImage.data = (unsigned char*)FontChar->Memory;
    FontChar->Tex.Init(&SrcImage);
    stbi_image_free(SrcImage.data);
#endif
    
    return FontChar;
}

internal void
FontStringLoadChars(font_string *FontString)
{
    for (int i = 0; i < FontString->Length; i++) {
        font_char *Temp = LoadFontChar(FontString->Font, FontString->Text[i], FontString->Scale, FontString->Color);
        FontString->Memory[i] = Temp;
    }
}

internal v2
FontStringGetDim(font_string *FontString)
{
    if (FontString->OldScale != FontString->Scale || FontString->NewText) {
        FontStringLoadChars(FontString);
        FontString->OldScale = FontString->Scale;
        real32 StringWidth = 0;
        real32 StringHeight = 0;
        
        for (int i = 0; i < FontString->Length; i++)
        {
            font_char *NextChar = FontString->Memory[i];
            
            real32 Y = -1.0f *  NextChar->C_Y1;
            if(StringHeight < Y)
                StringHeight = Y;
            
            // add kerning
            int kern = stbtt_GetCodepointKernAdvance(&FontString->Font->Info, FontString->Text[i], FontString->Text[i + 1]);
            
            real32 Advance = ((kern + NextChar->AX) * FontString->Scale);
            FontString->Advances[i] = Advance;
            StringWidth += Advance;
        }
        
        FontString->Dim = v2(StringWidth, StringHeight);
    }
    return FontString->Dim;
}

internal void
FontStringSetText(font_string *FontString, const char *NewText)
{
    FontString->MaxLength = FONT_STRING_MAX_LENGTH;
    int NewTextLength = GetLength(NewText);
    if (NewTextLength <= FontString->MaxLength)
    {
        memset(FontString->Text, 0, FontString->MaxLength);
        memcpy(FontString->Text, NewText, NewTextLength);
        FontString->Length = NewTextLength;
    }
    else {
        PrintqDebug(S() + "FontStringSetText: NewText is too long\n");
    }
    FontString->NewText = true;
}

internal v2
FontStringResize(font_string *FontString, real32 PixelHeight)
{
    if (FontString->PixelHeight != PixelHeight) {
        FontString->PixelHeight = PixelHeight;
        
        font_scale *FontScale = LoadFontScale(FontString->Font, FontString->PixelHeight);
        FontString->Scale = FontScale->Scale;
        
        FontStringLoadChars(FontString);
        return FontStringGetDim(FontString);
    }
    return FontStringGetDim(FontString);
}

internal void
FontStringInit(font_string *FontString, font *Font, const char *Text, real32 PixelHeight, uint32 Color)
{
    FontString->Font = Font;
    FontString->Color = Color;
    
    FontStringSetText(FontString, Text);
    FontStringResize(FontString, PixelHeight);
}

internal void
FontStringInit(font_string *FontString)
{
    FontString->MaxLength = FONT_STRING_MAX_LENGTH;
    
    int TextLength = GetLength(FontString->Text);
    if (TextLength <= FontString->MaxLength) {
        FontString->Length = TextLength;
    }
    else {
        PrintqDebug(S() + "FontStringInit: Text is too long\n");
    }
    
    font_scale *FontScale = LoadFontScale(FontString->Font, FontString->PixelHeight);
    FontString->Scale = FontScale->Scale;
    
    FontStringLoadChars(FontString);
    FontStringGetDim(FontString);
}

internal void
FontStringPrint(font_string *FontString, v2 Coords, v2 ScissorCoords, v2 ScissorDim)
{
    real32 XCoord = Coords.x;
    
    for (int i = 0; i < FontString->Length; i++)
    {
        font_char *NextChar = FontString->Memory[i];
        
        real32 Y = Coords.y + NextChar->C_Y1 + FontString->Dim.y;
        real32 X = XCoord + (NextChar->LSB * FontString->Scale);
        
        v2 Scissor = v2(ScissorCoords.x, ScissorCoords.y + ScissorDim.y);
        
        Push(RenderGroup, v3(X, Y, 100.0f), 
             v2((real32)NextChar->Tex.mWidth, (real32)NextChar->Tex.mHeight), 
             Scissor, ScissorDim,
             &NextChar->Tex, 0, BlendMode::gl_src_alpha);
        
        XCoord += FontString->Advances[i];
    }
}
inline void FontStringPrint(font_string *FontString, v2 Coords)
{
    FontStringPrint(FontString, Coords, 0, 0);
}

#endif //TEXT_H
