#define OUTPUTBUFFER_SIZE 1000

struct debug_buffer
{
    int Size = OUTPUTBUFFER_SIZE;
    char Data[OUTPUTBUFFER_SIZE];
    char* Next;
};

debug_buffer debug = {};