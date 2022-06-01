#include "osThread.h"

#include <assert.h>

OSThread::OSThread(OSProc pp)
{
    proc  =pp;
    sem   =(void*)CreateSemaphore(NULL,0,1,NULL);
    handle=0;
    if(proc==NULL)
        proc = (OSProc)OSThread::RunProc;

    running= false;

    assert(sem!=NULL);
    assert(proc!=NULL);
}

OSThread::~OSThread()
{
    assert(sem!=NULL);
    assert(proc!=NULL);

    Term();
    CloseHandle((HANDLE)sem);
}

bool OSThread::Load(size_t stackSize)
{
    if(proc && !running)
    {
        DWORD pthread;

        ResetEvent(sem);
        running = true;

        handle = (unsigned long)CreateThread(NULL,
            stackSize,(LPTHREAD_START_ROUTINE)proc,this,NULL,&pthread);
        if(handle==0)
        {
            running = false;
        }

    }

    return running;
}

bool OSThread::Term(bool bSoftly)
{
    if(handle)
    {
        if(bSoftly)
        {
            ResetEvent(sem);
            WaitForSingleObject((HANDLE)sem,500);
        }
        else
            TerminateThread((HANDLE)handle,0);

        CloseHandle((HANDLE)handle);
    }

    running = false;

    handle  = 0;

    return true;
}

bool OSThread::Wait()
{
    if(handle && sem)
    {
        WaitForSingleObject((HANDLE)sem,INFINITE);
    }
    return false;
}

bool OSThread::Proc()
{
    bool retval;

    assert(sem!=NULL);
    assert(handle!=NULL);

    retval = OnLoad();
    if (retval)
    {
        while(running && Main());
    }

    /*must be clear*/
    handle = NULL;
    OnTerm();

    ReleaseSemaphore((HANDLE)sem, 1, NULL);

    return retval;
}

bool OSThread::Main()
{

    return false;
}

bool OSThread::SetSched(Priority  priority)
{
    if(handle)
        return OSThread::SetPriority(priority,handle);
    else
        return false;
}

bool OSThread::GetSched(Priority &priority)
{
    if(handle)
        return OSThread::GetPriority(priority,handle);
    else
        return false;
}

bool OSThread::SetPriority(Priority  priority,unsigned long handle)
{
    if(priority<PriorityMin)
        priority = PriorityMin;
    if(priority>PriorityMax)
        priority = PriorityMax;

    if(handle==0)
        handle = (unsigned long)GetCurrentThread();

    priority = (Priority)((priority*(THREAD_BASE_PRIORITY_LOWRT-THREAD_PRIORITY_LOWEST))/PriorityMax);

    SetThreadPriority((HANDLE)handle,priority);

    return true;
}

bool OSThread::GetPriority(Priority &priority,unsigned long handle)
{
    if(handle==0)
        handle = (unsigned long)GetCurrentThread();

    priority = (Priority)GetThreadPriority((HANDLE)handle);
    priority = (Priority)(PriorityMin+(PriorityMax*priority)/(THREAD_PRIORITY_HIGHEST));

    return true;
}

bool OSThread::SetStackSize(unsigned long  size)
{

    return false;
}

bool OSThread::GetStackSize(unsigned long &size)
{

    return false;
}
bool OSThread::SetAffinity(unsigned  mask)
{
    HANDLE handle = GetCurrentThread();

    SetThreadAffinityMask(handle,mask);
    return true;
}
bool OSThread::GetAffinity(unsigned &mask)
{
    mask = 0;
    return 0;
}

void OSThread::MSleep(int msecond)
{
    if (msecond<=0)
        return;

#if 0
#define WAITABLETIMER_MS_INTERVAL (1000*10)
    HANDLE tmr = NULL;
    LARGE_INTEGER to;

    if((tmr=CreateWaitableTimer(NULL, FALSE, NULL)) == NULL)
    {
        Sleep(msecond);
        return;
    }

    to.QuadPart = (long long)(-(msecond*WAITABLETIMER_MS_INTERVAL));
    if(!SetWaitableTimer(tmr, &to, 0, NULL, NULL, FALSE))
    {
        Sleep(msecond);
        CloseHandle(tmr);
        return;
    }

    if(WaitForSingleObject(tmr, INFINITE) != WAIT_OBJECT_0)
    {
        Sleep(msecond);
        CloseHandle(tmr);
        return;
    }

    CloseHandle(tmr);
#else
    Sleep(msecond);
#endif
}

void *OSThread::RunProc(OSThread *self)
{
    if(self)
        self->Proc();
    return NULL;
}