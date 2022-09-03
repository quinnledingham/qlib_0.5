#ifndef STRING_H
#define STRING_H

#define MAX_STRING_LENGTH 200
struct string
{
    u32 Length;
    char Chars[MAX_STRING_LENGTH];
    
    char operator[](u32 i);
    string();
    string(const char* Text);
};

string::string() 
{
    string *String = this;
    memset(String, 0, sizeof(string));
}

string::string(const char* Text)
{
    string *String = this;
    *String = string();
    
    u32 TextIndex = 0;
    while (Text[TextIndex] != 0)
    {
        String->Chars[TextIndex] = Text[TextIndex];
        String->Length++;
        TextIndex++;
    }
}

char string::operator[](u32 i)
{
    Assert(i < MAX_STRING_LENGTH);
    string *String = this;
    return String->Chars[i];
}

string IntToString(u32 Value)
{
    string New = {};
    
    u32 Digits = GetLength(Value);
    
    // Help will equal 10s factor
    int Help = 0;
    for (u32 i = 0; i < Digits; i++) {
        if (Help == 0)
            Help += 1;
        else
            Help = Help * 10;
    }
    
    for (u32 i = 0; i < Digits; i++) {
        New.Chars[i] = ((char)(Value / Help) + '0');
        Value = Value - ((Value / Help) * Help);
        Help = Help / 10;
    }
    
    New.Chars[Digits] = 0;
    New.Length = Digits;
    
    return New;
}

string operator+(const string &L, const string &R)
{
    Assert((L.Length + R.Length) < MAX_STRING_LENGTH);
    
    string New = {};
    u32 NewIndex = 0;
    New.Length = L.Length + R.Length;
    
    for (u32 i = 0; i < L.Length; i++)
        New.Chars[NewIndex++] = L.Chars[i];
    for (u32 i = 0; i < R.Length; i++)
        New.Chars[NewIndex++] = R.Chars[i];
    
    return New;
}
inline string operator+(const string &L, u32 R) { return (L + IntToString(R)); }
inline string operator+(const string &L, const char* R) { return (L + string(R)); }

#if QLIB_OPENGL

inline string operator+(const string &L, const GLubyte *R) { return (L + string((char*)R)); }

#endif

inline void StringLog(string String) { Log(String.Chars); }
inline void Log(string String) { StringLog(String); }

#endif //STRING_H