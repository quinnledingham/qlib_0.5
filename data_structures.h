#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

template<typename T>
struct DynArray
{
    T* Data;
    unsigned int Size;
    unsigned int MaxSize;
    
    unsigned int GetSize();
    void Resize(int n, const T& copy);
    void push_back(const T& NewData);
    void *GetData();
};

struct MapEntry
{
    void* Key;
    void* Value;
};

struct entire_file 
{
    u32 ContentsSize;
    void *Contents;
};

#include "strinq.h"

entire_file ReadEntireFile(const char* FileName);
void DestroyEntireFile(entire_file &F);

#define MAP_SIZE 100
struct Map
{
    int32 Size = MAP_SIZE;
    MapEntry Values[MAP_SIZE] = {0};
    int Next = 0;
    
    void Init();
    int& operator[](char* i);
    int& operator[](Strinq& i);
    unsigned int MapFind(const Strinq &k);
    unsigned int MapFind(const char* k);
};

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
    
    void* Last()
    {
        char *Cursor = (char*)Data;
        Cursor += ((Size - 1) * TypeSize);
        return (void*)Cursor;
    }
};

#endif //DATA_STRUCTURES_H
