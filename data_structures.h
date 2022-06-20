#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#ifndef MEMORYMANAGER_H
#pragma message ("data_structures.h requires memorymanager.h")
#endif

// entire_file
struct entire_file 
{
    u32 ContentsSize;
    void *Contents;
};

entire_file
ReadEntireFile(const char*FileName)
{
    entire_file Result = {};
    
    FILE *In = fopen(FileName, "rb");
    if(In) {
        fseek(In, 0, SEEK_END);
        Result.ContentsSize = ftell(In);
        fseek(In, 0, SEEK_SET);
        
        Result.Contents = malloc(Result.ContentsSize);
        fread(Result.Contents, Result.ContentsSize, 1, In);
        fclose(In);
    }
    else {
        //printf("ERROR: Cannot open file %s.\n", FileName);
    }
    
    return(Result);
}

inline void
DestroyEntireFile(entire_file &F) { free(F.Contents); }
// End of entire_file

// strinq
inline int GetLength(const char* c)
{
    if (c == 0)
        return 0;
    
    int i = 0;
    while(c[i] != 0)
        i++;
    
    return i;
};

inline int GetLength(int i)
{
    int count = 0;
    int T = i;
    while((T = T / 10) != 0)
        count++;
    
    count++;
    return count;
}

inline bool Equal(const char *c1, const char* c2)
{
    int i = 0;
    while(c1[i] != 0 && c2[i] != 0) {
        if (c1[i] != c2[i])
            return false;
        i++;
    }
    
    if (c1[i] == c2[i])
        return true;
    else
        return false;
}

inline char* IntToString(int Value)
{
    // TODO(quinn): Implement my own int to string? 
    // https://guide.handmadehero.org/code/day328/ casey does do it
    
    char Temp[10];
    sprintf(Temp, "%d", Value);
    char* Result = (char*)qalloc(Temp, 10);
    
    return Result;
}

char* Concat(char* Source, char* Add)
{
    int Size = GetLength(Source) + GetLength(Add) + 1;
    char* Result = (char*)qalloc(Source, Size);
    int i = GetLength(Source);
    int j = 0;
    while(Add[j] != 0) {
        Result[i] = Add[j];
        i++;
        j++;
    }
    Result[i] = 0;
    return Result;
}

struct strinq
{
    char *Data;
    int Length;
    
    real32 DimScale;
    v2 Dim;
};
inline void DestroyStrinq(strinq *ToBeDestroyed) { free(ToBeDestroyed->Data); }

strinq NewStrinq(const char *InputChar)
{
    strinq NewS = {};
    int InputCharLength = GetLength(InputChar);
    int TotalSize = (InputCharLength * sizeof(char)) + 1;
    
    NewS.Data = (char*)MallocClear(TotalSize);
    
    for (int i = 0; i < InputCharLength; i++)
        NewS.Data[i] = InputChar[i];
    NewS.Length = InputCharLength;
    NewS.Data[NewS.Length] = 0;
    
    return NewS;
    //return MallocCopy(strinq, NewS);
}
inline strinq S() { return NewStrinq(""); }

strinq NewStrinq(entire_file *File)
{
    strinq NewS = {};
    int InputCharLength = File->ContentsSize;
    int TotalSize = InputCharLength * sizeof(char) + 1;
    
    NewS.Data = (char*)MallocClear(TotalSize);
    
    const char* InputChar = (char*)File->Contents;
    for (int i = 0; i < InputCharLength; i++)
        NewS.Data[i] = InputChar[i];
    
    NewS.Length = InputCharLength;
    NewS.Data[NewS.Length] = 0;
    
    return NewS;
    //return MallocCopy(strinq, NewS);
}

strinq IntToStrinq(int Value)
{
    strinq IntStrinq = {};
    int Digits = GetLength(Value);
    IntStrinq.Data = (char*)MallocClear(Digits + 1);
    
    // Help will equal 10s factor
    int Help = 0;
    for (int i = 0; i < Digits; i++) {
        if (Help == 0)
            Help += 1;
        else
            Help = Help * 10;
    }
    
    for (int i = 0; i < Digits; i++) {
        IntStrinq.Data[i] = ((char)(Value / Help)) + '0';
        Value = Value - ((Value / Help) * Help);
        Help = Help / 10;
    }
    
    IntStrinq.Data[Digits] = 0;
    IntStrinq.Length = Digits;
    
    return IntStrinq;
    //return MallocCopy(strinq, NewS);
}

strinq operator+(const strinq& L, const strinq& R)
{
    strinq ReturnStrinq = {};
    
    int CombinedLength = L.Length + R.Length;
    int TotalCombinedSize = CombinedLength + 1;
    
    ReturnStrinq.Data = (char*)MallocClear(TotalCombinedSize);
    ReturnStrinq.Length = CombinedLength;
    
    int i = 0;
    for (int j = 0; j < L.Length; j++) {
        ReturnStrinq.Data[i] = L.Data[j];
        i++;
    }
    for (int j = 0; j < R.Length; j++) {
        ReturnStrinq.Data[i] = R.Data[j];
        i++;
    }
    
    ReturnStrinq.Data[ReturnStrinq.Length] = 0;
    
    free(L.Data);
    free(R.Data);
    
    return ReturnStrinq;
}

inline strinq operator+(const strinq &L, const char* R) { return (L + NewStrinq(R)); }
inline strinq operator+(const strinq &L, int R) { return (L + IntToStrinq(R)); }

bool operator==(const strinq& L, const strinq& R)
{
    int i = 0;
    while(L.Data[i] != 0 && R.Data[i] != 0) {
        if (L.Data[i] != R.Data[i])
            return false;
        i++;
    }
    
    if (L.Data[i] == 0 && R.Data[i] == 0)
        return true;
    
    return false;
}

bool PrintqDebug(strinq Output)
{
    switch(WaitForSingleObject(GlobalDebugBuffer.Mutex, INFINITE))
    {
        case WAIT_OBJECT_0: _try 
        {
            Assert(GlobalDebugBuffer.Size + Output.Length < GlobalDebugBuffer.MaxSize);
            int i = 0;
            int j = 0;
            while(j <= Output.Length && i < 100) {
                GlobalDebugBuffer.Next[i] = Output.Data[j];
                i++;
                j++;
            }
            GlobalDebugBuffer.Size += Output.Length;
            GlobalDebugBuffer.Next = GlobalDebugBuffer.Next + i;
        }
        _finally{if(!ReleaseMutex(GlobalDebugBuffer.Mutex)){}}break;case WAIT_ABANDONED:return false;
    }
    
    return true;
}

void PrintqDebug(const char* Output)
{
    strinq ToOutput = NewStrinq(Output);
    PrintqDebug(ToOutput);
    DestroyStrinq(&ToOutput);
}

int StrinqFind(const strinq& s, char f)
{
    int i = 0;
    while(s.Data[i] != 0) {
        if (s.Data[i] == f)
            return i;
        i++;
    }
    
    return -1;
}

void CopyBuffer(char* B1, const char* B2, int Bytes)
{
    for (int i = 0; i < Bytes; i++)
        B1[i] = B2[i];
}

int StrinqErase(strinq& s, int f)
{
    char* NewData;
    NewData = (char*)malloc(f * sizeof(char));
    
    CopyBuffer(NewData, s.Data, f);
    
    free(s.Data);
    s.Data = NewData;
    
    return 0;
}

int GetLength(strinq& s)
{
    int i = 0;
    while (s.Data[i] != 0)
        i++;
    
    return i;
}
// end of strinq

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


// DynArray

template<typename T>
unsigned int DynArray<T>::GetSize()
{
    return Size;
}

template<typename T>
void DynArray<T>::Resize(int n, const T& copy)
{
    for (int i = 0; i < n; i++)
    {
        push_back(copy);
    }
}

template<typename T>
void DynArray<T>::push_back(const T& NewData)
{
    if (Data == 0)
    {
        Data = (T*)malloc(sizeof(T) * 10);
        memset(Data, 0, sizeof(T) * 10);
        MaxSize = 10;
    }
    // TODO(not-set): Make this work correctly
    else if (Size >= MaxSize)
    {
        T* Next = Data + (MaxSize * sizeof(T));
        Next = (T*)malloc(sizeof(T) * 10);
        memset(Next, 0, sizeof(T) * 10);
        MaxSize += 10;
    }
    
    T* Next = (T*)((char*)Data + (Size * sizeof(T)));
    memcpy((void*)Next, (void*)&NewData, sizeof(T));
    Size++;
}

template<typename T> 
void *DynArray<T>::GetData() { return Data; }
// End of DynArray

// Map
struct MapEntry
{
    void* Key;
    void* Value;
};

#define MAP_SIZE 100
struct Map
{
    int32 Size = MAP_SIZE;
    MapEntry Values[MAP_SIZE] = {0};
    int Next = 0;
    
    void Init();
    int& operator[](char* i);
    int& operator[](strinq& i);
    unsigned int MapFind(const strinq &k);
    unsigned int MapFind(const char* k);
};

void Map::Init()
{
    memset(Values, 0, sizeof(MapEntry) * MAP_SIZE);
}

int& Map::operator[](char* i)
{
    int cLength = GetLength(i);
    Values[Next].Key = qalloc(cLength + 1);
    Values[Next].Value = qalloc(sizeof(int));
    memcpy(Values[Next].Key, (void*)i, cLength + 1);
    return *((int*)Values[Next++].Value);
}

int& Map::operator[](strinq& i)
{
    Values[Next].Key = qalloc(i.Length + 1);
    Values[Next].Value = qalloc(sizeof(int));
    memcpy(Values[Next].Key, (void*)i.Data, i.Length + 1);
    return *((int*)Values[Next++].Value);
}

unsigned int 
Map::MapFind(const strinq &k)
{
    for(int i = 0; i < Next; i++) {
        strinq Key = NewStrinq(((char*)Values[i].Key));
        if (Key == k) {
            unsigned int* v = (unsigned int*)Values[i].Value;
            DestroyStrinq(&Key);
            return *v;
        } 
        DestroyStrinq(&Key);
    }
    return 0;
}

unsigned int
Map::MapFind(const char* k)
{
    for(int i = 0; i < Next; i++)
    {
        if (Equal(k, (char*)Values[i].Key))
        {
            unsigned int* v = (unsigned int*)Values[i].Value;
            return *v;
        } 
    }
    return 0;
}

// End of Map

// Arr
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
// End of Arr

#endif //DATA_STRUCTURES_H
