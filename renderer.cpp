#include "application.h"
#include "data_structures.h"

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
    
    // Load
    entire_file vertFile = ReadEntireFile(vertex);
    entire_file fragFile = ReadEntireFile(fragment);
    
    Strinq v_source = NewStrinq(&vertFile);
    Strinq f_source = NewStrinq(&fragFile);
    
    // Compile Vertex Shader
    u32 v_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* v = GetData(v_source); // v_source needs to be 0 terminated
    glShaderSource(v_shader, 1, &v, NULL);
    glCompileShader(v_shader);
    int GotVertexShader = 0;
    glGetShaderiv(v_shader, GL_COMPILE_STATUS, &GotVertexShader);
    
    if (GotVertexShader)
    {
        unsigned vert = v_shader;
        
        // Compile Fragment Shader
        u32 f_shader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* f = GetData(f_source);  // f_source needs to be 0 terminated
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
                            Strinq uniformName = {};
                            NewStrinq(uniformName, name);
                            // if name contains [, uniform is array
                            int found = StrinqFind(uniformName, '[');
                            
                            if (found != -1)
                            {
                                StrinqErase(uniformName, found);
                                unsigned int uniformIndex = 0;
                                while (true)
                                {
                                    memset(testName, 0, sizeof(char) * 256);
                                    
                                    Strinq n = S() + uniformName + "[" + uniformIndex++ + "]";
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
                            DestroyStrinq(uniformName);
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
    
    DestroyStrinq(v_source);
    DestroyStrinq(f_source);
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
    Strinq Name = {};
    NewStrinq(Name, name);
    unsigned int r =  mAttributes.MapFind(Name);
    DestroyStrinq(Name);
    
    return r;
}

unsigned int
Shader::GetUniform(const char* name)
{
    Strinq Name = {};
    NewStrinq(Name, name);
    unsigned int r =  mUniforms.MapFind(Name);
    DestroyStrinq(Name);
    
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
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

void
Texture::Init()
{
    mWidth = 0;
    mHeight = 0;
    mChannels = 0;
    glGenTextures(1, &mHandle);
}

void
Texture::Init(const char* path)
{
    glGenTextures(1, &mHandle);
    
    glBindTexture(GL_TEXTURE_2D, mHandle);
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    mWidth = width;
    mHeight = height;
    mChannels = channels;
}

void
Texture::Destroy()
{
    glDeleteTextures(1, &mHandle);
}

void
Texture::Set(unsigned int uniformIndex, unsigned int textureIndex)
{
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

void 
glDraw(unsigned int vertexCount, DrawMode mode)
{
    glDrawArrays(DrawModeToGLEnum(mode), 0, vertexCount);
}

void
glDraw(IndexBuffer& inIndexBuffer, DrawMode mode)
{
    if (!InMode3D)
    {
        //PrintqDebug("Error: Trying to print while not in Mode3D\n");
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


void
Rect::Init(Shader* s)
{
    VertexPositions.Init();
    VertexNormals.Init();
    VertexTexCoords.Init();
    rIndexBuffer.Init();
    
    DynArray<v3> position = {};
    position.push_back(v3(-0.5, -0.5, 0));
    position.push_back(v3(-0.5, 0.5, 0));
    position.push_back(v3(0.5, -0.5, 0));
    position.push_back(v3(0.5, 0.5, 0));
    VertexPositions.Set(position);
    //position.Destroy();
    
    DynArray<v3> normals = {};
    normals.Resize(4, v3(0, 0, 1));
    VertexNormals.Set(normals);
    
    DynArray<v2> uvs = {};
    uvs.push_back(v2(0, 0));
    uvs.push_back(v2(0, 1));
    uvs.push_back(v2(1, 0));
    uvs.push_back(v2(1, 1));
    VertexTexCoords.Set(uvs);
    
    DynArray<unsigned int> indices = {};
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(3);
    rIndexBuffer.Set(indices);
    
    shader = s;
}

void
Rect::Destroy()
{
    
}

void
Rect::Draw(Texture &texture, v2 position2, v2 size, float rotate, v3 color)
{
    mat4 model = TransformToMat4(Transform(v3(position2, 0.0f),
                                           AngleAxis(rotate * DEG2RAD, v3(0, 0, 1)),
                                           v3(size.x, size.y, size.x)));
    
    shader->Bind();
    
    VertexPositions.BindTo(shader->GetAttribute("position"));
    VertexNormals.BindTo(shader->GetAttribute("normal"));
    VertexTexCoords.BindTo(shader->GetAttribute("texCoord"));
    
    Uniform<mat4>::Set(shader->GetUniform("model"), model);
    
    texture.Set(shader->GetUniform("tex0"), 0);
    
    glDraw(rIndexBuffer, DrawMode::Triangles);
    
    texture.UnSet(0);
    
    VertexPositions.UnBindFrom(shader->GetAttribute("position"));
    VertexNormals.UnBindFrom(shader->GetAttribute("normal"));
    VertexTexCoords.UnBindFrom(shader->GetAttribute("texCoord"));
    
    shader->UnBind();
}

mat4 projection;
mat4 view;

void
BeginMode3D(Camera C)
{
    //projection = Perspective(C.FOV, C.inAspectRatio, 1.0f, C.F);
    float width = (float)GlobalBackbuffer.Width;
    float height = (float)GlobalBackbuffer.Height;
    projection = Ortho(-width/2, width/2, -height/2, height/2, 0.01f, 1000.0f);
    view = LookAt(C.Position, C.Target, C.Up);
    InMode3D = 1;
}

void
EndMode3D()
{
    InMode3D = 0;
}

#define NOFILL 0
#define FILL 1

#if QLIB_OPENGL

struct open_gl_rect
{
    Shader shader;
    IndexBuffer rIndexBuffer;
    Attribute<v3> VertexPositions;
    v4 Color;
    
    bool32 Initialized;
};

global_variable open_gl_rect GlobalOpenGLRect;

v4 u32toV4(uint32 input)
{
    uint32 R = input & 0x00FF0000;
    uint32 G = input & 0x0000FF00;
    uint32 B = input & 0x000000FF;
    uint32 A = input & 0xFF000000;
    
    return v4(real32(R / 16777216), real32(G / 16777216), real32(B / 16777216), real32(A / 16777216));
}

internal void
DrawRect(int x, int y, int width, int height, int fill, uint32 color)
{
    if (GlobalOpenGLRect.Initialized == 0)
    {
        GlobalOpenGLRect.shader.Init("../game/shaders/basic.vert", "../game/shaders/basic.frag");
        
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
        
        GlobalOpenGLRect.Color = u32toV4(color);
        
        GlobalOpenGLRect.Initialized = 1;
    }
    
    mat4 model = TransformToMat4(Transform(v3(v2(x, y), 0.0f),
                                           AngleAxis(90 * DEG2RAD, v3(0, 0, 1)),
                                           v3((real32)width, (real32)height, 0)));
    
    GlobalOpenGLRect.shader.Bind();
    
    Uniform<mat4>::Set(GlobalOpenGLRect.shader.GetUniform("model"), model);
    Uniform<mat4>::Set(GlobalOpenGLRect.shader.GetUniform("view"), view);
    Uniform<mat4>::Set(GlobalOpenGLRect.shader.GetUniform("projection"), projection);
    
    GlobalOpenGLRect.VertexPositions.BindTo(GlobalOpenGLRect.shader.GetAttribute("position"));
    
    glDraw(GlobalOpenGLRect.rIndexBuffer, DrawMode::Triangles);
    
    GlobalOpenGLRect.VertexPositions.UnBindFrom(GlobalOpenGLRect.shader.GetAttribute("position"));
    
    GlobalOpenGLRect.shader.UnBind();
    
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        PrintqDebug(S() + (int)err);
    }
    
}

#else // !QLIB_OPENGL

// Software Rendering
internal void
DrawRect(int x, int y, int width, int height, int fill, uint32 color)
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
                if (fill == FILL)
                {
                    *(uint32 *)Pixel = Color;
                }
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
            }
            
            Pixel += Buffer->Pitch;
        }
    }
}

#endif