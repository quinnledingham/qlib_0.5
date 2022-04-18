#ifndef STRINQ_H
#define STRINQ_H

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

int CharGetLength(const char* c)
{
    if (c == 0)
    {
        return 0;
    }
    
    int i = 0;
    while(c[i] != 0)
    {
        i++;
    }
    
    return i;
};

int IntGetLength(int i)
{
    int Count = 0;
    int T = i;
    while((T = T / 10) != 0)
    {
        Count++;
    }
    Count++;
    return Count;
}

#endif //STRINQ_H
