
/*
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
*/

internal void
ChangeGlyphBitmapColor(loaded_bitmap Bitmap, uint32 Color)
{
    u8 *C = (u8*)&Color;
    int R = *C++;
    int G = *C++;
    int B = *C++;
    int A = *C++;
    unsigned long NewColor = createRGBA(R, G, B, A);
    
    u8 *DestRow = (u8 *)Bitmap.Memory;
    for(s32 Y = 0; Y < Bitmap.Height; ++Y)
    {
        u32 *Dest = (u32 *)DestRow;
        for(s32 X = 0; X < Bitmap.Width; ++X)
        {
            u32 Gray = *Dest;
            NewColor &= 0x00FFFFFF;
            Gray &= 0xFF000000;
            NewColor += Gray;
            *Dest++ = NewColor;
        }
        DestRow += Bitmap.Pitch;
    }
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
        Result.Channels = BITMAP_BYTES_PER_PIXEL;
        Result.Pitch = Result.Width * Result.Channels;
        Result.Memory = malloc(Height*Result.Pitch);
        
        u8 *Source = MonoBitmap;
        u8 *DestRow = (u8 *)Result.Memory;
        for(s32 Y = 0; Y < Height; ++Y)
        {
            u32 *Dest = (u32 *)DestRow;
            for(s32 X = 0; X < Width; ++X)
            {
                /*
                u8 Gray = *Source++;
                u32 Alpha = ((Gray << 24) | (Gray << 16) | (Gray <<  8) | (Gray << 0));
                Color &= 0x00FFFFFF;
                Alpha &= 0xFF000000;
                Color += Alpha;
                *Dest++ = Color;
                */
                u8 Alpha = *Source++;
                *Dest++ = ((Alpha << 24) |
                           (Alpha << 16) |
                           (Alpha <<  8) |
                           (Alpha <<  0));
            }
            
            DestRow += Result.Pitch;
        }
        
        stbtt_FreeBitmap(MonoBitmap, 0);
    }
    
    ChangeGlyphBitmapColor(Result, Color);
    
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
    //DestroyEntireFile();
    return F;
}

internal font
LoadFont2(const char *FileName)
{
    font F = {};
    F.TTFFile = ReadEntireFile(FileName);
    stbtt_InitFont(&F.Info, (u8 *)F.TTFFile.Contents, stbtt_GetFontOffsetForIndex((u8 *)F.TTFFile.Contents, 0));
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
    FontChar->Bitmap = LoadGlyphBitmap(Font, Codepoint, Scale, Color);
    
#if QLIB_OPENGL
    TextureInit(&FontChar->Bitmap);
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
        
        Push(v3(X, Y, 100.0f), v2((real32)NextChar->Bitmap.Width, (real32)NextChar->Bitmap.Height), 
             Scissor, ScissorDim, &NextChar->Bitmap, 0, blend_mode::gl_src_alpha);
        
        XCoord += FontString->Advances[i];
    }
}
inline void FontStringPrint(font_string *FontString, v2 Coords)
{
    FontStringPrint(FontString, Coords, 0, 0);
}
