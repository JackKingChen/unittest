
#include "mutex.h"

#include <exception>
#include <stdexcept>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <winbase.h>
#include <process.h>

typedef HANDLE           os_mutex_t;
typedef CRITICAL_SECTION os_fast_mutex_t;

#if defined(__DMC__)
extern "C" WINBASEAPI BOOL WINAPI InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION,DWORD);
#endif
#if defined(_MSC_VER) && _MSC_VER < 1300
extern "C" WINBASEAPI BOOL WINAPI InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION,DWORD);
extern "C" WINBASEAPI DWORD WINAPI SetCriticalSectionSpinCount(LPCRITICAL_SECTION, DWORD);
extern "C" WINBASEAPI BOOL WINAPI TryEnterCriticalSection(LPCRITICAL_SECTION);
#endif
#else
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
typedef pthread_mutex_t os_mutex_t;
typedef pthread_mutex_t os_mutex_t;
typedef std::pair<pthread_t, std::pair<bool, pthread_cond_t> > puremvc_thread_t;
#endif

#if defined(__APPLE__)
#define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
#endif

using namespace CXF;

Mutex::Mutex(void)
    : _mutex(NULL)
{
#if defined(_WIN32) || defined(_WIN64)
    _mutex = ::CreateMutex((LPSECURITY_ATTRIBUTES) 0, FALSE,(LPCTSTR) 0);
    if (_mutex == NULL)
        throw std::runtime_error("Cannot create mutex!");
#else
    register int rc;
    os_mutex_t* mutex = new os_mutex_t();
    pthread_mutexattr_t attr;
    ::pthread_mutexattr_init(&attr);
    ::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    if ((rc = ::pthread_mutex_init(mutex, &attr)))
    {
        ::pthread_mutexattr_destroy(&attr);
        std::cerr << strerror(rc) << std::endl;
        throw std::runtime_error("Cannot create mutex!");
    }
    _mutex = mutex;
    ::pthread_mutexattr_destroy(&attr);
#endif
}

void Mutex::lock(void)
{
#if defined(_WIN32) || defined(_WIN64)
    if (::WaitForSingleObject(_mutex, INFINITE) == WAIT_OBJECT_0)
        return;
    throw std::runtime_error("Cannot lock mutex!");
#else
    register int rc;
    os_mutex_t* mutex = (os_mutex_t*)_mutex;
    if ((rc = ::pthread_mutex_lock(mutex)))
    {
        std::cerr << strerror(rc) << std::endl;
        throw std::runtime_error("Cannot lock mutex!");
    }
#endif
}

bool Mutex::tryLock(void)
{
#if defined(_WIN32) || defined(_WIN64)
    switch (::WaitForSingleObject(_mutex, 0)) 
    {
    case WAIT_TIMEOUT:
        return false;
    case WAIT_OBJECT_0:
        return true;
    default:
        throw std::runtime_error("Cannot try lock mutex!");
    }
#else
    register int rc;
    os_mutex_t* mutex = (os_mutex_t*)_mutex;
    if ((rc = ::pthread_mutex_trylock(mutex)) == 0)
        return true;
    if (rc == EBUSY)
        return false;
    std::cerr << strerror(rc) << std::endl;
    throw std::runtime_error("Cannot try lock mutex!");
#endif
#if defined(__DMC__)
    return false;
#endif
}

void Mutex::unlock(void)
{
#if defined(_WIN32) || defined(_WIN64)
    if (!::ReleaseMutex(_mutex))
        throw std::runtime_error("Cannot unlock mutex!");
#else
    register int rc;
    os_mutex_t* mutex = (os_mutex_t*)_mutex;
    if ((rc = ::pthread_mutex_unlock(mutex)))
    {
        std::cerr << strerror(rc) << std::endl;
        throw std::runtime_error("Cannot unlock mutex!");
    }
#endif
}
Mutex::~Mutex(void)
{
#if defined(_WIN32) || defined(_WIN64)
    ::CloseHandle(_mutex);
#else
    os_mutex_t* mutex = (os_mutex_t*)_mutex;
    ::pthread_mutex_destroy(mutex);
    delete mutex;
    _mutex = NULL;
#endif
}


FastMutex::FastMutex(void)
    : _mutex(NULL)
{
#if defined(_WIN32) || defined(_WIN64)
    _mutex = new CRITICAL_SECTION();
    ::InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)_mutex, 4000);
#else
    register int rc;
    os_mutex_t* mutex = new os_mutex_t();
    pthread_mutexattr_t attr;
    ::pthread_mutexattr_init(&attr);
    ::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    if ((rc = ::pthread_mutex_init(mutex, &attr)))
    {
        ::pthread_mutexattr_destroy(&attr);
        std::cerr << strerror(rc) << std::endl;
        throw std::runtime_error("Cannot create mutex!");
    }
    _mutex = mutex;
    ::pthread_mutexattr_destroy(&attr);
#endif
}

void FastMutex::lock(void)
{
    os_fast_mutex_t* mutex = (os_fast_mutex_t*)_mutex;
#if defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(mutex);
#else
    register int rc;
    if ((rc = ::pthread_mutex_lock(mutex)))
    {
        std::cerr << strerror(rc) << std::endl;
        throw std::runtime_error("Cannot lock mutex!");
    }
#endif
}

bool FastMutex::tryLock(void)
{
    os_fast_mutex_t* mutex = (os_fast_mutex_t*)_mutex;
#if defined(_WIN32) || defined(_WIN64)
    return ::TryEnterCriticalSection(mutex) != 0;
#else
    register int rc;
    if ((rc = ::pthread_mutex_trylock(mutex)) == 0)
        return true;
    if (rc == EBUSY)
        return false;
    std::cerr << strerror(rc) << std::endl;
    throw std::runtime_error("Cannot try lock mutex!");
#endif
#if defined(__DMC__)
    return false;
#endif
}

void FastMutex::unlock(void)
{
    os_fast_mutex_t* mutex = (os_fast_mutex_t*)_mutex;
#if defined(_WIN32) || defined(_WIN64)
    LeaveCriticalSection(mutex);
#else
    register int rc;
    if ((rc = ::pthread_mutex_unlock(mutex)))
    {
        std::cerr << strerror(rc) << std::endl;
        throw std::runtime_error("Cannot unlock mutex!");
    }
#endif
}

FastMutex::~FastMutex(void)
{
    if (_mutex == NULL) return;
    os_fast_mutex_t* mutex = (os_fast_mutex_t*)_mutex;
#if defined(_WIN32) || defined(_WIN64)
    ::DeleteCriticalSection(mutex);
#else
    ::pthread_mutex_destroy(mutex);
#endif
    delete mutex;
    _mutex = NULL;
}
