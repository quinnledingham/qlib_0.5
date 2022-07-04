#ifndef TEXT_H
#pragma message ("renderer.h requires text.h")
#endif

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
TextureInitialization(render_texture *Texture, game_assets *Assets, loaded_bitmap *LoadedBitmap)
{
    resizable_bitmap *Asset = 0;
    loaded_bitmap *Bitmap = 0;
    if (LoadedBitmap == 0) {
        Asset = GetResizableBitmap(Assets, Texture->BitmapID);
        if (Asset->Resized == 0)
            Bitmap = Asset->Original;
        else
            Bitmap = Asset->Resized;
    }
    else if (LoadedBitmap != 0) {
        Bitmap = LoadedBitmap;
    }
    
    
    Assert(Bitmap->Width != 0 && Bitmap->Height != 0);
    
    glGenTextures(1, &Texture->Handle);
    glBindTexture(GL_TEXTURE_2D, Texture->Handle);
    
    if (Bitmap->Channels == 3) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Bitmap->Width, Bitmap->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Bitmap->Memory);
    }
    else if (Bitmap->Channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Bitmap->Width, Bitmap->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Bitmap->Memory);
    
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
inline void TextureInit(render_texture *Texture, game_assets *Assets) { TextureInitialization(Texture, Assets, 0); }
inline void TextureInit(render_texture *Texture, loaded_bitmap *Bitmap) { TextureInitialization(Texture, 0, Bitmap); }
inline void TextureInit(render_texture *Texture, game_assets *Assets, int ID) { Texture->BitmapID = ID; TextureInitialization(Texture, Assets, 0); }
inline void TextureDestroy(render_texture *Texture) { glDeleteTextures(1, &Texture->Handle); }

internal void
TextureSet(render_texture *Texture, u32 UniformIndex, u32 TextureIndex)
{
    glActiveTexture(GL_TEXTURE0 + TextureIndex);
    glBindTexture(GL_TEXTURE_2D, Texture->Handle);
    glUniform1i(UniformIndex, TextureIndex);
}

internal void
TextureUnSet(render_texture *Texture, u32 TextureIndex)
{
    glActiveTexture(GL_TEXTURE0 + TextureIndex);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
}

internal void
TextureResize(render_texture *Texture, game_assets *Assets, iv2 Dim)
{
    ResizeBitmap(GetResizableBitmap(Assets, Texture->BitmapID), Dim);
    TextureInit(Texture, Assets);
}

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

// Function for Software Rendering
// Paints the screen white
inline void ClearScreen()
{
    platform_offscreen_buffer *Buffer = &GlobalBackbuffer;
    memset(Buffer->Memory, 0xFF, (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel);
}

mat4 Projection;
mat4 View;
iv2 CameraViewDim;

void BeginOpenGL(iv2 WindowDim)
{
    glViewport(0, 0, WindowDim.x, WindowDim.y);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_ALWAYS); 
    glPointSize(5.0f);
    glBindVertexArray(gVertexArrayObject);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void BeginMode(camera C)
{
#if QLIB_OPENGL
    BeginOpenGL(C.WindowDim);
#else
    ClearScreen();
#endif
    View = LookAt(C.Position, C.Target, C.Up);
}

internal void
BeginMode2D(camera C)
{
    BeginMode(C);
    CameraViewDim = v2(C.WindowDim.x, C.WindowDim.y);
    real32 width =  C.WindowDim.x;
    real32 height = C.WindowDim.y;
    Projection = Ortho(-width/2, width/2, -height/2, height/2, 0.1f, 1000.0f);
}

internal void
BeginMode3D(camera C)
{
    BeginMode(C);
    CameraViewDim = v2(C.WindowDim.x, C.WindowDim.y);
    C.inAspectRatio = C.WindowDim.x / C.WindowDim.y;
    Projection = Perspective(C.FOV, C.inAspectRatio, 0.01f, 1000.0f);
}

#define NOFILL 0
#define FILL 1

#if QLIB_OPENGL

struct open_gl_rect
{
    render_shader Shader;
    render_index_buffer IndexBuffer;
    render_attribute VertexPositions; // v3
    render_attribute Color; // v4
    
    bool32 Initialized;
};

struct open_gl_texture
{
    render_shader Shader;
    render_attribute VertexPositions; // v3
    render_index_buffer IndexBuffer;
    render_attribute VertexNormals; // v3
    render_attribute VertexTexCoords; // v2
    
    bool32 Initialized;
};

global_variable open_gl_rect GlobalOpenGLRect;
global_variable open_gl_texture GlobalOpenGLTexture;

v4 u32toV4(uint32 input)
{
    //uint8 *C = (uint8*)malloc(sizeof(uint32));
    //memcpy(C, &input, sizeof(uint32));
    uint8 *C = (uint8*)&input;
    uint32 B = *C++;
    uint32 G = *C++;
    uint32 R = *C++;
    real32 A = *C++;
    return v4(real32(R), real32(G), real32(B), A);
}

v3 u32toV3(uint32 input)
{
    v4 r = u32toV4(input);
    return v3(r.x, r.y, r.z);
}

void
DrawRect(int x, int y, int width, int height, uint32 color)
{
    v3 Coords = v3((real32)x, (real32)y, 1);
    v2 Size = v2((real32)width, (real32)height);
    DrawRect(Coords, Size, color, 0);
}

void
DrawRect(v3 Coords, v2 Size, uint32 color, real32 Rotation)
{
    if (GlobalOpenGLRect.Initialized == 0)
    {
        ShaderInit(&GlobalOpenGLRect.Shader, "../shaders/basic.vert", "../shaders/basic.frag");
        
        AttributeInit(&GlobalOpenGLRect.VertexPositions);
        DynArray<v3> position = {};
        position.push_back(v3(-0.5, -0.5, 0));
        position.push_back(v3(-0.5, 0.5, 0));
        position.push_back(v3(0.5, -0.5, 0));
        position.push_back(v3(0.5, 0.5, 0));
        AttributeSet(&GlobalOpenGLRect.VertexPositions, position.GetData(), sizeof(v3), position.GetSize());
        
        IndexBufferInit(&GlobalOpenGLRect.IndexBuffer);
        DynArray<u32> indices = {};
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(2);
        indices.push_back(1);
        indices.push_back(3);
        IndexBufferSet(&GlobalOpenGLRect.IndexBuffer, indices);
        
        GlobalOpenGLRect.Initialized = 1;
    }
    
    // Change to standard coordinate system
    v2 NewCoords = {};
    NewCoords.x = (real32)(-Coords.x - (Size.x/2));
    NewCoords.y = (real32)(-Coords.y - (Size.y/2));
    
    mat4 Model = TransformToMat4(Transform(v3(NewCoords, Coords.z), AngleAxis(Rotation * DEG2RAD, v3(0, 0, 1)), v3(Size.x, Size.y, 1)));
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    ShaderBind(&GlobalOpenGLRect.Shader);
    
    UniformSet(mat4, ShaderGetUniform(&GlobalOpenGLRect.Shader, "model"), Model);
    UniformSet(mat4, ShaderGetUniform(&GlobalOpenGLRect.Shader, "view"), View);
    UniformSet(mat4, ShaderGetUniform(&GlobalOpenGLRect.Shader, "projection"), Projection);
    
    v4 c = u32toV4(color);
    c = v4(c.x/255, c.y/255, c.z/255, c.w/255);
    UniformSet(v4, ShaderGetUniform(&GlobalOpenGLRect.Shader, "my_color"), c);
    
    AttributeBindTo(v3, &GlobalOpenGLRect.VertexPositions, ShaderGetAttribute(&GlobalOpenGLRect.Shader, "position"));
    
    glDraw(GlobalOpenGLRect.IndexBuffer, render_draw_mode::triangles);
    
    AttributeUnBindFrom(&GlobalOpenGLRect.VertexPositions, ShaderGetAttribute(&GlobalOpenGLRect.Shader, "position"));
    
    ShaderUnBind(&GlobalOpenGLRect.Shader);
    
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        PrintqDebug(S() + "Opengl Error: " + (int)err + "\n");
    }
}

void
DrawRect(v3 Coords, v2 Size, v2 ScissorCoords, v2 ScissorDim, render_texture *Texture, real32 Rotation, render_blend_mode BlendMode)
{
    if (GlobalOpenGLTexture.Initialized == 0)
    {
        ShaderInit(&GlobalOpenGLTexture.Shader, "../shaders/static.vert", "../shaders/lit.frag");
        
        AttributeInit(&GlobalOpenGLTexture.VertexPositions);
        DynArray<v3> position = {};
        position.push_back(v3(-0.5, -0.5, 0));
        position.push_back(v3(-0.5, 0.5, 0));
        position.push_back(v3(0.5, -0.5, 0));
        position.push_back(v3(0.5, 0.5, 0));
        AttributeSet(&GlobalOpenGLTexture.VertexPositions, position.GetData(), sizeof(v3), position.GetSize());
        
        AttributeInit(&GlobalOpenGLTexture.VertexNormals);
        DynArray<v3> normals = {};
        normals.Resize(4, v3(0, 0, 1));
        AttributeSet(&GlobalOpenGLTexture.VertexNormals, normals.GetData(), sizeof(v3), normals.GetSize());
        
        AttributeInit(&GlobalOpenGLTexture.VertexTexCoords);
        DynArray<v2> uvs = {};
        uvs.push_back(v2(1, 1));
        uvs.push_back(v2(1, 0));
        uvs.push_back(v2(0, 1));
        uvs.push_back(v2(0, 0));
        AttributeSet(&GlobalOpenGLTexture.VertexTexCoords, uvs.GetData(), sizeof(v2), uvs.GetSize());
        
        IndexBufferInit(&GlobalOpenGLTexture.IndexBuffer);
        DynArray<u32> indices = {};
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(2);
        indices.push_back(1);
        indices.push_back(3);
        IndexBufferSet(&GlobalOpenGLTexture.IndexBuffer, indices);
        
        GlobalOpenGLTexture.Initialized = 1;
    }
    
    // Change to standard coordinate system
    v2 NewCoords = {};
    NewCoords.x = (real32)(-Coords.x - (Size.x/2));
    NewCoords.y = (real32)(-Coords.y - (Size.y/2));
    
    mat4 Model = TransformToMat4(Transform(v3(NewCoords, Coords.z),
                                           AngleAxis(Rotation * DEG2RAD, v3(0, 0, 1)),
                                           v3(Size.x, Size.y, 1)));
    
    if (ScissorDim.x > 0 && ScissorDim.y > 0) {
        glScissor((GLsizei)(ScissorCoords.x + (CameraViewDim.x/2)), 
                  (GLsizei)(-ScissorCoords.y + (CameraViewDim.y/2)), 
                  (GLint)ScissorDim.x,
                  (GLint)ScissorDim.y);
        //glScissor(500, 500, 100, 100);
        glEnable(GL_SCISSOR_TEST);
    }
    
    if (BlendMode == render_blend_mode::gl_one)
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    else if (BlendMode == render_blend_mode::gl_src_alpha)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    ShaderBind(&GlobalOpenGLTexture.Shader);
    
    UniformSet(mat4, ShaderGetUniform(&GlobalOpenGLTexture.Shader, "model"), Model);
    UniformSet(mat4, ShaderGetUniform(&GlobalOpenGLTexture.Shader, "view"), View);
    v3 Light = v3(0, 0, 1);
    UniformSet(v3, ShaderGetUniform(&GlobalOpenGLTexture.Shader, "light"), Light);
    UniformSet(mat4, ShaderGetUniform(&GlobalOpenGLTexture.Shader, "projection"), Projection);
    
    AttributeBindTo(v3, &GlobalOpenGLTexture.VertexPositions, ShaderGetAttribute(&GlobalOpenGLTexture.Shader, "position"));
    AttributeBindTo(v3, &GlobalOpenGLTexture.VertexNormals, ShaderGetAttribute(&GlobalOpenGLTexture.Shader, "normal"));
    AttributeBindTo(v2, &GlobalOpenGLTexture.VertexTexCoords, ShaderGetAttribute(&GlobalOpenGLTexture.Shader, "texCoord"));
    
    TextureSet(Texture, ShaderGetUniform(&GlobalOpenGLTexture.Shader, "tex0"), 0);
    
    glDraw(GlobalOpenGLTexture.IndexBuffer, render_draw_mode::triangles);
    
    TextureUnSet(Texture, 0);
    
    AttributeUnBindFrom(&GlobalOpenGLTexture.VertexPositions, ShaderGetAttribute(&GlobalOpenGLTexture.Shader, "position"));
    AttributeUnBindFrom(&GlobalOpenGLTexture.VertexNormals, ShaderGetAttribute(&GlobalOpenGLTexture.Shader, "normal"));
    AttributeUnBindFrom(&GlobalOpenGLTexture.VertexTexCoords, ShaderGetAttribute(&GlobalOpenGLTexture.Shader, "texCoord"));
    
    ShaderUnBind(&GlobalOpenGLTexture.Shader);
    
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        PrintqDebug(S() + (int)err + "\n");
    }
    
    if (ScissorDim.x > 0 && ScissorDim.y > 0)
        glDisable(GL_SCISSOR_TEST);
    
}
inline void DrawRect(v3 Coords, v2 Size, render_texture *Texture, real32 Rotation, blend_mode BlendMode)
{
    DrawRect(Coords, Size, v2(0, 0), v2(0, 0), Texture, Rotation, BlendMode);
}

#else // !QLIB_OPENGL

// Software Rendering
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
            if((Pixel >= Buffer->Memory) &&
               ((Pixel + 4) <= EndOfBuffer))
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

inline unsigned long createRGBA(int r, int g, int b, int a) { return ((a & 0xff) << 24) + ((r & 0xff) << 16) + ((g & 0xff) << 8) + ((b & 0xff));}
inline unsigned long createRGB(int r, int g, int b) { return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff); }

void
DrawRect(int x, int y, int width, int height, Texture texture)
{
    platform_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    Image re = {};
    re.data = texture.data;
    re.x = texture.mWidth;
    re.y = texture.mHeight;
    re.n = texture.mChannels;
    //RenderImage(Buffer, &re);
    
    x += Buffer->Width / 2;
    y += Buffer->Height / 2;
    
    uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
    
    for(int X = x; X < (x + width); ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory + X * Buffer->BytesPerPixel + y*Buffer->Pitch);
        uint8 *Color = ((uint8 *)re.data + (X - x) * re.n);
        
        for(int Y = y; Y < (y + height); ++Y)
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

internal void
ChangeBitmapColor(loaded_bitmap Bitmap, uint32 Color)
{
    u8 *DestRow = (u8 *)Bitmap.Memory + (Bitmap.Height -1)*Bitmap.Pitch;
    for(s32 Y = 0;
        Y < Bitmap.Height;
        ++Y)
    {
        u32 *Dest = (u32 *)DestRow;
        for(s32 X = 0;
            X < Bitmap.Width;
            ++X)
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
/*
void
PrintOnScreen(Font* SrcFont, char* SrcText, int InputX, int InputY, uint32 Color)
{
    platform_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    InputX += Buffer->Width / 2;
    InputY += Buffer->Height / 2;
    
    int StrLength = StringLength(SrcText);
    int BiggestY = 0;
    
    for (int i = 0; i < StrLength; i++)
    {
        int SrcChar = SrcText[i];
        FontChar NextChar = LoadFontChar(SrcFont, SrcChar, Color);
        int Y = -1 *  NextChar.C_Y1;
        if(BiggestY < Y)
        {
            BiggestY = Y;
        }
    }
    
    real32 X = (real32)InputX;
    
    for (int i = 0; i < StrLength; i++)
    {
        int SrcChar = SrcText[i];
        
        FontChar NextChar = LoadFontChar(SrcFont, SrcChar, Color);
        
        int Y = InputY + NextChar.C_Y1 + BiggestY;
        
        loaded_bitmap SrcBitmap = {};
        SrcBitmap.Width = NextChar.Width;
        SrcBitmap.Height = NextChar.Height;
        SrcBitmap.Pitch = NextChar.Pitch;
        SrcBitmap.Memory = NextChar.Memory;
        
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&SrcFont->Info, SrcText[i], &ax, &lsb);
        
        //ChangeBitmapColor(SrcBitmap, Color);
        RenderBitmap(&SrcBitmap, X + (lsb * SrcFont->Scale) , (real32)Y);
        
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&SrcFont->Info, SrcText[i], SrcText[i + 1]);
        X += ((kern + ax) * SrcFont->Scale);
    }
}
*/
#endif

//
// track
//

template track<real32, 1>;
template track<v3, 3>;
template track<quat, 4>;

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

template<typename T, int N> T
TrackSample(track<T, N> *Track, real32 Time, bool Looping)
{
    if (Track->Interpolation = interpolation::constant)
        return SampleConstant(Time, Looping);
    else if (Track->Interpolation = interpolation::constant)
        return SampleLinear(Time, Looping);
    else if (Track->Interpolation = interpolation::cubic)
        return SampleCubic(Time, Looping);
}

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
    return Frames.GetSize();
}

//
// FPS Counter
//

internal void
DrawFPS(real32 Seconds, v2 ScreenDim, font *Font)
{
    real32 fps = 0;
    if (Seconds != 0) {
        fps= 1 / Seconds;
        strinq FPS = S() + (int)fps;
        
        font_string FontString = {};
        FontStringInit(&FontString, Font, FPS.Data, 50, 0xFFFFFF00);
        v2 SDim = FontStringGetDim(&FontString);
        FontStringPrint(&FontString, v2((ScreenDim.x/2)-(int)SDim.x-10, -ScreenDim.y/2 + 10));
    }
}