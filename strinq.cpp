void NewStrinq(Strinq& NewS, const char* InputChar)
{
    NewS = {};
    
    int InputCharLength = CharGetLength(InputChar);
    int TotalSize = InputCharLength * sizeof(char) + 1;
    
    NewS.Data = (char*)malloc(TotalSize);
    memset(NewS.Data, 0, TotalSize);
    
    for (int i = 0; i < InputCharLength; i++)
    {
        NewS.Data[i] = InputChar[i];
    }
    
    NewS.Length = InputCharLength;
    NewS.Data[NewS.Length] = 0;
}

void NewStrinq(Strinq& NewS, entire_file *File)
{
    NewS = {};
    
    int InputCharLength = File->ContentsSize;
    int TotalSize = InputCharLength * sizeof(char) + 1;
    
    NewS.Data = (char*)malloc(TotalSize);
    memset(NewS.Data, 0, TotalSize);
    
    const char* InputChar = (char*)File->Contents;
    
    for (int i = 0; i < InputCharLength; i++)
    {
        NewS.Data[i] = InputChar[i];
    }
    
    NewS.Length = InputCharLength;
    NewS.Data[NewS.Length] = 0;
}

Strinq NewStrinq(const char* InputChar)
{
    Strinq NewS = {};
    NewStrinq(NewS, InputChar);
    return NewS;
}

Strinq NewStrinq(entire_file* File)
{
    Strinq NewS = {};
    NewStrinq(NewS, File);
    return NewS;
}

void DestroyStrinq(Strinq& ToBeDestroyed)
{
    free(ToBeDestroyed.Data);
}

char* GetData(const Strinq& q)
{
    return q.Data;
}

void IntToStrinq(Strinq& IntStrinq, int Value)
{
    int Digits = IntGetLength(Value);
    
    IntStrinq.Data = (char*)malloc(Digits + 1);
    memset(IntStrinq.Data, 0, Digits + 1);
    
    // Help will equal 10s factor
    int Help = 0;
    for (int i = 0; i < Digits; i++)
    {
        if (Help == 0)
        {
            Help += 1;
        }
        else
        {
            Help = Help * 10;
        }
    }
    
    for (int i = 0; i < Digits; i++)
    {
        IntStrinq.Data[i] = ((char)(Value / Help)) + '0';
        Value = Value - ((Value / Help) * Help);
        Help = Help / 10;
    }
    
    IntStrinq.Data[Digits] = 0;
    IntStrinq.Length = Digits;
}

Strinq S()
{
    Strinq StartStrinq = {};
    NewStrinq(StartStrinq, "");
    return StartStrinq;
}

Strinq operator+(const Strinq& L, const Strinq& R)
{
    Strinq ReturnStrinq = {};
    
    int CombinedLength = L.Length + R.Length;
    int TotalCombinedSize = CombinedLength + 1;
    ReturnStrinq.Data = (char*)malloc(TotalCombinedSize);
    memset(ReturnStrinq.Data, 0, TotalCombinedSize);
    
    ReturnStrinq.Length = CombinedLength;
    
    int i = 0;
    for (int j = 0; j < L.Length; j++)
    {
        ReturnStrinq.Data[i] = L.Data[j];
        i++;
    }
    for (int j = 0; j < R.Length; j++)
    {
        ReturnStrinq.Data[i] = R.Data[j];
        i++;
    }
    
    ReturnStrinq.Data[ReturnStrinq.Length] = 0;
    
    free(L.Data);
    free(R.Data);
    
    return ReturnStrinq;
}

Strinq operator+(const Strinq &L, char* R)
{
    Strinq SR = {};
    NewStrinq(SR, R);
    return (L + SR);
}

Strinq operator+(const Strinq &L, const char* R)
{
    Strinq SR = {};
    NewStrinq(SR, R);
    return (L + SR);
}


Strinq operator+(const Strinq &L, int R)
{
    Strinq SR = {};
    IntToStrinq(SR, R);
    return (L + SR);
}
/*
Strinq operator+(const Strinq &L, bool R)
{
    Strinq SR = {};
    if (R == true)
    {
        NewStrinq(SR, "true");
    }
    else if (R == false)
    {
        NewStrinq(SR, "false");
    }
    return (L + SR);
}
*/
bool operator==(const Strinq& L, const Strinq& R)
{
    int i = 0;
    while(L.Data[i] != 0 && R.Data[i] != 0)
    {
        if (L.Data[i] != R.Data[i])
        {
            return false;
        }
        i++;
    }
    
    if (L.Data[i] == 0 && R.Data[i] == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void PrintqDebug(Strinq Output)
{
    int i = 0;
    int j = 0;
    while(j <= Output.Length && i < 100)
    {
        GlobalDebugBuffer.Next[i] = Output.Data[j];
        i++;
        j++;
    }
    
    GlobalDebugBuffer.Next = GlobalDebugBuffer.Next + i;
}

void PrintqDebug(char* Output)
{
    Strinq ToOutput = {};
    NewStrinq(ToOutput, Output);
    PrintqDebug(ToOutput);
    DestroyStrinq(ToOutput);
}

int StrinqFind(const Strinq& s, char f)
{
    int i = 0;
    while(s.Data[i] != 0)
    {
        if (s.Data[i] == f)
        {
            return i;
        }
        i++;
    }
    
    return -1;
}

void CopyBuffer(char* B1, const char* B2, int Bytes)
{
    for (int i = 0; i < Bytes; i++)
    {
        B1[i] = B2[i];
    }
}

int StrinqErase(Strinq& s, int f)
{
    char* NewData;
    NewData = (char*)malloc(f * sizeof(char));
    
    CopyBuffer(NewData, s.Data, f);
    
    free(s.Data);
    s.Data = NewData;
    
    return 0;
}

int Length(Strinq& s)
{
    int i = 0;
    while (s.Data[i] != 0)
    {
        i++;
    }
    return i;
}

bool32 Equal(const char* r, const char* l)
{
    int i = 0;
    while(r[i] != 0 && l[i] != 0)
    {
        if (r[i] != l[i])
        {
            return false;
        }
        i++;
    }
    if (r[i] == l[i])
    {
        return true;
    }
    else
    {
        return false;
    }
}