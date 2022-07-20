#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

struct FreeMemory
{
    void *Data;
    int Size;
};

struct MemoryManager
{
    char* Next;
    
    FreeMemory Free[100];
    int FreeIndex = 0;
    
    HANDLE Mutex;
    //u32 Mutex;
};

global_variable MemoryManager Manager;

internal void* qalloc(void* newM, int size);
internal void* qalloc(int size);

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
    //MemoryCopy(Manager.Next, &size, sizeof(int));
    //Manager.Next += sizeof(int);
    void *r = 0;
    //switch(WaitForSingleObject(Manager.Mutex, INFINITE))
    //{
    //case WAIT_OBJECT_0: _try 
    //{
    MemoryCopy(Manager.Next, newM, size);
    r = (void*)Manager.Next;
    Manager.Next += size;
    //}
    //_finally{if(!ReleaseMutex(Manager.Mutex)){}}break;case WAIT_ABANDONED:return false;
    //}
    return r;
}

internal void*
qalloc(int size)
{
    //MemoryCopy(Manager.Next, &size, sizeof(int));
    //Manager.Next += sizeof(int);
    
    void* r = 0;
    //switch(WaitForSingleObject(Manager.Mutex, INFINITE))
    //{
    //case WAIT_OBJECT_0: _try 
    //{
    r = Manager.Next;
    Manager.Next += size;
    //}
    //_finally{if(!ReleaseMutex(Manager.Mutex)){}}break;case WAIT_ABANDONED:return false;
    //}
    memset(r, 0, size);
    return r;
}

internal void
dalloc(void* Storage)
{
    //void* i = (char*)Storage - sizeof(int);
    //int Size = *(int*)i;
    
    //Manager.Next -= Size;
    //memset(i, 0, sizeof(int));
    //memset(Storage, 0, Size);
    
    /*
    Manager.Free[Manager.FreeIndex].Data = i;
    Manager.Free[Manager.FreeIndex].Size = Size;
    Manager.FreeIndex++;
    
    if (Manager.FreeIndex > 99) {
        for (int i = 0; i < Manager.FreeIndex; i++) {
            void* NextMemory = Manager.Free[i].Data + Manager.Free[i].Size;
            
        }
    }
*/
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

#define MallocCopy(t, d) ((t*)MallocCpy((void*)&d, sizeof t))

#define Qalloc(t) ((t*)qalloc(sizeof(t)))

#endif //MEMORYMANAGER_H
