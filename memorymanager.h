#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

struct MemoryManager
{
    bool32 StartInitialized;
    char *Start;
    uint32 BytesUsed; // bytes from start
    
    char* Next;
};
global_variable MemoryManager Manager;

internal void
MemoryCopy(void* Dest, void* Source, int Size)
{
    char* CharDest = (char*)Dest;
    char* CharSource = (char*)Source;
    for (int i = 0; i < Size; i++)
    {
        CharDest[i] = CharSource[i];
    }
}

internal void*
qalloc(void* newM, int size)
{
    void *r = 0;
    
    MemoryCopy(Manager.Next, newM, size);
    r = (void*)Manager.Next;
    Manager.Next += size;
    
    
    Manager.BytesUsed += size;
    Assert(Manager.BytesUsed < Permanent_Storage_Size);
    
    return r;
}

internal void*
qalloc(int size)
{
    void* r = 0;
    
    r = Manager.Next;
    Manager.Next += size;
    
    memset(r, 0, size);
    
    Manager.BytesUsed += size;
    Assert(Manager.BytesUsed < Permanent_Storage_Size);
    
    return r;
}

// Function that allocates state at beginning of memory
inline void*
GetMemoryStart(int Size)
{
    if (!Manager.StartInitialized)
    {
        qalloc(Size);
        Manager.StartInitialized = true;
    }
    
    return Manager.Start;
}
#define MemStart(t) ((t*)GetMemoryStart(sizeof(t)))

internal void
dalloc(void* Storage)
{
    
}

inline void* MallocClear(int Size)
{
    void* Data = malloc(Size);
    memset(Data, 0, Size);
    return Data;
}
inline void* MallocCpy(void* Data, int Size)
{
    void* Mem = malloc(Size);
    memcpy(Mem, Data, Size);
    return Mem;
}
// Buffer Clear
inline char* BufferC(char* Buffer, int BufferSize)
{
    memset(Buffer, 0, BufferSize);
    return Buffer;
}
// Buffer Clear and Copy
inline char* BufferCC(char* Buffer, int BufferSize, void* DataToCopy, int DataToCopySize)
{
    memset(Buffer, 0, BufferSize);
    memcpy(Buffer, DataToCopy, DataToCopySize); 
    return Buffer;
}

#define Qalloc(t) ((t*)qalloc(sizeof(t)))

#endif //MEMORYMANAGER_H
