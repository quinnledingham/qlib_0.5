// SHADER 

void
Shader::Init()
{
    mHandle = glCreateProgram();
}

void
Shader::Init(const char* vertex, const char* fragment)
{
    // Init
    mHandle = glCreateProgram();
    
    mAttributes.Init();
    mUniforms.Init();
    
    // Load
    entire_file vertFile = ReadEntireFile(vertex);
    entire_file fragFile = ReadEntireFile(fragment);
    
    strinq v_source = NewStrinq(&vertFile);
    strinq f_source = NewStrinq(&fragFile);
    
    // Compile Vertex Shader
    u32 v_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* v = v_source.Data; // v_source needs to be 0 terminated
    glShaderSource(v_shader, 1, &v, NULL);
    glCompileShader(v_shader);
    int GotVertexShader = 0;
    glGetShaderiv(v_shader, GL_COMPILE_STATUS, &GotVertexShader);
    
    if (GotVertexShader)
    {
        unsigned vert = v_shader;
        
        // Compile Fragment Shader
        u32 f_shader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* f = f_source.Data;  // f_source needs to be 0 terminated
        glShaderSource(f_shader, 1, &f, NULL);
        glCompileShader(f_shader);
        
        int GotFragmentShader = 0;
        glGetShaderiv(f_shader, GL_COMPILE_STATUS, &GotFragmentShader);
        
        if (GotFragmentShader)
        {
            unsigned int frag = f_shader;
            
            // Link
            glAttachShader(mHandle, vert);
            glAttachShader(mHandle, frag);
            glLinkProgram(mHandle);
            int GotProgram = 0;
            glGetProgramiv(mHandle, GL_LINK_STATUS, &GotProgram);
            
            if (GotProgram)
            {
                glDeleteShader(vert);
                glDeleteShader(frag);
                
                // Populate Attributes
                {
                    int count = -1;
                    int length;
                    char name[128];
                    int size;
                    GLenum type;
                    
                    glUseProgram(mHandle);
                    glGetProgramiv(mHandle, GL_ACTIVE_ATTRIBUTES, &count);
                    
                    for (int i = 0; i < count; ++i)
                    {
                        memset(name, 0, sizeof(char) * 128);
                        glGetActiveAttrib(mHandle, (GLuint)i, 128, &length, &size, &type, name);
                        int attrib = glGetAttribLocation(mHandle, name);
                        if (attrib >= 0)
                        {
                            mAttributes[name] = attrib;
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
                    
                    glUseProgram(mHandle);
                    glGetProgramiv(mHandle, GL_ACTIVE_UNIFORMS, &count);
                    
                    for (int i = 0; i < count; ++i)
                    {
                        memset(name, 0, sizeof(char) * 128);
                        glGetActiveUniform(mHandle, (GLuint)i, 128, &length, &size, &type, name);
                        int uniform = glGetUniformLocation(mHandle, name);
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
                                    int uniformLocation = glGetUniformLocation(mHandle, testName);
                                    
                                    if (uniformLocation < 0)
                                    {
                                        break;
                                    }
                                    
                                    mUniforms[testName] = uniformLocation;
                                }
                            }
                            
                            mUniforms[uniformName] = uniform;
                            DestroyStrinq(&uniformName);
                        }
                    }
                    
                    glUseProgram(0);
                }
                
            }
            else // !GotProgram
            {
                char infoLog[512];
                glGetProgramInfoLog(mHandle, 512, NULL, infoLog);
                PrintqDebug("ERROR: Shader linking failed.\n");
                PrintqDebug(S() + "\t" + infoLog + "\n");
                glDeleteShader(vert);
                glDeleteShader(frag);
            }
        }
        else // !GotFragmentShader
        {
            char infoLog[512];
            glGetShaderInfoLog(f_shader, 512, NULL, infoLog);
            PrintqDebug("Fragment compilation failed.\n");
            PrintqDebug(S() + "\t" + infoLog + "\n");
            glDeleteShader(f_shader);
            return;
        }
    }
    else // !GotVertexShader
    {
        char infoLog[512];
        glGetShaderInfoLog(v_shader, 512, NULL, infoLog);
        PrintqDebug("Vertex compilation failed.\n");
        PrintqDebug(S() + "\t" + infoLog + "\n");
        
        glDeleteShader(v_shader);
    }
    
    DestroyStrinq(&v_source);
    DestroyStrinq(&f_source);
    DestroyEntireFile(vertFile);
    DestroyEntireFile(fragFile);
}

void
Shader::Destroy()
{
    glDeleteProgram(mHandle);
}

void
Shader::Bind()
{
    glUseProgram(mHandle);
}

void
Shader::UnBind()
{
    glUseProgram(0);
}

unsigned int 
Shader::GetHandle()
{
    return mHandle;
}

unsigned int 
Shader::GetAttribute(const char* name)
{
    //Strinq Name = {};
    //NewStrinq(Name, name);
    unsigned int r =  mAttributes.MapFind(name);
    //DestroyStrinq(Name);
    
    return r;
}

unsigned int
Shader::GetUniform(const char* name)
{
    //Strinq Name = {};
    //NewStrinq(Name, name);
    unsigned int r =  mUniforms.MapFind(name);
    //DestroyStrinq(Name);
    
    return r;
}



// ATTRIBUTE

template Attribute<int>;
template Attribute<float>;
template Attribute<v2>;
template Attribute<v3>;
template Attribute<v4>;
template Attribute<iv4>;
template Attribute<quat>;

template<typename T> void
Attribute<T>::Init()
{
    glGenBuffers(1, &mHandle);
    mCount = 0;
}

template<typename T> void 
Attribute<T>::Destroy()
{
    glDeleteBuffers(1, &mHandle);
}

template<typename T> unsigned int 
Attribute<T>::Count()
{
    return mCount;
}

template<typename T> unsigned int 
Attribute<T>::GetHandle()
{
    return mHandle;
}

template<typename T> void
Attribute<T>::Set(T* inputArray, unsigned int arrayLength)
{
    mCount = arrayLength;
    unsigned int size = sizeof(T);
    
    glBindBuffer(GL_ARRAY_BUFFER, mHandle);
    glBufferData(GL_ARRAY_BUFFER, size * mCount, inputArray, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template<typename T> void
Attribute<T>::Set(DynArray<T> &input)
{
    Set((T*)input.GetData(), input.GetSize());
}

template<> void
Attribute<int>::SetAttribPointer(unsigned int s)
{
    glVertexAttribIPointer(s, 1, GL_INT, 0, (void*)0);
}

template<> void
Attribute<iv4>::SetAttribPointer(unsigned int s)
{
    glVertexAttribIPointer(s, 4, GL_INT, 0, (void*)0);
}

template<> void 
Attribute<float>::SetAttribPointer(unsigned int s)
{
    glVertexAttribPointer(s, 1, GL_FLOAT, GL_FALSE, 0, 0);
}

template<> void
Attribute<v2>::SetAttribPointer(unsigned int s)
{
    glVertexAttribPointer(s, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

template<> void
Attribute<v3>::SetAttribPointer(unsigned int s)
{
    glVertexAttribPointer(s, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

template<> void
Attribute<v4>::SetAttribPointer(unsigned int s)
{
    glVertexAttribPointer(s, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

template<> void
Attribute<quat>::SetAttribPointer(unsigned int slot) {
	glVertexAttribPointer(slot, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

template<typename T> void
Attribute<T>::BindTo(unsigned int slot)
{
    glBindBuffer(GL_ARRAY_BUFFER, mHandle);
    glEnableVertexAttribArray(slot);
    SetAttribPointer(slot);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template<typename T> void
Attribute<T>::UnBindFrom(unsigned int slot)
{
    glBindBuffer(GL_ARRAY_BUFFER, mHandle);
    glDisableVertexAttribArray(slot);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// UNIFORM

template Uniform<int>;
template Uniform<iv4>;
template Uniform<iv2>;
template Uniform<float>;
template Uniform<v2>;
template Uniform<v3>;
template Uniform<v4>;
template Uniform<quat>;
template Uniform<mat4>;

#define UNIFORM_IMPL(gl_func, tType, dType) \
template<> \
void Uniform<tType>::Set(unsigned int slot, tType* data, unsigned int length) { \
gl_func(slot, (GLsizei)length, (dType*)&data[0]); \
}

UNIFORM_IMPL(glUniform1iv, int, int)
UNIFORM_IMPL(glUniform4iv, iv4, int)
UNIFORM_IMPL(glUniform2iv, iv2, int)
UNIFORM_IMPL(glUniform1fv, float, float)
UNIFORM_IMPL(glUniform2fv, v2, float)
UNIFORM_IMPL(glUniform3fv, v3, float)
UNIFORM_IMPL(glUniform4fv, v4, float)
UNIFORM_IMPL(glUniform4fv, quat, float)

template<> void
Uniform<mat4>::Set(unsigned int slot, mat4* inputArray, unsigned int arrayLength)
{
    glUniformMatrix4fv(slot, (GLsizei)arrayLength, false, (float*) &inputArray[0]);
}

template<typename T>  void
Uniform<T>::Set(unsigned int slot, const T& value)
{
    Set(slot, (T*)&value, 1);
}

template<typename T> void
Uniform<T>::Set(unsigned int s, DynArray<T> &v)
{
    Set(s, (T*)v.GetData(), v.GetSize());
}

// INDEXBUFFER

void
IndexBuffer::Init()
{
    glGenBuffers(1, &mHandle);
    mCount = 0;
}

void
IndexBuffer::Destroy()
{
    glDeleteBuffers(1, &mHandle);
}

unsigned int
IndexBuffer::Count()
{
    return mCount;
}

unsigned int
IndexBuffer::GetHandle()
{
    return mHandle;
}

void
IndexBuffer::Set(unsigned int* inputArray, unsigned int arrayLength)
{
    mCount = arrayLength;
    unsigned int size = sizeof(unsigned int);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * mCount, inputArray, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::Set(DynArray<unsigned int>& input)
{
    Set((unsigned int*)input.GetData(), input.GetSize());
}

// TEXTURE

void
Texture::Init()
{
    mWidth = 0;
    mHeight = 0;
    mChannels = 0;
    glGenTextures(1, &mHandle);
    
    Initialized = true;
}

void
Texture::Init(unsigned char* Data)
{
    glGenTextures(1, &mHandle);
    glBindTexture(GL_TEXTURE_2D, mHandle);
    if (mChannels == 3) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)mWidth, (int)mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else if (mChannels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)mWidth, (int)mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Tile
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    Initialized = true;
}

void
Texture::Init(Image* image)
{
    glGenTextures(1, &mHandle);
    
    glBindTexture(GL_TEXTURE_2D, mHandle);
    //int width, height, channels;
    //unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
    //unsigned char* Data = image->data;
    //width = image->x;
    //height = image->y;
    //channels = image->n;
    //data = image->data;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->x, image->y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
    glGenerateMipmap(GL_TEXTURE_2D);
    //stbi_image_free(data);
    
    // Tile
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    mWidth = image->x;
    mHeight = image->y;
    mChannels = image->n;
    
    Initialized = true;
}

void
Texture::Init(const char* path)
{
    Image i = LoadImage(path);
    Init(&i);
    stbi_image_free(i.data);
    Initialized = true;
}

void
Texture::Destroy()
{
    glDeleteTextures(1, &mHandle);
}

void
Texture::Set(unsigned int uniformIndex, unsigned int textureIndex)
{
    if (!Initialized)
        Init(data);
    
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindTexture(GL_TEXTURE_2D, mHandle);
    glUniform1i(uniformIndex, textureIndex);
}

void
Texture::UnSet(unsigned int textureIndex)
{
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
}

unsigned int
Texture::GetHandle()
{
    return mHandle;
}

internal void
ResizeTexture(Texture *Tex, v2 Dim)
{
    //unsigned char* delsize = Tex->data;
    //Tex->data = (unsigned char*)qalloc((int)Dim.x * (int)Dim.y * Tex->mChannels);
    //stbi_image_free(Tex->data);
    
    if (!stbir_resize_uint8(Tex->og.data, Tex->og.x, Tex->og.y, 0, Tex->data, (int)Dim.x, (int)Dim.y, 0, Tex->mChannels))
        PrintqDebug(S() + "ResizeTexture(): stbir_resize_uint8 Error\n");
    
    Tex->mWidth = (int)Dim.x;
    Tex->mHeight = (int)Dim.y;
    Tex->mChannels = Tex->mChannels;
    
    //dalloc((void*)delsize);
    //
    
    Tex->Init(Tex->data);
}

internal Texture*
LoadTexture(const char* FileName)
{
    Texture Tex = {};
    unsigned char* data = stbi_load(FileName, (int*)&Tex.mWidth, (int*)&Tex.mHeight, (int*)&Tex.mChannels, 0);
    Assert(Tex.mWidth != 0 && Tex.mHeight != 0);
    Tex.data = data;
    //Tex.data = (unsigned char*)qalloc((void*)data, Tex.mWidth * Tex.mHeight * Tex.mChannels);
    //stbi_image_free(data);
    
    Tex.og.x = Tex.mWidth;
    Tex.og.y = Tex.mHeight;
    Tex.og.n = Tex.mChannels;
    Tex.og.data = (unsigned char*)qalloc((void*)Tex.data, Tex.mWidth * Tex.mHeight * Tex.mChannels);
    
    //Tex.Init(Tex.data);
    return (Texture*)qalloc(&Tex, sizeof(Texture));
}

internal void
SaveTextureEthan(Texture* Tex, const char* SaveFileName)
{
    FILE *File = fopen(SaveFileName, "wb");
    ImageHeader Header = 
    {
        Tex->mWidth,
        Tex->mHeight,
        Tex->mChannels,
    };
    fwrite(&Header, sizeof(struct ImageHeader), 1, File);
    fwrite(Tex->data, Tex->mWidth * Tex->mHeight * Tex->mChannels, 1, File);
    fclose(File);
}

internal Texture*
LoadTextureEthan(Texture *Tex, const char* LoadFileName)
{
    entire_file File = ReadEntireFile(LoadFileName);
    ImageHeader *Header = (ImageHeader*)File.Contents;
    char* Cursor = (char*)File.Contents + sizeof(ImageHeader);
    Tex->mWidth = Header->x;
    Tex->mHeight = Header->y;
    Tex->mChannels = Header->n;
    Tex->data = (unsigned char*)qalloc((void*)Cursor, Header->x * Header->y * Header->n);
    //Tex->data = (unsigned char*)malloc(Header->x * Header->y * Header->n);
    //memcpy(Tex->data, Cursor, Header->x * Header->y * Header->n);
    
    Tex->og.x = Tex->mWidth;
    Tex->og.y = Tex->mHeight;
    Tex->og.n = Tex->mChannels;
    Tex->og.data = (unsigned char*)qalloc((void*)Cursor, Header->x * Header->y * Header->n);
    
    Tex->Init(Tex->data);
    return Tex;
}
// End of Texture

// Drawing Methods
static GLenum DrawModeToGLEnum(DrawMode input) {
	if (input == DrawMode::Points) {
		return  GL_POINTS;
	}
	else if (input == DrawMode::LineStrip) {
		return GL_LINE_STRIP;
	}
	else if (input == DrawMode::LineLoop) {
		return  GL_LINE_LOOP;
	}
	else if (input == DrawMode::Lines) {
		return  GL_LINES;
	}
	else if (input == DrawMode::Triangles) {
		return  GL_TRIANGLES;
	}
	else if (input == DrawMode::TriangleStrip) {
		return  GL_TRIANGLE_STRIP;
	}
	else if (input == DrawMode::TriangleFan) {
		return   GL_TRIANGLE_FAN;
	}
    
    PrintqDebug("DrawModeToGLEnum unreachable code hit\n");
	return 0;
}

global_variable int InMode3D = 0;
global_variable int InMode2D = 0;

void 
glDraw(unsigned int vertexCount, DrawMode mode)
{
    glDrawArrays(DrawModeToGLEnum(mode), 0, vertexCount);
}

void
glDraw(IndexBuffer& inIndexBuffer, DrawMode mode)
{
    if (!InMode3D && !InMode2D)
    {
        PrintqDebug("Error: Trying to print while not in Mode3D\n");
        //return;
    }
    
    unsigned int handle = inIndexBuffer.GetHandle();
    unsigned int numIndices = inIndexBuffer.Count();
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
    glDrawElements(DrawModeToGLEnum(mode), numIndices, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void
glDrawInstanced(unsigned int vertexCount, DrawMode node, unsigned int numInstances)
{
    glDrawArraysInstanced(DrawModeToGLEnum(node), 0, vertexCount, numInstances);
}

void
glDrawInstanced(IndexBuffer& inIndexBuffer, DrawMode mode, unsigned int instanceCount)
{
    unsigned int handle = inIndexBuffer.GetHandle();
    unsigned int numIndices = inIndexBuffer.Count();
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
    glDrawElementsInstanced(DrawModeToGLEnum(mode), numIndices, GL_UNSIGNED_INT, 0, instanceCount);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Renderering Elements

mat4 projection;
mat4 view;
v2 CameraViewDim;

// Paints the screen white
internal void
ClearScreen()
{
    platform_offscreen_buffer *Buffer = &GlobalBackbuffer;
    memset(Buffer->Memory, 0xFF, (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel);
}

void BeginOpenGL(platform_window_dimension Dimension)
{
    glViewport(0, 0, Dimension.Width, Dimension.Height);
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

void BeginMode(Camera C)
{
#if QLIB_OPENGL
    BeginOpenGL(C.Dimension);
#else
    ClearScreen();
#endif
    view = LookAt(C.Position, C.Target, C.Up);
}

void
BeginMode2D(Camera C)
{
    BeginMode(C);
    CameraViewDim = v2((float)C.Dimension.Width, (float)C.Dimension.Height);
    float width = (float)C.Dimension.Width;
    float height = (float)C.Dimension.Height;
    projection = Ortho(-width/2, width/2, -height/2, height/2, 0.1f, 1000.0f);
    InMode2D = 1;
}

void
BeginMode3D(Camera C)
{
    BeginMode(C);
    CameraViewDim = v2((float)C.Dimension.Width, (float)C.Dimension.Height);
    C.inAspectRatio = (float)C.Dimension.Width / (float)C.Dimension.Height;
    projection = Perspective(C.FOV, C.inAspectRatio, 0.01f, 1000.0f);
    InMode3D = 1;
}

void
EndMode3D()
{
    InMode3D = 0;
}

void
EndMode2D()
{
    InMode2D = 0;
}

#define NOFILL 0
#define FILL 1

#if QLIB_OPENGL

struct open_gl_rect
{
    Shader shader;
    IndexBuffer rIndexBuffer;
    Attribute<v3> VertexPositions;
    Attribute<v4> Color;
    
    bool32 Initialized;
};

struct open_gl_texture
{
    Shader shader;
    Attribute<v3> VertexPositions;
    IndexBuffer rIndexBuffer;
    Attribute<v3> VertexNormals;
    Attribute<v2> VertexTexCoords;
    
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
        GlobalOpenGLRect.shader.Init("../shaders/basic.vert", "../shaders/basic.frag");
        
        GlobalOpenGLRect.VertexPositions.Init();
        DynArray<v3> position = {};
        position.push_back(v3(-0.5, -0.5, 0));
        position.push_back(v3(-0.5, 0.5, 0));
        position.push_back(v3(0.5, -0.5, 0));
        position.push_back(v3(0.5, 0.5, 0));
        GlobalOpenGLRect.VertexPositions.Set(position);
        
        GlobalOpenGLRect.rIndexBuffer.Init();
        DynArray<unsigned int> indices = {};
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(2);
        indices.push_back(1);
        indices.push_back(3);
        GlobalOpenGLRect.rIndexBuffer.Set(indices);
        
        GlobalOpenGLRect.Initialized = 1;
    }
    
    // Change to standard coordinate system
    v2 NewCoords = {};
    NewCoords.x = (real32)(-Coords.x - (Size.x/2));
    NewCoords.y = (real32)(-Coords.y - (Size.y/2));
    
    mat4 model = TransformToMat4(Transform(v3(NewCoords, Coords.z), AngleAxis(Rotation * DEG2RAD, v3(0, 0, 1)), v3(Size.x, Size.y, 1)));
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    GlobalOpenGLRect.shader.Bind();
    
    Uniform<mat4>::Set(GlobalOpenGLRect.shader.GetUniform("model"), model);
    Uniform<mat4>::Set(GlobalOpenGLRect.shader.GetUniform("view"), view);
    Uniform<mat4>::Set(GlobalOpenGLRect.shader.GetUniform("projection"), projection);
    v4 c = u32toV4(color);
    Uniform<v4>::Set(GlobalOpenGLRect.shader.GetUniform("my_color"), v4(c.x/255, c.y/255, c.z/255, c.w/255));
    
    GlobalOpenGLRect.VertexPositions.BindTo(GlobalOpenGLRect.shader.GetAttribute("position"));
    
    glDraw(GlobalOpenGLRect.rIndexBuffer, DrawMode::Triangles);
    
    GlobalOpenGLRect.VertexPositions.UnBindFrom(GlobalOpenGLRect.shader.GetAttribute("position"));
    
    GlobalOpenGLRect.shader.UnBind();
    
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        PrintqDebug(S() + (int)err + "\n");
    }
}

void
DrawRect(v3 Coords, v2 Size, v2 ScissorCoords, v2 ScissorDim, Texture *Tex, real32 Rotation, BlendMode Mode)
{
    if (GlobalOpenGLTexture.Initialized == 0)
    {
        GlobalOpenGLTexture.shader.Init("../shaders/static.vert", "../shaders/lit.frag");
        
        GlobalOpenGLTexture.VertexPositions.Init();
        DynArray<v3> position = {};
        position.push_back(v3(-0.5, -0.5, 0));
        position.push_back(v3(-0.5, 0.5, 0));
        position.push_back(v3(0.5, -0.5, 0));
        position.push_back(v3(0.5, 0.5, 0));
        GlobalOpenGLTexture.VertexPositions.Set(position);
        
        GlobalOpenGLTexture.VertexNormals.Init();
        DynArray<v3> normals = {};
        normals.Resize(4, v3(0, 0, 1));
        GlobalOpenGLTexture.VertexNormals.Set(normals);
        
        GlobalOpenGLTexture.VertexTexCoords.Init();
        DynArray<v2> uvs = {};
        uvs.push_back(v2(1, 1));
        uvs.push_back(v2(1, 0));
        uvs.push_back(v2(0, 1));
        uvs.push_back(v2(0, 0));
        GlobalOpenGLTexture.VertexTexCoords.Set(uvs);
        
        GlobalOpenGLTexture.rIndexBuffer.Init();
        DynArray<unsigned int> indices = {};
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(2);
        indices.push_back(1);
        indices.push_back(3);
        GlobalOpenGLTexture.rIndexBuffer.Set(indices);
        
        GlobalOpenGLTexture.Initialized = 1;
    }
    
    // Change to standard coordinate system
    v2 NewCoords = {};
    NewCoords.x = (real32)(-Coords.x - (Size.x/2));
    NewCoords.y = (real32)(-Coords.y - (Size.y/2));
    
    mat4 model = TransformToMat4(Transform(v3(NewCoords, Coords.z),
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
    
    if (Mode == BlendMode::gl_one)
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    else if (Mode == BlendMode::gl_src_alpha)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    GlobalOpenGLTexture.shader.Bind();
    
    Uniform<mat4>::Set(GlobalOpenGLTexture.shader.GetUniform("view"), view);
    Uniform<mat4>::Set(GlobalOpenGLTexture.shader.GetUniform("projection"), projection);
    Uniform<v3>::Set(GlobalOpenGLTexture.shader.GetUniform("light"), v3(0, 0, 1));
    Uniform<mat4>::Set(GlobalOpenGLTexture.shader.GetUniform("model"), model);
    
    GlobalOpenGLTexture.VertexPositions.BindTo(GlobalOpenGLTexture.shader.GetAttribute("position"));
    GlobalOpenGLTexture.VertexNormals.BindTo(GlobalOpenGLTexture.shader.GetAttribute("normal"));
    GlobalOpenGLTexture.VertexTexCoords.BindTo(GlobalOpenGLTexture.shader.GetAttribute("texCoord"));
    
    Tex->Set(GlobalOpenGLTexture.shader.GetUniform("tex0"), 0);
    
    glDraw(GlobalOpenGLTexture.rIndexBuffer, DrawMode::Triangles);
    
    Tex->UnSet(0);
    
    GlobalOpenGLTexture.VertexPositions.UnBindFrom(GlobalOpenGLTexture.shader.GetAttribute("position"));
    GlobalOpenGLTexture.VertexNormals.UnBindFrom(GlobalOpenGLTexture.shader.GetAttribute("normal"));
    GlobalOpenGLTexture.VertexTexCoords.UnBindFrom(GlobalOpenGLTexture.shader.GetAttribute("texCoord"));
    
    GlobalOpenGLTexture.shader.UnBind();
    
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        PrintqDebug(S() + (int)err + "\n");
    }
    
    if (ScissorDim.x > 0 && ScissorDim.y > 0)
        glDisable(GL_SCISSOR_TEST);
    
}
inline void DrawRect(v3 Coords, v2 Size, Texture *Tex, real32 Rotation, BlendMode Mode)
{
    DrawRect(Coords, Size, v2(0, 0), v2(0, 0), Tex, Rotation, Mode);
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

void
PrintqScreen(Font* SrcFont, char* SrcText, int InputX, int InputY, uint32 Color)
{
    
}

#endif