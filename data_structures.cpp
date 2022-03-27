
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
void *DynArray<T>::GetData()
{
    return Data;
}


// entire_file

entire_file
ReadEntireFile(const char*FileName)
{
    entire_file Result = {};
    
    FILE *In = fopen(FileName, "rb");
    if(In)
    {
        fseek(In, 0, SEEK_END);
        Result.ContentsSize = ftell(In);
        fseek(In, 0, SEEK_SET);
        
        Result.Contents = malloc(Result.ContentsSize);
        fread(Result.Contents, Result.ContentsSize, 1, In);
        fclose(In);
    }
    else
    {
        //printf("ERROR: Cannot open file %s.\n", FileName);
    }
    
    return(Result);
}

void
DestroyEntireFile(entire_file &F)
{
    free(F.Contents);
}


// Map

int& Map::operator[](char* i)
{
    int cLength = CharGetLength(i);
    Values[Next].Key = malloc(cLength + 1);
    Values[Next].Value = malloc(sizeof(int));
    memcpy(Values[Next].Key, (void*)i, cLength + 1);
    return *((int*)Values[Next++].Value);
}

int& Map::operator[](Strinq& i)
{
    Values[Next].Key = malloc(i.Length + 1);
    Values[Next].Value = malloc(sizeof(int));
    memcpy(Values[Next].Key, (void*)GetData(i), i.Length + 1);
    return *((int*)Values[Next++].Value);
}

unsigned int 
Map::MapFind(const Strinq &k)
{
    for(int i = 0; i < Next; i++)
    {
        Strinq Key = {};
        NewStrinq(Key, ((char*)Values[i].Key));
        if (Key == k)
        {
            unsigned int* v = (unsigned int*)Values[i].Value;
            
            DestroyStrinq(Key);
            
            return *v;
        } 
        
        DestroyStrinq(Key);
    }
    return 0;
}