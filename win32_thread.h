#ifndef WIN32_THREAD_H
#define WIN32_THREAD_H

#if COMPILER_MSVC
#define CompletePreviousWritesBeforeFutureWrites _WriteBarrier()
inline uint32 AtomicCompareExchangeUInt32(uint32 volatile *Value, uint32 New, uint32 Expected)
{
    uint32 Result = _InterlockedCompareExchange((long *)Value, New, Expected);
    
    return(Result);
}
#elif COMPILER_LLVM
#define CompletePreviousWritesBeforeFutureWrites asm volatile("" ::: "memory")
inline uint32 AtomicCompareExchangeUInt32(uint32 volatile *Value, uint32 New, uint32 Expected)
{
    uint32 Result = __sync_val_compare_and_swap(Value, Expected, New);
    
    return(Result);
}
#else
// TODO(casey): Need GCC/LLVM equivalents!
#endif

struct win32_thread_info
{
    int LogicalThreadIndex;
    platform_work_queue *Queue;
};

internal void
Win32AddEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    // TODO(casey): Switch to InterlockedCompareExchange eventually
    // so that any thread can add?
    uint32 NewNextEntryToWrite = (Queue->NextEntryToWrite + 1) % ArrayCount(Queue->Entries);
    Assert(NewNextEntryToWrite != Queue->NextEntryToRead);
    platform_work_queue_entry *Entry = Queue->Entries + Queue->NextEntryToWrite;
    Entry->Callback = Callback;
    Entry->Data = Data;
    ++Queue->CompletionGoal;
    _WriteBarrier();
    _mm_sfence();
    Queue->NextEntryToWrite = NewNextEntryToWrite;
    ReleaseSemaphore((HANDLE)Queue->SemaphoreHandle, 1, 0);
}
inline void
WorkQueueAddEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    Win32AddEntry(Queue, Callback, Data);
}

internal bool32
Win32DoNextWorkQueueEntry(platform_work_queue *Queue)
{
    bool32 WeShouldSleep = false;
    //Sleep(1);
    
    uint32 OriginalNextEntryToRead = Queue->NextEntryToRead;
    uint32 NewNextEntryToRead = (OriginalNextEntryToRead + 1) % ArrayCount(Queue->Entries);
    if(OriginalNextEntryToRead != Queue->NextEntryToWrite)
    {
        uint32 Index = InterlockedCompareExchange((LONG volatile *)&Queue->NextEntryToRead,
                                                  NewNextEntryToRead,
                                                  OriginalNextEntryToRead);
        if(Index == OriginalNextEntryToRead)
        {        
            platform_work_queue_entry Entry = Queue->Entries[Index];
            Entry.Callback(Queue, Entry.Data);
            InterlockedIncrement((LONG volatile *)&Queue->CompletionCount);
        }
    }
    else
    {
        WeShouldSleep = true;
    }
    
    return(WeShouldSleep);
}

internal void
Win32CompleteAllWork(platform_work_queue *Queue)
{
    while(Queue->CompletionGoal != Queue->CompletionCount)
    {
        Win32DoNextWorkQueueEntry(Queue);
    }
    
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}
inline void
WorkQueueCompleteAllWork(platform_work_queue *Queue)
{
    Win32CompleteAllWork(Queue);
}

DWORD WINAPI
ThreadProc(LPVOID lpParameter)
{
    win32_thread_info *ThreadInfo = (win32_thread_info *)lpParameter;
    
    for(;;)
    {
        if(Win32DoNextWorkQueueEntry(ThreadInfo->Queue))
        {
            WaitForSingleObjectEx((HANDLE)ThreadInfo->Queue->SemaphoreHandle, INFINITE, FALSE);
        }
    }
    
    //    return(0);
}

#endif //WIN32_THREAD_H
