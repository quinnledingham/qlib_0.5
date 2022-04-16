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
LoadGlyphBitmap(char *FileName, char *FontName, u32 Codepoint, float Scale, uint32 Color)
{
    loaded_bitmap Result = {};
    entire_file TTFFile = ReadEntireFile(FileName);
    if(TTFFile.ContentsSize != 0)
    {
        stbtt_fontinfo Font;
        stbtt_InitFont(&Font, (u8 *)TTFFile.Contents, stbtt_GetFontOffsetForIndex((u8 *)TTFFile.Contents, 0));
        
        int Width, Height, XOffset, YOffset;
        u8 *MonoBitmap = stbtt_GetCodepointBitmap(&Font, 0, stbtt_ScaleForPixelHeight(&Font, Scale),
                                                  Codepoint, &Width, &Height, &XOffset, &YOffset);
        
        Result.Width = Width;
        Result.Height = Height;
        Result.Pitch = Result.Width * BITMAP_BYTES_PER_PIXEL;
        Result.Memory = malloc(Height*Result.Pitch);
        Result.Free = Result.Memory;
        
        u8 *Source = MonoBitmap;
        u8 *DestRow = (u8 *)Result.Memory + (Height -1)*Result.Pitch;
        for(s32 Y = 0;
            Y < Height;
            ++Y)
        {
            u32 *Dest = (u32 *)DestRow;
            for(s32 X = 0;
                X < Width;
                ++X)
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
            
            DestRow -= Result.Pitch;
        }
        
        stbtt_FreeBitmap(MonoBitmap, 0);
        free(TTFFile.Contents);
    }
    
    return (Result);
}

internal v2
GetStringDimensions(Font* SrcFont, char* SrcText)
{
    int X = 0;
    int StrLength = StringLength(SrcText);
    int BiggestY = 0;
    
    for (int i = 0; i < StrLength; i++)
    {
        int SrcChar = SrcText[i];
        SrcFont->Memory[SrcChar].Advance = 0;
        
        int Y = -1 *  SrcFont->Memory[SrcChar].C_Y1;
        if(BiggestY < Y)
        {
            BiggestY = Y;
        }
        
        // advance x 
        SrcFont->Memory[SrcChar].Advance += (int)roundf(SrcFont->Memory[SrcChar].AX * SrcFont->Scale);
        
        // add kerning
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&SrcFont->Info, SrcText[i], SrcText[i + 1]);
        SrcFont->Memory[SrcChar].Advance += (int)roundf(kern * SrcFont->Scale);
        
        X += SrcFont->Memory[SrcChar].Advance;
    }
    
    int StringWidth = X;
    
    v2 Dimension = {};
    Dimension.x = (real32)StringWidth;
    Dimension.y = (real32)BiggestY;
    
    return(Dimension);
}

#define MAXSTRINGSIZE 1000

internal Font
LoadEntireFont(char* FileName, float ScaleIn)
{
    entire_file File = ReadEntireFile(FileName);
    
    stbtt_fontinfo Info;
    stbtt_InitFont(&Info, (u8 *)File.Contents, stbtt_GetFontOffsetForIndex((u8 *)File.Contents, 0));
    
    float Scale = stbtt_ScaleForPixelHeight(&Info, ScaleIn);
    
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&Info, &ascent, &descent, &lineGap);
    
    ascent = (int)roundf(ascent * Scale);
    descent = (int)roundf(descent * Scale);
    
    Font NewFont = {};
    NewFont.Info = Info;
    NewFont.Ascent = ascent;
    NewFont.Scale = Scale;
    
    
    for (int i = 0; i < NewFont.Size; i++)
    {
        
        // how wide is this character
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&Info, i, &ax, &lsb);
        
        // get bounding box for character (may be offset to account for chars that dip above or below the line
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&Info, i, Scale, Scale, &c_x1, &c_y1, &c_x2, &c_y2);
        
        // compute y (different characters have different heights)
        //int y = ascent + c_y1 + yin;
        
        // render character
        //int byteOffset = x + roundf(lsb * scale) + (y * b_w);
        loaded_bitmap Temp = LoadGlyphBitmap(FileName, "FauneRegular", i, ScaleIn, 0xFF000000);
        
        FontChar NewFontChar = {};
        NewFontChar.Width = Temp.Width;
        NewFontChar.Height = Temp.Height;
        NewFontChar.Pitch = Temp.Pitch;
        NewFontChar.Memory = Temp.Memory;
        NewFontChar.AX = ax;
        NewFontChar.C_X1 = c_x1;
        NewFontChar.C_Y1 = c_y1;
        NewFontChar.C_X2 = c_x2;
        NewFontChar.C_Y2 = c_y2;
        
        NewFont.Memory[i] = NewFontChar;
    }
    
    //free(File.Contents);
    return(NewFont);
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
            StringConcat(FontName, "FontChar"),
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
    
    SaveMemoryToHeaderFile(File, StringConcat(FontName, "stbtt"),
                           (void*)&SaveFont->Info, sizeof(stbtt_fontinfo));
    SaveMemoryToHeaderFile(File, StringConcat(FontName, "stbttuserdata"), 
                           (void*)&SaveFont->Info.userdata, 100);
    SaveMemoryToHeaderFile(File, StringConcat(FontName, "stbttdata"), (void*)SaveFont->Info.data, 100);
    SaveIntToHeaderFile(File, StringConcat(FontName, "Size"), SaveFont->Size);
    SaveIntToHeaderFile(File, StringConcat(FontName, "Ascent"), SaveFont->Ascent);
    SaveFloatToHeaderFile(File, StringConcat(FontName, "Scale"), SaveFont->Scale);
    
    int MemorySize = 0;
    
    
    fprintf(File,
            "const unsigned char %s[%d] = {",
            StringConcat(FontName, "FontChar"),
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
            StringConcat(FontName, "FontCharMemory"),
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