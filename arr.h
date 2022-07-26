#ifndef ARR_H
#define ARR_H

struct Arr
{
    void *Data;
    int Size;
    int MaxSize;
    int TypeSize;
    
    // Number of indices. Size of indices.
    void Init(int ms, int ts)
    {
        MaxSize = ms;
        TypeSize = ts;
        
        if (Data == 0)
            Data = qalloc(ms * ts);
    }
    
    void Del()
    {
        dalloc(Data);
    }
    
    void Push(void *NewData)
    {
        if (Size + 1 >= MaxSize)
            return;
        
        char *Cursor = (char*)Data;
        Cursor += (Size * TypeSize);
        memcpy(Cursor, NewData, TypeSize);
        Size++;
    }
    
    void PopFront()
    {
        char *Cursor = (char*)Data;
        Cursor += TypeSize;
        memcpy(Data, Cursor, Size * TypeSize);
        Size--;
    }
    
    void Clear()
    {
        memset(Data, 0, MaxSize * TypeSize);
        Size = 0;
    }
    
    void* operator[](int i)
    {
        char *Cursor = (char*)Data;
        Cursor += (i * TypeSize);
        return (void*)Cursor;
    }
    
    void* Get(int i)
    {
        char *Cursor = (char*)Data;
        Cursor += (i * TypeSize);
        return (void*)Cursor;
    }
    
    void* Last()
    {
        char *Cursor = (char*)Data;
        Cursor += ((Size - 1) * TypeSize);
        return (void*)Cursor;
    }
};

struct arr
{
    bool Initialized;
    void *Data;
    int CurrentSize;
    int MaxSize;
    int TypeSize;
    
    void* operator[](int i)
    {
        char *Cursor = (char*)Data;
        Cursor += (i * TypeSize);
        return (void*)Cursor;
    }
};

internal void
ArrInit(arr *Arr, int MaxSize, int TypeSize)
{
    Arr->Initialized = true;
    Arr->MaxSize = MaxSize;
    Arr->TypeSize = TypeSize;
    Arr->CurrentSize = 0;
    
    if (Arr->Data == 0)
        Arr->Data = qalloc(Arr->MaxSize * Arr->TypeSize);
    else
        memset(Arr->Data, 0, Arr->MaxSize * Arr->TypeSize);
}

internal void*
ArrPush(arr *Arr, void *NewData, int TypeSize)
{
    Assert(Arr->TypeSize == TypeSize);
    if (Arr->CurrentSize + 1 >= Arr->MaxSize)
        return 0;
    
    char *Cursor = (char*)Arr->Data;
    Cursor += (Arr->CurrentSize * Arr->TypeSize);
    memcpy(Cursor, NewData, Arr->TypeSize);
    Arr->CurrentSize++;
    
    return (void*)Cursor;
}

internal void
ArrPushPointer(arr *Arr, void *Pointer, int TypeSize)
{
    Assert(Arr->TypeSize == TypeSize);
    if (Arr->CurrentSize + 1 >= Arr->MaxSize)
        return;
    char *Cursor = (char*)Arr->Data;
    Cursor += (Arr->CurrentSize * Arr->TypeSize);
    memcpy(Cursor, &Pointer, Arr->TypeSize);
    Arr->CurrentSize++;
}

internal void
ArrClear(arr *Arr, int TypeSize)
{
    Assert(Arr->TypeSize == TypeSize);
    memset(Arr->Data, 0, Arr->MaxSize * Arr->TypeSize);
    Arr->CurrentSize = 0;
}

internal void*
ArrGetNext(arr *Arr, int TypeSize)
{
    Assert(Arr->TypeSize == TypeSize);
    
    char *Ret = (char*)Arr->Data;
    Ret += (Arr->CurrentSize * TypeSize);
    Arr->CurrentSize++;
    return (void*)Ret;
}
#define GetNext(a, t) ((t*)ArrGetNext(&a, sizeof(t)))
#define MemCpy(d, s, t) (memcpy(d, (void*)s, sizeof(t)))

// Adds new space if return pointer is null. replaces mem if return pointer is not null
internal void*
ArrUseNext(arr *Arr, void *Data, void *Return, int TypeSize)
{
    Assert(Arr->Initialized);
    Assert(Arr->TypeSize == TypeSize);
    
    if (Return == 0)
        Return = ArrPush(Arr, Data, TypeSize);
    else
        MemCpy(Return, Data, TypeSize);
    
    return Return;
}
#define ArrUseNext(a, p, r, t) ((t*)ArrUseNext(&a, (void*)p, (void*)r,  sizeof(t)))

#endif //ARR_H
