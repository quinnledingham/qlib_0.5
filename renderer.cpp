
#if QLIB_OPENGL

//
// render_shader
//

internal void
ShaderInit(render_shader *Shader, const char *VertexFileName, const char *FragmentFileName)
{
    Shader->Handle = glCreateProgram();
    
    Shader->Attributes.Init();
    Shader->Uniforms.Init();
    
    // Load
    entire_file VertFile = ReadEntireFile(VertexFileName);
    entire_file FragFile = ReadEntireFile(FragmentFileName);
    
    Assert(VertFile.Contents != 0);
    Assert(FragFile.Contents != 0);
    
    strinq V_Source = NewStrinq(&VertFile);
    strinq F_Source = NewStrinq(&FragFile);
    
    // Compile Vertex Shader
    u32 V_Shader = glCreateShader(GL_VERTEX_SHADER);
    const char* v = V_Source.Data; // v_source needs to be 0 terminated
    glShaderSource(V_Shader, 1, &v, NULL);
    glCompileShader(V_Shader);
    
    int GotVertexShader = 0;
    glGetShaderiv(V_Shader, GL_COMPILE_STATUS, &GotVertexShader);
    if (GotVertexShader)
    {
        unsigned Vert = V_Shader;
        
        // Compile Fragment Shader
        u32 F_Shader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* f = F_Source.Data;  // f_source needs to be 0 terminated
        glShaderSource(F_Shader, 1, &f, NULL);
        glCompileShader(F_Shader);
        
        int GotFragmentShader = 0;
        glGetShaderiv(F_Shader, GL_COMPILE_STATUS, &GotFragmentShader);
        if (GotFragmentShader)
        {
            unsigned int Frag = F_Shader;
            
            // Link
            glAttachShader(Shader->Handle, Vert);
            glAttachShader(Shader->Handle, Frag);
            glLinkProgram(Shader->Handle);
            
            int GotProgram = 0;
            glGetProgramiv(Shader->Handle, GL_LINK_STATUS, &GotProgram);
            if (GotProgram)
            {
                glDeleteShader(Vert);
                glDeleteShader(Frag);
                
                // Populate Attributes
                {
                    int count = -1;
                    int length;
                    char name[128];
                    int size;
                    GLenum type;
                    
                    glUseProgram(Shader->Handle);
                    glGetProgramiv(Shader->Handle, GL_ACTIVE_ATTRIBUTES, &count);
                    
                    for (int i = 0; i < count; ++i)
                    {
                        memset(name, 0, sizeof(char) * 128);
                        glGetActiveAttrib(Shader->Handle, (GLuint)i, 128, &length, &size, &type, name);
                        int attrib = glGetAttribLocation(Shader->Handle, name);
                        if (attrib >= 0)
                        {
                            Shader->Attributes[name] = attrib;
                        }
                    }
                    
                    glUseProgram(0);
                }
                
                // Populate Uniforms
                {
                    int count = -1;
                    int length;
                    char name[128];
                    int size;
                    GLenum type;
                    char testName[256];
                    
                    glUseProgram(Shader->Handle);
                    glGetProgramiv(Shader->Handle, GL_ACTIVE_UNIFORMS, &count);
                    
                    for (int i = 0; i < count; ++i)
                    {
                        memset(name, 0, sizeof(char) * 128);
                        glGetActiveUniform(Shader->Handle, (GLuint)i, 128, &length, &size, &type, name);
                        int uniform = glGetUniformLocation(Shader->Handle, name);
                        if (uniform >= 0)
                        {
                            // Is uniform valid?
                            strinq uniformName = NewStrinq(name);
                            // if name contains [, uniform is array
                            int found = StrinqFind(uniformName, '[');
                            
                            if (found != -1)
                            {
                                StrinqErase(uniformName, found);
                                unsigned int uniformIndex = 0;
                                while (true)
                                {
                                    memset(testName, 0, sizeof(char) * 256);
                                    
                                    strinq n = S() + uniformName + "[" + uniformIndex++ + "]";
                                    CopyBuffer(testName, n.Data, n.Length);
                                    
                                    int uniformLocation = glGetUniformLocation(Shader->Handle, testName);
                                    if (uniformLocation < 0)
                                    {
                                        break;
                                    }
                                    
                                    Shader->Uniforms[testName] = uniformLocation;
                                }
                            }
                            
                            Shader->Uniforms[uniformName] = uniform;
                            DestroyStrinq(&uniformName);
                        }
                    }
                    
                    glUseProgram(0);
                }
                
            }
            else // !GotProgram
            {
                char infoLog[512];
                glGetProgramInfoLog(Shader->Handle, 512, NULL, infoLog);
                PrintqDebug("ERROR: Shader linking failed.\n");
                PrintqDebug(S() + "\t" + infoLog + "\n");
                glDeleteShader(Vert);
                glDeleteShader(Frag);
            }
        }
        else // !GotFragmentShader
        {
            char infoLog[512];
            glGetShaderInfoLog(F_Shader, 512, NULL, infoLog);
            PrintqDebug("Fragment compilation failed.\n");
            PrintqDebug(S() + "\t" + infoLog + "\n");
            glDeleteShader(F_Shader);
            return;
        }
    }
    else // !GotVertexShader
    {
        char infoLog[512];
        glGetShaderInfoLog(V_Shader, 512, NULL, infoLog);
        PrintqDebug("Vertex compilation failed.\n");
        PrintqDebug(S() + "\t" + infoLog + "\n");
        
        glDeleteShader(V_Shader);
    }
    
    DestroyStrinq(&V_Source);
    DestroyStrinq(&F_Source);
    DestroyEntireFile(VertFile);
    DestroyEntireFile(FragFile);
}

inline void ShaderDestroy(render_shader *Shader) { glDeleteProgram(Shader->Handle); }
inline void ShaderBind(render_shader *Shader) { glUseProgram(Shader->Handle); }
inline void ShaderUnBind(render_shader *Shader) { glUseProgram(0); }
inline u32 ShaderGetAttribute(render_shader *Shader, const char *Name) { return Shader->Attributes.MapFind(Name); }
inline u32 ShaderGetUniform(render_shader *Shader, const char *Name) { return Shader->Uniforms.MapFind(Name); }

//
// render_attribute
//

inline void
AttributeInit(render_attribute *Attribute)
{
    glGenBuffers(1, &Attribute->Handle);
    Attribute->Count = 0;
}
inline void AttributeDestroy(render_attribute *Attribute) { glDeleteBuffers(1, &Attribute->Handle); }

internal void
AttributeSet(render_attribute *Attribute, void *Data, u32 TypeSize, u32 ArrayLength)
{
    Attribute->Count = ArrayLength;
    glBindBuffer(GL_ARRAY_BUFFER, Attribute->Handle);
    glBufferData(GL_ARRAY_BUFFER, TypeSize * Attribute->Count, Data, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

inline void AttribSetPointerint(u32 s) { glVertexAttribIPointer(s, 1, GL_INT, 0, (void*)0); }
inline void AttribSetPointeriv4(u32 s) { glVertexAttribIPointer(s, 4, GL_INT, 0, (void*)0); }
inline void AttribSetPointerreal32(u32 s) { glVertexAttribPointer(s, 1, GL_FLOAT, GL_FALSE, 0, 0); }
inline void AttribSetPointerv2(u32 s) { glVertexAttribPointer(s, 2, GL_FLOAT, GL_FALSE, 0, 0); }
inline void AttribSetPointerv3(u32 s) { glVertexAttribPointer(s, 3, GL_FLOAT, GL_FALSE, 0, 0); }
inline void AttribSetPointerv4(u32 s) { glVertexAttribPointer(s, 4, GL_FLOAT, GL_FALSE, 0, 0); }
inline void AttribSetPointerquat(u32 s) { glVertexAttribPointer(s, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); }
#define AttributeSetPointer(s, t) (AttribSetPointer##t(s))

inline void AttributeBindBuffer(render_attribute *Attribute, u32 Slot)
{ 
    glBindBuffer(GL_ARRAY_BUFFER, Attribute->Handle);
    glEnableVertexAttribArray(Slot);
}
#define AttributeBindTo(t, a, s) \
{\
AttributeBindBuffer(a, s);\
AttributeSetPointer(s, t);\
glBindBuffer(GL_ARRAY_BUFFER, 0);\
}

inline void AttributeUnBindFrom(render_attribute *Attribute, u32 Slot)
{
    glBindBuffer(GL_ARRAY_BUFFER, Attribute->Handle);
    glDisableVertexAttribArray(Slot);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//
// render_uniform
//

#define UNIFORM_IMPL(gl_func, tType, dType) void UniformSet##tType(unsigned int Slot, tType* Data, unsigned int Length) \
{ gl_func(Slot, (GLsizei)Length, (dType*)&Data[0]); }

UNIFORM_IMPL(glUniform1iv, int, int)
UNIFORM_IMPL(glUniform4iv, iv4, int)
UNIFORM_IMPL(glUniform2iv, iv2, int)
UNIFORM_IMPL(glUniform1fv, float, float)
UNIFORM_IMPL(glUniform2fv, v2, float)
UNIFORM_IMPL(glUniform3fv, v3, float)
UNIFORM_IMPL(glUniform4fv, v4, float)
UNIFORM_IMPL(glUniform4fv, quat, float)
inline void UniformSetmat4(unsigned int Slot, mat4* Data, unsigned int Length) 
{ 
    glUniformMatrix4fv(Slot, (GLsizei)Length, false, (float*) &Data[0]); 
}
#define UniformSet(t, s, v) (UniformSet##t(s, &v, 1))

//
// render_index_buffer
//

inline void
IndexBufferInit(render_index_buffer *IndexBuffer)
{
    glGenBuffers(1, &IndexBuffer->Handle);
    IndexBuffer->Count = 0;
}
inline void IndexBufferDestroy(render_index_buffer *IndexBuffer) { glDeleteBuffers(1, &IndexBuffer->Handle); }

inline void
IndexBufferSet(render_index_buffer *IndexBuffer, u32 *InputArray, u32 ArrayLength)
{
    IndexBuffer->Count = ArrayLength;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer->Handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * IndexBuffer->Count, InputArray, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
inline void IndexBufferSet(render_index_buffer *IndexBuffer, DynArray<u32> &Input) { IndexBufferSet(IndexBuffer, (u32*)Input.GetData(), Input.GetSize()); }

//
// render_texture
//

internal void
TextureInitialization(void **Handle, loaded_bitmap *Source)
{
    Assert(Source->Memory != 0);
    
    glGenTextures(1, (u32*)Handle);
    glBindTexture(GL_TEXTURE_2D, (GLuint)U32FromPointer(*Handle));
    
    if (Source->Channels == 3) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Source->Width, Source->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Source->Memory);
    }
    else if (Source->Channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Source->Width, Source->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Source->Memory);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Tile
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

//inline void TextureInit(render_texture *Texture, loaded_bitmap *LoadedBitmap) { TextureInitialization(&Texture->Handle, LoadedBitmap); }
inline void TextureInit(loaded_bitmap *Bitmap) { TextureInitialization(&Bitmap->TextureHandle, Bitmap); }
inline void TextureDestroy(loaded_bitmap *Bitmap) { glDeleteTextures(1, (u32*)&Bitmap->TextureHandle); }

internal void
TextureSet(unsigned int Handle, u32 UniformIndex, u32 TextureIndex)
{
    glActiveTexture(GL_TEXTURE0 + TextureIndex);
    glBindTexture(GL_TEXTURE_2D, Handle);
    glUniform1i(UniformIndex, TextureIndex);
}
/*
inline void TextureSet(render_texture *Texture, u32 UniformIndex, u32 TextureIndex)
{
    TextureSet(U32FromPointer(Texture->Handle), UniformIndex, TextureIndex);
}
*/
internal void
TextureUnSet(u32 TextureIndex)
{
    glActiveTexture(GL_TEXTURE0 + TextureIndex);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
}
//inline void TextureUnSet(render_texture *Texture, u32 TextureIndex) { TextureUnSet(TextureIndex); }

/*
internal void
TextureResize(render_texture *Texture, assets *Assets, iv2 Dim)
{
    ResizeBitmap(GetResizableBitmap(Assets, Texture->BitmapID), Dim);
    TextureDestroy(Texture);
    TextureInit(Texture, Assets);
}
*/

//
// Drawing Methods
//

static GLenum DrawModeToGLEnum(render_draw_mode Input) {
	if (Input == render_draw_mode::points)
		return  GL_POINTS;
	else if (Input == render_draw_mode::line_strip)
		return GL_LINE_STRIP;
	else if (Input == render_draw_mode::line_loop)
		return  GL_LINE_LOOP;
	else if (Input == render_draw_mode::lines)
		return  GL_LINES;
	else if (Input == render_draw_mode::triangles)
		return  GL_TRIANGLES;
	else if (Input == render_draw_mode::triangle_strip)
		return  GL_TRIANGLE_STRIP;
	else if (Input == render_draw_mode::triangle_fan)
		return   GL_TRIANGLE_FAN;
    
    PrintqDebug("DrawModeToGLEnum unreachable code hit\n");
	return 0;
}

inline void glDraw(u32 VertexCount, render_draw_mode DrawMode) { glDrawArrays(DrawModeToGLEnum(DrawMode), 0, VertexCount); }
inline void glDraw(render_index_buffer& IndexBuffer,  render_draw_mode DrawMode)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer.Handle);
    glDrawElements(DrawModeToGLEnum(DrawMode), IndexBuffer.Count, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
inline void glDrawInstanced(u32 VertexCount, render_draw_mode DrawMode, u32 NumInstances) { glDrawArraysInstanced(DrawModeToGLEnum(DrawMode), 0, VertexCount, NumInstances); }
inline void glDrawInstanced(render_index_buffer& IndexBuffer,  render_draw_mode DrawMode, u32 InstanceCount)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer.Handle);
    glDrawElementsInstanced(DrawModeToGLEnum(DrawMode), IndexBuffer.Count, GL_UNSIGNED_INT, 0, InstanceCount);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Renderering Elements

struct open_gl_rect
{
    render_index_buffer IndexBuffer;
    render_attribute VertexPositions; // v3
    render_attribute VertexNormals; // v3
    render_attribute VertexTexCoords; // v2
};

internal void
OpenGLRectCenter(open_gl_rect *OpenGLRect)
{
    DynArray<v3> position = {};
    position.push_back(v3(-0.5f, -0.5f, 0.0f)); // 0
    position.push_back(v3(0.5f, -0.5f, 0.0f)); // 1
    position.push_back(v3(0.5f, 0.5f, 0.0f)); // 2
    position.push_back(v3(-0.5f, 0.5f, 0.0f)); // 3
    real32 position2[] = 
    {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5, 0.0f,
        0.5f, 0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f
    };
    AttributeSet(&OpenGLRect->VertexPositions, &position2, sizeof(v3), position.GetSize());
}

internal void
OpenGLRectTopRight(open_gl_rect *OpenGLRect)
{
    DynArray<v3> position = {};
    position.push_back(v3(0.0f, -1.0f, 0.0f));
    position.push_back(v3(1.0f, -1.0f, 0.0f));
    position.push_back(v3(1.0f, 0.0f, 0.0f));
    position.push_back(v3(0.0f, 0.0f, 0.0f));
    AttributeSet(&OpenGLRect->VertexPositions, position.GetData(), sizeof(v3), position.GetSize());
}

internal void
OpenGLRectInit(open_gl_rect *OpenGLRect)
{
    AttributeInit(&OpenGLRect->VertexPositions);
    OpenGLRectCenter(OpenGLRect);
    //OpenGLRectTopRight(OpenGLRect);
    
    AttributeInit(&OpenGLRect->VertexNormals);
    DynArray<v3> normals = {};
    normals.Resize(4, v3(0, 0, 1));
    AttributeSet(&OpenGLRect->VertexNormals, normals.GetData(), sizeof(v3), normals.GetSize());
    
    AttributeInit(&OpenGLRect->VertexTexCoords);
    DynArray<v2> uvs = {};
    uvs.push_back(v2(0, 1));
    uvs.push_back(v2(1, 1));
    uvs.push_back(v2(1, 0));
    uvs.push_back(v2(0, 0));
    AttributeSet(&OpenGLRect->VertexTexCoords, uvs.GetData(), sizeof(v2), uvs.GetSize());
    
    // Here is where the direction of the triangle is defined.
    // Counter clockwise definition of vertexes makes it face forward.
    IndexBufferInit(&OpenGLRect->IndexBuffer);
    DynArray<u32> indices = {};
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);
    IndexBufferSet(&OpenGLRect->IndexBuffer, indices);
}

global_variable render_shader BasicShader;
global_variable render_shader StaticShader;

global_variable open_gl_rect GlobalOpenGLRect;

#endif

// Function for Software Rendering
// Paints the screen white
inline void ClearScreen()
{
    platform_offscreen_buffer *Buffer = &GlobalBackbuffer;
    memset(Buffer->Memory, 0xFF, (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel);
}

void BeginRenderer(camera *C)
{
    if (C->WindowDim != C->PlatformDim) {
        C->WindowDim = C->PlatformDim;
        
#if QLIB_OPENGL
        glViewport(0, 0, C->WindowDim.x, C->WindowDim.y);
#endif
        
        if (!C->Mode3D) {
            C->Projection = Ortho(-(real32)C->WindowDim.x/2, (real32)C->WindowDim.x/2, 
                                  -(real32)C->WindowDim.y/2, (real32)C->WindowDim.y/2, 
                                  0.1f, 1000.0f);
        }
        else if (C->Mode3D) {
            C->inAspectRatio = (real32)C->WindowDim.x / (real32)C->WindowDim.y;
            C->Projection = Perspective(C->FOV, C->inAspectRatio, 0.01f, 1000.0f);
        }
    }
    
#if QLIB_OPENGL
    if (!C->OpenGLInitialized) {
        ShaderInit(&BasicShader, "shaders/basic.vert", "shaders/basic.frag");
        ShaderInit(&StaticShader, "shaders/static.vert", "shaders/lit.frag");
        
        OpenGLRectInit(&GlobalOpenGLRect);
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_ALWAYS); 
        
        glPointSize(5.0f);
        glBindVertexArray(gVertexArrayObject);
        
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        C->OpenGLInitialized = true;
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#else
    ClearScreen();
#endif
    
    C->View = LookAt(C->Position, C->Target, C->Up);
}

#define NOFILL 0
#define FILL 1

#if QLIB_OPENGL

void DrawRect(render_camera *Camera, render_piece *Piece, render_shader *Shader)
{
    mat4 Model = TransformToMat4(Transform(v3(Piece->Coords.x, Piece->Coords.y, Piece->Coords.z), 
                                           AngleAxis(Piece->Rotation * DEG2RAD, v3(0, 0, 1)), 
                                           v3(Piece->Dim.x, Piece->Dim.y, 1)));
    
    UniformSet(mat4, ShaderGetUniform(Shader, "model"), Model);
    UniformSet(mat4, ShaderGetUniform(Shader, "view"), Camera->View);
    UniformSet(mat4, ShaderGetUniform(Shader, "projection"), Camera->Projection);
    
    AttributeBindTo(v3, &GlobalOpenGLRect.VertexPositions, ShaderGetAttribute(Shader, "position"));
    
    glDraw(GlobalOpenGLRect.IndexBuffer, render_draw_mode::triangles);
    
    AttributeUnBindFrom(&GlobalOpenGLRect.VertexPositions, ShaderGetAttribute(Shader, "position"));
    
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        PrintqDebug(S() + "Opengl Error: " + (int)err + "\n");
    }
}

void
DrawRect(render_camera *Camera, render_piece *Piece)
{
    ShaderBind(&BasicShader);
    
    v4 c = u32toV4(Piece->Color);
    UniformSet(v4, ShaderGetUniform(&BasicShader, "my_color"), c);
    
    DrawRect(Camera, Piece, &BasicShader);
    
    ShaderUnBind(&BasicShader);
}

void
DrawRect(render_camera *Camera, render_piece *Piece, loaded_bitmap *Bitmap)
{
    // Only works when coord system == top_right
    if (Piece->ScissorDim.x > 0 && Piece->ScissorDim.y > 0) {
        glScissor((GLsizei)(Piece->ScissorCoords.x), 
                  (GLsizei)(Camera->WindowDim.y - Piece->ScissorCoords.y), 
                  (GLint)Piece->ScissorDim.x, (GLint)Piece->ScissorDim.y);
        glEnable(GL_SCISSOR_TEST);
    }
    
    ShaderBind(&StaticShader);
    
    v3 Light = v3(0, 0, 1);
    UniformSet(v3, ShaderGetUniform(&StaticShader, "light"), Light);
    
    AttributeBindTo(v3, &GlobalOpenGLRect.VertexNormals, ShaderGetAttribute(&StaticShader, "normal"));
    AttributeBindTo(v2, &GlobalOpenGLRect.VertexTexCoords, ShaderGetAttribute(&StaticShader, "texCoord"));
    
    TextureSet(U32FromPointer(Bitmap->TextureHandle), ShaderGetUniform(&StaticShader, "tex0"), 0);
    
    DrawRect(Camera, Piece, &StaticShader);
    
    TextureUnSet(0);
    
    AttributeUnBindFrom(&GlobalOpenGLRect.VertexNormals, ShaderGetAttribute(&StaticShader, "normal"));
    AttributeUnBindFrom(&GlobalOpenGLRect.VertexTexCoords, ShaderGetAttribute(&StaticShader, "texCoord"));
    
    ShaderUnBind(&StaticShader);
    
    if (Piece->ScissorDim.x > 0 && Piece->ScissorDim.y > 0)
        glDisable(GL_SCISSOR_TEST);
    
}

inline void DrawRect(camera *Camera, render_piece *Piece, assets *Assets)
{
    loaded_bitmap *Bitmap = GetBitmap(Assets, Piece->BitmapID);
    DrawRect(Camera, Piece, Bitmap);
}

#else // !QLIB_OPENGL

//
// Software Rendering
//

inline void TextureInit(loaded_bitmap *Bitmap) {}

void
DrawRect(int x, int y, int width, int height, uint32 color)
{
    platform_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
    uint32 Color = color;
    
    x = x + (Buffer->Width / 2);
    y = y + (Buffer->Height / 2);
    
    for(int X = x; X < (x + width); ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory + X*Buffer->BytesPerPixel + y*Buffer->Pitch);
        
        for(int Y = y; Y < (y + height); ++Y)
        {
            // Check if the pixel exists
            if((Pixel >= Buffer->Memory) && ((Pixel + 4) <= EndOfBuffer))
            {
                
                *(uint32 *)Pixel = Color;
                
                /*
                else if (fill == NOFILL)
                {
                    // Only draw border
                    if ((X == x) ||
                        (Y == y) ||
                        (X == (x + width) - 1) ||
                        (Y == (y + height) - 1))
                    {
                        *(uint32 *)Pixel = Color;
                    }
                }
*/
            }
            
            Pixel += Buffer->Pitch;
        }
    }
}
void DrawRect(v3 Coords, v2 Size, uint32 Color, real32 Rotation)
{
    
}

void DrawRect(v3 Coords, v2 Size, v2 ScissorCoords, v2 ScissorDim, loaded_bitmap *Bitmap, real32 Rotation, blend_mode BlendMode)
{
    platform_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    ResizeBitmap(Bitmap, Size);
    
    Image re = {};
    re.data = (unsigned char*)Bitmap->Memory;
    re.x = Bitmap->Width;
    re.y = Bitmap->Height;
    re.n = Bitmap->Channels;
    //RenderImage(Buffer, &re);
    
    Coords.x += Buffer->Width / 2;
    Coords.y += Buffer->Height / 2;
    
    uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
    
    for(int X = (int)Coords.x; X < (int)(Coords.x + Size.Width); ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory + X * Buffer->BytesPerPixel + (int)Coords.y * Buffer->Pitch);
        uint8 *Color = ((uint8 *)re.data + (X - (int)Coords.x) * re.n);
        
        for(int Y = (int)Coords.y; Y < (Coords.y + Size.Height); ++Y)
        {
            // Check if the pixel exists
            if((Pixel >= Buffer->Memory) && ((Pixel + 4) <= EndOfBuffer))
            {
                uint32 c = *Color;
                
                int r = *Color++;
                int g = *Color++;
                int b = *Color;
                Color--;
                Color--;
                
                c = createRGB(r, g, b);
                *(uint32 *)Pixel =c;
                Color += (re.n * re.x);
            }
            Pixel += Buffer->Pitch;
        }
    }
}
inline void DrawRect(v3 Coords, v2 Size, loaded_bitmap *Bitmap, real32 Rotation, render_blend_mode BlendMode)
{
    DrawRect(Coords, Size, v2(0, 0), v2(0, 0), Bitmap, Rotation, BlendMode);
}



internal void
ChangeBitmapColor(loaded_bitmap Bitmap, uint32 Color)
{
    u8 *DestRow = (u8*)Bitmap.Memory + (Bitmap.Height -1)*Bitmap.Pitch;
    for(s32 Y = 0; Y < Bitmap.Height; ++Y)
    {
        u32 *Dest = (u32 *)DestRow;
        for(s32 X = 0; X < Bitmap.Width; ++X)
        {
            u32 Gray = *Dest;
            Color &= 0x00FFFFFF;
            Gray &= 0xFF000000;
            Color += Gray;
            *Dest++ = Color;
        }
        
        DestRow -= Bitmap.Pitch;
    }
}

internal void
RenderBitmap(loaded_bitmap *Bitmap, real32 RealX, real32 RealY)
{
    platform_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    int32 MinX = RoundReal32ToInt32(RealX);
    int32 MinY = RoundReal32ToInt32(RealY);
    int32 MaxX = MinX + Bitmap->Width;
    int32 MaxY = MinY + Bitmap->Height;
    
    if(MinX < 0)
    {
        MinX = 0;
    }
    
    if(MinY < 0)
    {
        MinY = 0;
    }
    
    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }
    
    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
    
    uint32 *SourceRow = (uint32*)Bitmap->Memory;
    uint8 *DestRow = ((uint8*)Buffer->Memory + MinX*Buffer->BytesPerPixel + MinY*Buffer->Pitch);
    
    for(int Y = MinY; Y < MaxY; ++Y)
    {
        uint32 *Dest = (uint32*)DestRow;
        uint32 *Source = SourceRow;
        
        for(int X = MinX; X < MaxX; ++X)
        {
            real32 A = (real32)((*Source >> 24) & 0xFF) / 255.0f;
            real32 SR = (real32)((*Source >> 0) & 0xFF);
            real32 SG = (real32)((*Source >> 8) & 0xFF);
            real32 SB = (real32)((*Source >> 16) & 0xFF);
            
            real32 DR = (real32)((*Dest >> 16) & 0xFF);
            real32 DG = (real32)((*Dest >> 8) & 0xFF);
            real32 DB = (real32)((*Dest >> 0) & 0xFF);
            
            real32 R = (1.0f-A)*DR + A*SR;
            real32 G = (1.0f-A)*DG + A*SG;
            real32 B = (1.0f-A)*DB + A*SB;
            
            *Dest = (((uint32)(R + 0.5f) << 16) |
                     ((uint32)(G + 0.5f) << 8) |
                     ((uint32)(B + 0.5f) << 0));
            
            ++Dest;
            ++Source;
        }
        
        DestRow += Buffer->Pitch;
        SourceRow += Bitmap->Width;
    }
}
#endif

//
// track
//

//template track<real32, 1>;
//template track<v3, 3>;
//template track<quat, 4>;

// track helpers

inline real32 Interpolate(real32 a, real32 b, real32 t)
{
    return a + (b - a) * t;
}

inline v3 Interpolate(const v3& a, const v3 &b, real32 t)
{
    return Lerp(a, b, t);
}

inline quat Interpolate(const quat &a, const quat &b, real32 t)
{
    quat Result = Mix(a, b, t);
    if (Dot(a, b) < 0) // Neighborhood
    {
        Result = Mix(a, -b, t);
    }
    return Normalized(Result); // NLerp, not slerp
}

inline real32 AdjustHermiteResult(real32 f) { return f; }
inline v3 AdjustHermiteResult(const v3 &v) { return v; }
inline quat AdjustHermiteResult(const quat &q)
{
    return Normalized(q);
}

inline void Neighborhood(const real32 &a, real32 &b) {}
inline void Neighborhood(const v3 &a, v3 &b) {}
inline void Neighborhood(const quat &a, quat &b) 
{
    if (Dot(a, b) < 0)
        b = -b;
}

// end of track helpers

template<typename T, int N> void
TrackInit(track<T, N> *Track)
{
    Track->Interpolation = interpolation::linear;
}

template<typename T, int N> real32
TrackGetStartTime(track<T, N> *Track)
{
    return Track->Frames[0].Time;
}

template<typename T, int N> real32
TrackGetEndTime(track<T, N> *Track)
{
    return Track->Frames[Track->Frames.GetSize() - 1].Time;
}
/*
template<typename T, int N> T
TrackSample(track<T, N> *Track, real32 Time, bool Looping)
{
    if (Track->Interpolation == interpolation::constant)
        return SampleConstant(Time, Looping);
    else if (Track->Interpolation == interpolation::constant)
        return SampleLinear(Time, Looping);
    else if (Track->Interpolation == interpolation::cubic)
        return SampleCubic(Time, Looping);
}
*/
template<typename T, int N> frame<N>&
track<T, N>::operator[] (unsigned int Index)
{
    return Frames[Index];
}

template<typename T, int N> void
TrackResize(track<T, N> *Track, unsigned int Size)
{
    Track->Frames.Resize(Size);
}

template<typename T, int N> unsigned int
TrackGetSize(track<T, N> *Track)
{
    return Track->Frames.GetSize();
}

//
// FPS Counter
//

internal void
DrawFPS(real32 Milliseconds, v2 ScreenDim, font *Font)
{
    qlibCoordSystem(QLIB_TOP_RIGHT);
    real32 fps = 0;
    if (Milliseconds != 0) {
        fps = (1 / Milliseconds) * 1000;
        strinq FPS = S() + (int)fps;
        
        font_string FontString = {};
        FontStringInit(&FontString, Font, FPS.Data, 60, 0xFFFFFFFF);
        v2 SDim = FontStringGetDim(&FontString);
        FontStringPrint(&FontString, v2(ScreenDim.x- SDim.x - 10.0f, 10.0f));
        
        Push(v3(ScreenDim.x - SDim.x - 15.0f, 5.0f, 99.0f), 
             v2(SDim.x + 10.0f, SDim.y + 10.0f),
             0xFF000000);
    }
    qlibCoordSystem(QLIB_CENTER);
}