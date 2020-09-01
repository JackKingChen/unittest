/*/*******************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2013-04-07
*
*******************************************************************/

#include "windows_os.h"

namespace host
{
    /************************************************************************/
    /*  static & standalone                                                 */
    /************************************************************************/
    bool   OS::sys_initialized  = false;
    float  OS::cpu_bogomips     = 0.0;
    static OS theHost;

    /************************************************************************/
    /* class of OS                                                          */
    /************************************************************************/
    OS::OS()
    {
        /*
        * global init
        */
        if(!sys_initialized)
        {
            sys_initialized = true;
            HKEY hKey;

            if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey)
                == ERROR_SUCCESS)
            {
                unsigned long SpeedVal = 0;
                unsigned long SpeedLen = sizeof(SpeedVal);

                RegQueryValueEx( hKey, "~MHz", NULL, NULL, (LPBYTE)&SpeedVal, &SpeedLen );
                RegCloseKey(hKey);

                cpu_bogomips = (float)SpeedVal;
            }

            /*
            * adjust console for Windows
            */
            HANDLE     handle = GetStdHandle(STD_OUTPUT_HANDLE);
            SMALL_RECT rect   = {0,0,120,200};
            COORD      size   = {120,800};
            SetConsoleScreenBufferSize(handle,size);
            SetConsoleWindowInfo(handle,true,&rect);

            /*
            * init WSA
            */
            WSAData WSA;
            WSAStartup(MAKEWORD(2,2), &WSA);

            /*
            * init COM
            */
#ifdef OS_WINDOWS_AFX
            if(!AfxOleInit())
            {
                assert(false);
            }
#endif

        }

        /*
        * private init
        */


    }

    OS::~OS()
    {

    }

    float OS::GetMips()
    {
        return cpu_bogomips;
    }

    void  OS::Sleep(int sec)
    {
        ::Sleep(sec*1000);
    }

    void  OS::SleepMs(int ms)
    {
        ::Sleep(ms);
    }

    /************************************************************************/
    /* class of OSFile                                                      */
    /************************************************************************/


    /************************************************************************/
    /* class of OSTime                                                      */
    /************************************************************************/
    void OSTime::GetLocalTime()
    {
        SYSTEMTIME      st_1970;
        SYSTEMTIME      st_now;
        LARGE_INTEGER   li_1970;
        LARGE_INTEGER   li_now;
        FILETIME        ft;
        
        memset(&st_1970,0,sizeof(st_1970));
        st_1970.wYear   = 1970;
        st_1970.wMonth  = 1;
        st_1970.wDay    = 1;
        ::SystemTimeToFileTime(&st_1970, &ft);
        li_1970.LowPart  = ft.dwLowDateTime;
        li_1970.HighPart = ft.dwHighDateTime;
        li_1970.QuadPart/= (1000*1000*10);

        ::GetLocalTime(&st_now);
        ::SystemTimeToFileTime(&st_now, &ft);

        li_now.LowPart   = ft.dwLowDateTime;
        li_now.HighPart  = ft.dwHighDateTime;
        li_now.QuadPart /= (1000*1000*10);
        li_now.QuadPart -= li_1970.QuadPart;

        tv_sec = li_now.LowPart;
        tv_usec= (st_now.wMilliseconds*1000);
    }

    void OSTime::GetGreenwichTime()
    {
        SYSTEMTIME      st_1970;
        SYSTEMTIME      st_now;
        LARGE_INTEGER   li_1970;
        LARGE_INTEGER   li_now;
        FILETIME        ft;

        memset(&st_1970,0,sizeof(st_1970));
        st_1970.wYear   = 1970;
        st_1970.wMonth  = 1;
        st_1970.wDay    = 1;
        ::SystemTimeToFileTime(&st_1970, &ft);
        li_1970.LowPart  = ft.dwLowDateTime;
        li_1970.HighPart = ft.dwHighDateTime;
        li_1970.QuadPart/= (1000*1000*10);

        ::GetSystemTime(&st_now);
        ::SystemTimeToFileTime(&st_now, &ft);

        li_now.LowPart   = ft.dwLowDateTime;
        li_now.HighPart  = ft.dwHighDateTime;
        li_now.QuadPart /= (1000*1000*10);
        li_now.QuadPart -= li_1970.QuadPart;

        tv_sec = li_now.LowPart;
        tv_usec= (st_now.wMilliseconds*1000);
    }

    unsigned long OSTime::GetSystemTick(void)
    {
        LARGE_INTEGER litmp;
        long double   dfreq;
        long double   dtick;

        QueryPerformanceFrequency(&litmp);
        dfreq = (long double)litmp.QuadPart;
        QueryPerformanceCounter(&litmp);
        dtick = (long double)litmp.QuadPart;

        return static_cast<unsigned long>((1000*1000)*(dtick/dfreq));
    }

    /************************************************************************/
    /* class of OSThread                                                    */
    /************************************************************************/
    OSThread::OSThread(OSProc pp)
    {
        proc  =pp;
        sem   =(void*)CreateSemaphore(NULL,0,1,NULL);
        handle=0;
        if(proc==NULL)
            proc = (OSProc)OSThread::RunProc;

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
        if(proc)
        {
            DWORD pthread;

            Term();

            ResetEvent(sem);
            running= true;

            handle = (unsigned long)CreateThread(NULL,
                stackSize,(LPTHREAD_START_ROUTINE)proc,this,NULL,&pthread);
            if(handle==0)
            {
                return false;
            }

            return true;
        }

        return false;
    }

    bool OSThread::Term(bool bSoftly)
    {
        if(handle)
        {
            if(bSoftly)
            {
                ResetEvent(sem);
                running = false;
                WaitForSingleObject((HANDLE)sem,500);
            }
            else
                TerminateThread((HANDLE)handle,0);
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
        assert(sem!=NULL);
        assert(handle!=NULL);

        OnLoad();

        while(running && Main());

        /*must be clear*/
        handle = NULL;
        OnTerm();

        ReleaseSemaphore((HANDLE)sem, 1, NULL);

        return true;
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

    void *OSThread::RunProc(OSThread *self)
    {
        if(self)
            self->Proc();
        return NULL;
    }
    /************************************************************************/
    /* class of OSSemaphore                                                 */
    /************************************************************************/
    OSSemaphore::OSSemaphore()
    {
        handle = (void*)CreateSemaphore(NULL,0,1,NULL);

    }
    OSSemaphore::~OSSemaphore()
    {
        ReleaseSemaphore(handle,1,NULL);
        CloseHandle(handle);
    }

    bool OSSemaphore::Reset(int count)
    {
        ResetEvent(handle);
        return true;
    }
    bool OSSemaphore::Wait(int timeout)
    {
        if(timeout<=0)
            timeout = INFINITE;

        DWORD waitResult = WaitForSingleObject(handle,timeout);
        switch (waitResult)
        {
        case WAIT_OBJECT_0:
            return true;
        case WAIT_TIMEOUT:
        default:
            return false;
        }
        return true;
    }
    bool OSSemaphore::Post()
    {
        ReleaseSemaphore(handle,1,NULL);
        return true;
    }

    /************************************************************************/
    /* class of OSLock                                                      */
    /************************************************************************/
    OSLock::OSLock()
    {
        handle = (void*)CreateMutex(NULL,false,NULL);
    }
    OSLock::~OSLock()
    {
        ReleaseMutex(handle);
        CloseHandle(handle);
    }

    bool OSLock::Lock(int timeout)
    {
        if(timeout<=0)
            timeout = INFINITE;

        DWORD waitResult = WaitForSingleObject(handle,timeout);
        switch (waitResult)
        {
        case WAIT_OBJECT_0:
            return true;
        case WAIT_TIMEOUT:
        default:
            return false;
        }
        return true;
    }
    bool OSLock::Unlock()
    {
        ReleaseMutex(handle);
        return true;
    }

    bool OSLock::TryLock()
    {

        return Lock();
    }
    bool OSLock::LockRead()
    {

        return Lock();
    }
    bool OSLock::TryLockRead()
    {

        return Lock();
    }
    bool OSLock::LockWrite()
    {

        return Lock();
    }
    bool OSLock::TryLockWrite()
    {

        return Lock();
    }


    /************************************************************************/
    /* class of OSSock                                                      */
    /************************************************************************/
    OSSock::OSSock(void *ref)
    {
        nPeerAddr = 0;
        nPeerPort = 0;
        nThisAddr = 0;
        nThisPort = 0;
        handle    = ref;
    }
    OSSock::~OSSock()
    {
        Close();
    }

    unsigned int   OSSock::Htonl(unsigned int hostlong)
    {
        return htonl(hostlong);
    }

    unsigned short OSSock::Htons(unsigned short hostshort)
    {
        return htons(hostshort);
    }

    unsigned int   OSSock::Ntohl(unsigned int netlong)
    {
        return ntohl(netlong);
    }

    unsigned short OSSock::Ntohs(unsigned short netshort)
    {
        return ntohs(netshort);
    }
    unsigned long  OSSock::IPv4(const char*strIP)
    {
        return inet_addr(strIP);
    }
    const char *OSSock::IPv4(unsigned long nIP)
    {
        struct in_addr in;
        in.s_addr = nIP;
        return inet_ntoa(in);
    }

    bool OSSock::Select(OSSock *skset[],int nr,int timeout)
    {
        struct timeval tv;
        fd_set fdread;
        int    retval;

        assert(skset!=NULL);
        assert(nr>0);

        FD_ZERO(&fdread);
        for (int i = 0; i < nr; i++) 
            FD_SET((SOCKET)(skset[i]->handle), &fdread); 

        if(timeout>0)
        {
            tv.tv_sec = timeout/1000;
            tv.tv_usec= (timeout%1000)*1000;
            retval = select(0, &fdread, NULL, NULL,&tv); 
        }
        else
        {
            retval = select(0, &fdread, NULL, NULL,NULL); 
        }
        if (retval <= 0) 
            return false;

        for (int i = 0; i < nr; i++) 
        {
            if (!FD_ISSET((SOCKET)(skset[i]->handle), &fdread))
                skset[i]= NULL;
        }
        return true;
    }

    bool OSSock::Create(int nType,const char*strIP,unsigned nPort)
    {
        /*for safe*/
        Close();

        /*open socket*/
        switch(nType)
        {
        case ST_RAW:
            nType = SOCK_RAW;
            break;
        case ST_UDP:
            nType = SOCK_DGRAM;
            break;
        case ST_TCP:
            nType = SOCK_STREAM;
            break;
        default:
            nType = SOCK_DGRAM;
            break;
        }

        handle = (void*)socket(AF_INET,nType,0);

        if(handle)
        {
            /*try bind*/
            if(nPort!=0)
                return Bind(strIP,nPort);
            return true;
        }
        else
            return false;
    }

    void OSSock::Close()
    {
        if(handle)
            closesocket((SOCKET)handle);
        handle = NULL;
    }

    bool OSSock::Connect(const char* strIP,unsigned nPort)
    {
        if(handle)
        {
            SOCKADDR_IN addr;

            addr.sin_family  = AF_INET;
            addr.sin_port    = htons(nPort);
            if(strIP)
                addr.sin_addr.s_addr = inet_addr(strIP);
            else
                addr.sin_addr.s_addr = htonl(INADDR_ANY);

            /*save*/
            nPeerAddr = ntohl(addr.sin_addr.s_addr);
            nPeerPort = nPort;

            if (connect((SOCKET)handle, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
                return false;

            return true;
        }

        return false;
    }

    bool OSSock::Bind(const char* strIP,unsigned nPort)
    {
        if(handle)
        {
            SOCKADDR_IN addr;

            addr.sin_family  = AF_INET;
            addr.sin_port    = htons(nPort);
            if(strIP)
                addr.sin_addr.s_addr = inet_addr(strIP);
            else
                addr.sin_addr.s_addr = htonl(INADDR_ANY);

            /*save*/
            nThisAddr = ntohl(addr.sin_addr.s_addr);
            nThisPort = nPort;

            if (bind((SOCKET)handle, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
                return false;

            return true;
        }

        return false;
    }

    bool OSSock::Listen ()
    {
        assert(handle!=NULL);

        return (listen((SOCKET)handle,SOMAXCONN)>0);
    }

    bool OSSock::Accept(OSSock *&ref_sock)
    {
        SOCKADDR_IN addr;
        SOCKET      sock;
        int         addrlen = sizeof(addr);

        sock = accept((SOCKET)handle, (SOCKADDR*)&addr, &addrlen);
        if (sock != INVALID_SOCKET)
        {
            ref_sock = new OSSock((void*)sock);
            if(ref_sock)
            {
                ref_sock->nPeerAddr = ntohl(addr.sin_addr.s_addr);
                ref_sock->nPeerPort = ntohs(addr.sin_port);
                return true;
            }
        }
        return false;
    }

    int  OSSock::Recv(void* lpBuf, int nBufLen,unsigned long*addIP,unsigned *nPort)
    {
        if(handle==NULL)
            return -1;

        if(addIP || nPort)
        {
            SOCKADDR_IN addr;
            int         size  = sizeof(addr);
            int         retval= 0;

            retval = recvfrom((SOCKET)handle, (char*)lpBuf,nBufLen, 0,(struct sockaddr*)&addr,&size);
            if(retval>0)
            {
                if(addIP)
                    *addIP = ntohl(addr.sin_addr.s_addr);
                if(nPort)
                    *nPort = ntohs(addr.sin_port);
            }
            return retval;
        }
        else
        {
            return recv((SOCKET)handle, (char*)lpBuf,nBufLen, 0);
        }
    }

    int  OSSock::Send(void* lpBuf, int nBufLen,unsigned long addIP,unsigned nPort)
    {
        if(handle==NULL)
            return -1;

        assert(lpBuf!=NULL);
        assert(nBufLen>0);

        if(addIP!=0)
        {
            SOCKADDR_IN addr;

            addr.sin_family      = AF_INET;
            addr.sin_port        = htons(nPort);
            addr.sin_addr.s_addr = htonl(addIP);

            return sendto((SOCKET)handle,(char*)lpBuf,nBufLen, 0,(struct sockaddr*)&addr,sizeof(addr));
        }
        else
        {
            return send((SOCKET)handle,(char*)lpBuf,nBufLen, 0);
        }
        return 0;
    }

    bool OSSock::SetBuffer(int nSend,int nRecv)
    {
        assert(handle!=NULL);

        if(nSend>0)
        {
            if(::setsockopt((SOCKET)handle, 
                SOL_SOCKET, SO_SNDBUF, (char *)&nSend, sizeof(nSend)) == SOCKET_ERROR )
                return false;
        }

        if(nRecv>0)
        {
            if(::setsockopt((SOCKET)handle,
                SOL_SOCKET, SO_RCVBUF, (char *)&nRecv, sizeof(nRecv)) == SOCKET_ERROR )
                return false;
        }
        return true;
    }
    bool OSSock::GetBuffer(int&nSend,int&nRecv)
    {
        assert(handle!=NULL);

        int len;

        if(::getsockopt((SOCKET)handle, 
            SOL_SOCKET, SO_SNDBUF, (char *)&nSend, &len) == SOCKET_ERROR )
            return false;

        if(::getsockopt((SOCKET)handle,
            SOL_SOCKET, SO_RCVBUF, (char *)&nRecv, &len) == SOCKET_ERROR )
            return false;

        return true;
    }

    bool OSSock::SetTimeout(int nSend,int nRecv)
    {
        assert(handle!=NULL);

        if(nSend>=0)
        {
            if(::setsockopt((SOCKET)handle, 
                SOL_SOCKET, SO_SNDTIMEO, (char *)&nSend, sizeof(nSend)) == SOCKET_ERROR )
                return false;
        }

        if(nRecv>=0)
        {
            if(::setsockopt((SOCKET)handle,
                SOL_SOCKET, SO_RCVTIMEO, (char *)&nRecv, sizeof(nRecv)) == SOCKET_ERROR )
                return false;
        }

        return true;
    }

    bool OSSock::GetTimeout(int&nSend,int&nRecv)
    {
        assert(handle!=NULL);

        int len;

        if(::getsockopt((SOCKET)handle, 
            SOL_SOCKET, SO_SNDTIMEO, (char *)&nSend, &len) == SOCKET_ERROR )
            return false;

        if(::getsockopt((SOCKET)handle,
            SOL_SOCKET, SO_RCVTIMEO, (char *)&nRecv, &len) == SOCKET_ERROR )
            return false;

        return true;
    }

    bool OSSock::SetBlock(bool blocking)
    {
        unsigned long ul = blocking?0:1;

        if (ioctlsocket((SOCKET)handle, 
            FIONBIO, (unsigned long *)&ul) == SOCKET_ERROR)
            return false;

        return true;
    }

    bool OSSock::GetBlock(bool&blocking)
    {
        return true;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};

