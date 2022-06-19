#include "text.h"
#include "memorymanager.h"
#include <malloc.h>

internal void
FilenameSearchModify(char* filename, char* result)
{
    int j = 0;
    
    char* cursor = filename;
    while (*cursor != 0)
    {
        if (*cursor == '.')
        {
            result[j] = '_';
        }
        else
        {
            result[j] = *cursor;
        }
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
    while (*cursor != 0)
    {
        if (*cursor == '_' || *cursor == '.')
        {
            result[j] = '_';
        }
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

#define MAXSTRINGSIZE 1000

internal void
LoadFont(Font *F, char* FileName, real32 ScaleIn)
{
    entire_file File = ReadEntireFile(FileName);
    
    stbtt_fontinfo Info;
    stbtt_InitFont(&Info, (u8 *)File.Contents, stbtt_GetFontOffsetForIndex((u8 *)File.Contents, 0));
    
    float Scale = stbtt_ScaleForPixelHeight(&Info, ScaleIn);
    
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&Info, &ascent, &descent, &lineGap);
    
    ascent = (int)roundf(ascent * Scale);
    descent = (int)roundf(descent * Scale);
    
    F->Info = Info;
    F->Ascent = ascent;
    F->Scale = Scale;
    F->ScaleIn = ScaleIn;
    F->TTFFile = File;
}

internal Font*
LoadFont(const char* FileName, real32 ScaleIn)
{
    entire_file File = ReadEntireFile(FileName);
    
    stbtt_fontinfo Info;
    stbtt_InitFont(&Info, (u8 *)File.Contents, stbtt_GetFontOffsetForIndex((u8 *)File.Contents, 0));
    
    float Scale = stbtt_ScaleForPixelHeight(&Info, ScaleIn);
    
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&Info, &ascent, &descent, &lineGap);
    
    ascent = (int)roundf(ascent * Scale);
    descent = (int)roundf(descent * Scale);
    
    Font F = {};
    F.Info = Info;
    F.Ascent = ascent;
    F.Scale = Scale;
    F.ScaleIn = ScaleIn;
    F.TTFFile = File;
    return (Font*)qalloc((void*)&F, sizeof(Font));
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
    return &font->Memory[font->Size];
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
        
        NextChar->Advance = 0;
        
        int Y = -1 *  NextChar->C_Y1;
        if(BiggestY < Y)
            BiggestY = Y;
        
        // advance x 
        NextChar->Advance += (int)roundf(NextChar->AX * SrcFont->Scale);
        
        // add kerning
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&SrcFont->Info, SrcText[i], SrcText[i + 1]);
        NextChar->Advance += (int)roundf(kern * SrcFont->Scale);
        
        X += NextChar->Advance;
    }
    
    int StringWidth = (int)X;
    
    v2 Dimension = v2((real32)StringWidth, (real32)BiggestY);
    return(Dimension);
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
    SaveIntToHeaderFile(File, Concat(FontName, "Ascent"), SaveFont->Ascent);
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
    LoadFont->Ascent = Ascent;
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