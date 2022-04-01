#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

struct MemoryManager
{
    char* Next;
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
    MemoryCopy(Manager.Next, newM, size);
    void* r = (void*)Manager.Next;
    Manager.Next += size;
    return r;
}

internal void*
qalloc(int size)
{
    void* r = Manager.Next;
    Manager.Next += size;
    return r;
}


#endif //MEMORYMANAGER_H
