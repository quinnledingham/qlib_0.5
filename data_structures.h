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
        fprintf(stderr, "ERROR: Cannot open file %s.\n", FileName);
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

inline bool Equal(char *c1, const char* c2)
{
    return Equal((const char*)c1, c2);
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

const char* Insert(const char* Source, int InsertPosition, const char* Add)
{
    int AddLength = GetLength(Add);
    int SourceLength = GetLength(Source);
    int Size =  SourceLength + AddLength + 1;
    char* Result = (char*)qalloc(Size);
    
    int SourceIndex = 0;
    int AddIndex = 0;
    int ResultIndex = 0;
    while (ResultIndex < Size-1) {
        if (SourceIndex < InsertPosition) {
            Result[ResultIndex] = Source[SourceIndex];
            SourceIndex++;
        }
        else if (SourceIndex >= InsertPosition) {
            if (AddIndex < AddLength) {
                Result[ResultIndex] = Add[AddIndex];
                AddIndex++;
            }
            else {
                Result[ResultIndex] = Source[SourceIndex];
                SourceIndex++;
            }
        }
        ResultIndex++;
    }
    Result[Size-1] = 0;
    
    return Result;
}

const char* RemoveAt(const char* Source, int RemovePosition)
{
    int Size = GetLength(Source);
    char* Result = (char*)qalloc(Size);
    
    int SourceIndex = 0;
    int ResultIndex = 0;
    while (ResultIndex < Size-1) {
        if (SourceIndex < RemovePosition) {
            Result[ResultIndex] = Source[SourceIndex];
            ResultIndex++;
        }
        else if (SourceIndex > RemovePosition) {
            Result[ResultIndex] = Source[SourceIndex];
            ResultIndex++;
        }
        SourceIndex++;
    }
    Result[Size - 1] = 0;
    
    return Result;
}

struct strinq
{
    char *Data;
    int Length;
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
#if QLIB_INTERNAL
    //switch(WaitForSingleObject(GlobalDebugBuffer.Mutex, INFINITE))
    //{
    //case WAIT_OBJECT_0: _try 
    //{
    Assert(GlobalDebugBuffer.Size + Output.Length < GlobalDebugBuffer.MaxSize);
    int i = 0;
    
    // Overwrite end of sting char
    if (GlobalDebugBuffer.Size > 0)
        i = -1;
    
    int j = 0;
    while(j <= Output.Length && i < 100) {
        GlobalDebugBuffer.Next[i] = Output.Data[j];
        i++;
        j++;
    }
    GlobalDebugBuffer.Size += Output.Length;
    GlobalDebugBuffer.Next = GlobalDebugBuffer.Next + i;
    //}
    //_finally{if(!ReleaseMutex(GlobalDebugBuffer.Mutex)){}}break;case WAIT_ABANDONED:return false;
    //}
#endif
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
    
    T& operator[](unsigned int index);
    unsigned int GetSize();
    void Resize(int n, const T& copy);
    void push_back(const T& NewData);
    void *GetData();
};


// DynArray

template<typename T>
T& DynArray<T>::operator[](unsigned int index)
{
    T* i = Data + (Size * sizeof(T));
    return *i;
}

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

// pair_int_string
struct pair_int_string
{
    int Int;
    char* String;
};
int GetInt(pair_int_string *IDs, int NumOf, const char *String)
{
    for (int i = 0; i < NumOf; i++) {
        if (Equal(IDs[i].String, String))
        {
            return IDs[i].Int;
        }
    }
    return -1;
}


// End of pair_int_string

// linked_list

struct linked_list_node
{
    void *Data;
    linked_list_node *Next;
};

struct linked_list
{
    u32 NodeCount;
    linked_list_node *I;
    linked_list_node *Head;
};

internal void
LinkedListAddNode(linked_list *List, void *Data)
{
    linked_list_node *Node = (linked_list_node*)qalloc(sizeof(linked_list_node));
    Node->Data = Data;
    if (List->Head != 0) {
        linked_list_node *Last = List->Head;
        for (u32 i = 1; i < List->NodeCount; i++)
            Last = Last->Next;
        Last->Next = Node;
    }
    else {
        List->Head = Node;
    }
    
    List->NodeCount++;
}

internal void*
LinkedListIGetNext(linked_list *List)
{
    linked_list_node *Ret = List->I;
    if (List->I->Next != 0)
        List->I = List->I->Next;
    else
        List->I = List->Head;
    
    return Ret->Data;
    
}
inline void LinkedListIReset(linked_list *List) { List->I = List->Head; }

// End of linked_list

#endif //DATA_STRUCTURES_H
