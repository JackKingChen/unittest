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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <termios.h> 
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/socket.h> 

#include <arpa/inet.h> 
#include <netinet/ip.h>
#include <netinet/ip6.h>

#include "host.h"

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
            FILE* fp = fopen("/proc/cpuinfo","r");

            if(fp)
            {
                char line[512];
                while(fgets(line,sizeof(line)-1,fp))
                {
                    /*
                    * BogoMIPS        : 110.18
                    * bogomips        : 5866.71
                    */
                    if(strncasecmp(line,"BogoMIPS",sizeof("BogoMIPS")-1)==0)
                    {
                        char * mips = strchr(line,':');
                        if(mips)
                        {
                            sscanf(mips+1,"%f",&cpu_bogomips);
                        }
                    }
                }
            }

            fclose(fp);
        }

        /*
        * private init
        */

    };
    OS::~OS()
    {

    };

    float OS::GetMips()
    {
        return cpu_bogomips;
    }

    void  OS::Sleep(int sec)
    {
        ::sleep(sec);
    }

    void  OS::SleepMs(int ms)
    {
        ::usleep(ms*1000);
    }

    /************************************************************************/
    /* class of  OSTime                                                     */
    /************************************************************************/
    void OSTime::GetLocalTime()
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        tv_sec  = tv.tv_sec;
        tv_usec = tv.tv_usec;
    }
    void OSTime::GetGreenwichTime()
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        tv_sec  = tv.tv_sec;
        tv_usec = tv.tv_usec;
    }

    unsigned long OSTime::GetSystemTick(void)
    {
        struct timeval tv;

        gettimeofday(&tv,NULL);

        return ((tv.tv_sec)*1000*1000) + (tv.tv_usec);
    }

    /************************************************************************/
    /* class of OSThread                                                    */
    /************************************************************************/
    OSThread::OSThread(OSProc pp)
    {
        proc  =pp;
        sem   =malloc(sizeof(sem_t));
        handle=0;
        if(proc==NULL)
            proc = (OSProc)OSThread::RunProc;

        sem_init((sem_t*)sem,0,0);
    }

    OSThread::~OSThread()
    {
        Term();
        if(sem)
            free(sem);
    }

    bool OSThread::Load(size_t stackSize)
    {
        if(proc)
        {
            pthread_t pthread;
            pthread_attr_t attr;

            pthread_attr_init(&attr);
            if(stackSize>0)
                pthread_attr_setstacksize(&attr,stackSize);

            Term();
            sem_init((sem_t*)sem,0,0);

            if(pthread_create(&pthread, &attr,proc, this))
            {
                assert(false);
                return false;
            }

            running = true;
            handle  = (unsigned long)pthread;

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
                sem_init((sem_t*)sem,0,0);
                running = false;
                sem_wait((sem_t*)sem);
            }
            else
            {
                //pthread_cancel((pthread_t)handle);
                pthread_join((pthread_t)handle,NULL);
            }
        }
        running = false;
        handle  = 0;

        return true;
    }
    bool OSThread::Wait()
    {
        assert(sem!=NULL);

        if(handle)
        {
            sem_wait((sem_t*)sem);
        }
        return false;
    }
    bool OSThread::Proc()
    {
        assert(sem!=NULL);

        OnLoad();

        while(running && Main());

        /*must be clear*/
        handle = NULL;
        OnTerm();

        sem_post((sem_t*)sem);
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
        struct sched_param  param;

        if(priority<PriorityMin)
            priority = PriorityMin;
        if(priority>PriorityMax)
            priority = PriorityMax;

        if(handle==0)
            handle = (unsigned long)pthread_self();

        if(priority<=0)
        {
            param.sched_priority  =  sched_get_priority_min(SCHED_OTHER);
            if(pthread_setschedparam((pthread_t)handle,SCHED_OTHER,&param)<0)
            {
                return false;
            }
        }
        else
        {
            int priority_min = sched_get_priority_min(SCHED_RR);
            int priority_max = sched_get_priority_max(SCHED_RR);
            int priority_set = priority_max-priority_min;

            param.sched_priority  = priority_set*priority/100;
            if(pthread_setschedparam(pthread_self(),SCHED_RR,&param)<0)
            {
                return false;
            }
        }
        return true;
    }

    bool OSThread::GetPriority(Priority &priority,unsigned long handle)
    {
        struct sched_param  param;
        int                 policy;
        int priority_min = sched_get_priority_min(SCHED_RR);
        int priority_max = sched_get_priority_max(SCHED_RR);
        int priority_set = priority_max-priority_min;

        if(handle==0)
            handle = (unsigned long)pthread_self();

        if(pthread_getschedparam((pthread_t)handle,&policy,&param)<0)
        {
            return false;
        }

        priority = (Priority)(PriorityMin+(PriorityMax*param.sched_priority)/(priority_set));

        return true;
    }
    bool OSThread::SetStackSize(unsigned long  size)
    {
        size = 0;
        return true;
    }

    bool OSThread::GetStackSize(unsigned long &size)
    {
        size = 0;
        return true;
    }
    bool OSThread::SetAffinity(unsigned  mask)
    {
        
        return true;
    }
    bool OSThread::GetAffinity(unsigned &mask)
    {
        
        return true;
    }

    void *OSThread::RunProc(OSThread *self)
    {
        /*
        * set cancelable
        */
        //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,       NULL);
        //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,  NULL);

        if(self)
            self->Proc();
        return NULL;
    }

    /************************************************************************/
    /* class of OSSemaphore                                                 */
    /************************************************************************/
    OSSemaphore::OSSemaphore()
    {
        handle = calloc(1,sizeof(sem_t));
        sem_init((sem_t*)handle,0,0);
    }
    OSSemaphore::~OSSemaphore()
    {
        free(handle);
    }
    bool OSSemaphore::Reset(int count)
    {
        sem_init((sem_t*)handle,0,0);
        return true;
    }
    bool OSSemaphore::Wait(int timeout)
    {
        if(timeout>0)
        {
            struct timespec ts; 
            struct timeval  tv; 

            gettimeofday(&tv,NULL);

            ts.tv_sec  = tv.tv_sec + timeout/1000;
            ts.tv_nsec = tv.tv_usec+ (timeout%1000)*1000;

            if(ts.tv_nsec > 1000*1000*1000)
            {
                ts.tv_nsec-=1000*1000*1000;
                ts.tv_sec +=1;
            }

            //sem_timedwait((sem_t*)handle, &ts);
            
        }
        else
        {
            sem_wait((sem_t*)handle);
        }
        return true;
    }
    bool OSSemaphore::Post()
    {
        sem_post((sem_t*)handle);

        return true;
    }

    /************************************************************************/
    /* class of OSLock                                                      */
    /************************************************************************/
    OSLock::OSLock()
    {
        handle = calloc(1,sizeof(pthread_mutex_t));
        pthread_mutex_init((pthread_mutex_t*)handle,NULL);
    }
    OSLock::~OSLock()
    {
        pthread_mutex_destroy((pthread_mutex_t*)handle);
        free(handle);
    }

    bool OSLock::Lock(int timeout)
    {
        pthread_mutex_lock((pthread_mutex_t*)handle);
        return true;
    }
    bool OSLock::TryLock()
    {
        pthread_mutex_trylock((pthread_mutex_t*)handle);
        return true;
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

        return TryLock();
    }
    bool OSLock::Unlock()
    {
        pthread_mutex_unlock((pthread_mutex_t*)handle);
        return true;
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
        int    fdmax;
        long    fdcur;
        int    retval;

        assert(skset!=NULL);
        assert(nr>0);

        FD_ZERO(&fdread);
        fdmax = 0;
        for (int i = 0; i < nr; i++) 
        {
            fdcur = (long)(skset[i]->handle);
            FD_SET(fdcur, &fdread); 
            if(fdcur>fdmax)
                fdmax = fdcur;
        }

        if(timeout>0)
        {
            tv.tv_sec = timeout/1000;
            tv.tv_usec= (timeout%1000)*1000;
            retval = select(fdmax+1, &fdread, NULL, NULL,&tv); 
        }
        else
        {
            retval = select(fdmax+1, &fdread, NULL, NULL,NULL); 
        }
        if (retval <= 0) 
            return false;

        for (int i = 0; i < nr; i++) 
        {
            if (!FD_ISSET((long)(skset[i]->handle), &fdread))
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
            close((long)handle);
        handle = NULL;
    }

    bool OSSock::Connect(const char* strIP,unsigned nPort)
    {
        if(handle)
        {
            struct sockaddr_in addr;

            addr.sin_family  = AF_INET;
            addr.sin_port    = htons(nPort);
            if(strIP)
                addr.sin_addr.s_addr = inet_addr(strIP);
            else
                addr.sin_addr.s_addr = htonl(INADDR_ANY);

            /*save*/
            nPeerAddr = ntohl(addr.sin_addr.s_addr);
            nPeerPort = nPort;

            if (connect((long)handle, (struct sockaddr*)&addr, sizeof(addr)) < 0)
                return false;

            return true;
        }

        return false;
    }

    bool OSSock::Bind(const char* strIP,unsigned nPort)
    {
        if(handle)
        {
            struct sockaddr_in addr;

            addr.sin_family  = AF_INET;
            addr.sin_port    = htons(nPort);
            if(strIP)
                addr.sin_addr.s_addr = inet_addr(strIP);
            else
                addr.sin_addr.s_addr = htonl(INADDR_ANY);

            /*save*/
            nThisAddr = ntohl(addr.sin_addr.s_addr);
            nThisPort = nPort;

            if (bind((long)handle, (struct sockaddr*)&addr, sizeof(addr)) < 0)
                return false;

            return true;
        }

        return false;
    }

    bool OSSock::Listen ()
    {
        assert(handle!=NULL);

        return (listen((long)handle,SOMAXCONN)>0);
    }

    bool OSSock::Accept(OSSock *&ref_sock)
    {
        struct sockaddr_in addr;
        socklen_t  addrlen = sizeof(addr);
        int        sock;

        sock = accept((long)handle, (struct sockaddr*)&addr, &addrlen);
        if (sock != -1)
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
            struct sockaddr_in addr;
            socklen_t size  = sizeof(addr);
            int       retval= 0;

            retval = recvfrom((long)handle, (char*)lpBuf,nBufLen, 0,(struct sockaddr*)&addr,&size);
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
            return recv((long)handle, (char*)lpBuf,nBufLen, 0);
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
            struct sockaddr_in addr;

            addr.sin_family      = AF_INET;
            addr.sin_port        = htons(nPort);
            addr.sin_addr.s_addr = htonl(addIP);

            return sendto((long)handle,(char*)lpBuf,nBufLen, 0,(struct sockaddr*)&addr,sizeof(addr));
        }
        else
        {
            return send((long)handle,(char*)lpBuf,nBufLen, 0);
        }
        return 0;
    }

    bool OSSock::SetBuffer(int nSend,int nRecv)
    {
        assert(handle!=NULL);

        if(nSend>0)
        {
            if(::setsockopt((long)handle, 
                SOL_SOCKET, SO_SNDBUF, (char *)&nSend, sizeof(nSend)) < 0 )
                return false;
        }

        if(nRecv>0)
        {
            if(::setsockopt((long)handle,
                SOL_SOCKET, SO_RCVBUF, (char *)&nRecv, sizeof(nRecv)) < 0 )
                return false;
        }
        return true;
    }
    bool OSSock::GetBuffer(int&nSend,int&nRecv)
    {
        assert(handle!=NULL);

        socklen_t len;

        if(::getsockopt((long)handle, 
            SOL_SOCKET, SO_SNDBUF, (char *)&nSend, &len) < 0 )
            return false;

        if(::getsockopt((long)handle,
            SOL_SOCKET, SO_RCVBUF, (char *)&nRecv, &len) < 0 )
            return false;

        return true;
    }

    bool OSSock::SetTimeout(int nSend,int nRecv)
    {
        assert(handle!=NULL);

        if(nSend>=0)
        {
            struct timeval timeout={nSend/1000,(nSend%1000)*1000};

            if(::setsockopt((long)handle, 
                SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0 )
                return false;
        }

        if(nRecv>=0)
        {
            struct timeval timeout={nRecv/1000,(nRecv%1000)*1000};

            if(::setsockopt((long)handle,
                SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0 )
                return false;
        }

        return true;
    }

    bool OSSock::GetTimeout(int&nSend,int&nRecv)
    {
        assert(handle!=NULL);

        struct timeval timeout;
        socklen_t len = sizeof(timeout);

        if(::getsockopt((long)handle, 
            SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, &len) < 0 )
            return false;
        nSend = timeout.tv_sec*1000+timeout.tv_usec/1000;

        if(::getsockopt((long)handle,
            SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, &len) < 0 )
            return false;
        nRecv = timeout.tv_sec*1000+timeout.tv_usec/1000;

        return true;
    }

    bool OSSock::SetBlock(bool blocking)
    {
        assert(handle!=NULL);

        long flags = fcntl((long)handle,F_GETFL);

        if(blocking)
            flags &=~O_NONBLOCK;
        else
            flags |= O_NONBLOCK;

        fcntl((long)handle,F_SETFL,flags);
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
