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
    
    MemoryCopy(Manager.Next, newM, size);
    void* r = (void*)Manager.Next;
    Manager.Next += size;
    
    return r;
}

internal void*
qalloc(int size)
{
    //MemoryCopy(Manager.Next, &size, sizeof(int));
    //Manager.Next += sizeof(int);
    
    void* r = Manager.Next;
    Manager.Next += size;
    
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


#endif //MEMORYMANAGER_H
