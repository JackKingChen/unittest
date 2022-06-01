#pragma once

#include <Windows.h>

class OSThread
{
public:
        /*
        * setting
        */
        enum Priority
        {
            PriorityMin = 0,
            PriorityMax = 100,
        };

        /*
        * proc type
        */
        typedef void *(*OSProc)(void*);

    public:

        OSThread(OSProc pp=0);
        virtual ~OSThread();

        virtual bool Load(size_t stackSize=0);
        virtual bool Term(bool bSoftly=true);
        virtual bool Wait();
        virtual bool Proc();
        virtual bool Main();

        virtual bool OnLoad(){ return true; };
        virtual void OnTerm(){ running = false; };

        virtual bool SetSched(Priority  priority);
        virtual bool GetSched(Priority &priority);

        virtual bool isRunning(void) { return running; };

    public:
        static bool SetPriority(Priority  priority,unsigned long handle=0);
        static bool GetPriority(Priority &priority,unsigned long handle=0);
        static bool SetStackSize(unsigned long  size);
        static bool GetStackSize(unsigned long &size);
        static bool SetAffinity(unsigned  mask=0x01);
        static bool GetAffinity(unsigned &mask);

        static void*RunProc(OSThread *self);

        static void MSleep(int msecond);

    protected:
        OSProc        proc;
        volatile bool running;
        void         *sem;
        unsigned long handle;
};
