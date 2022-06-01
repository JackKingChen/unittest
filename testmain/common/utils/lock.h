
#pragma once;
#include <Windows.h>
class Mutex
{
public:
    enum Type
    {
        DEFAULT,
        RECURSIVE
    };

    explicit Mutex (Type = RECURSIVE) { InitializeCriticalSection (&cs); };
    ~Mutex () { DeleteCriticalSection (&cs); } ;

    void lock () const { EnterCriticalSection (&cs); };
    void unlock () const { LeaveCriticalSection (&cs); };

private:
    mutable CRITICAL_SECTION cs;
};
