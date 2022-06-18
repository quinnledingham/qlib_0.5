#ifndef STRINQ_H
#define STRINQ_H

#include "memorymanager.h"
#include "platform.h"

struct Strinq
{
    char* Data;
    int Length;
};

char* GetData(const Strinq& q);
void NewStrinq(Strinq& NewS, const char* InputChar);
void NewStrinq(Strinq& NewS, entire_file *File);
Strinq NewStrinq(const char* InputChar);
Strinq NewStrinq(entire_file* File);
void DestroyStrinq(Strinq& ToBeDestroyed);
bool operator==(const Strinq& L, const Strinq& R);
void CopyBuffer(char* B1, const char* B2, int Bytes);

int Length(const char* c)
{
    if (c == 0)
        return 0;
    
    int i = 0;
    while(c[i] != 0)
        i++;
    
    return i;
};

int IntLength(int i)
{
    int Count = 0;
    int T = i;
    while((T = T / 10) != 0)
        Count++;
    
    Count++;
    return Count;
}

bool Compare(const char *c1, const char* c2)
{
    int i = 0;
    while(c1[i] != 0 && c2[i] != 0) {
        if (c1[i] != c2[i])
            return false;
        
        i++;
    }
    
    return true;
}

char* IntToString(int Value)
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
    int Size = Length(Source) + Length(Add) + 1;
    char* Result = (char*)qalloc(Source, Size);
    int i = Length(Source);
    int j = 0;
    while(Add[j] != 0) {
        Result[i] = Add[j];
        i++;
        j++;
    }
    Result[i] = 0;
    return Result;
}

#endif //STRINQ_H
