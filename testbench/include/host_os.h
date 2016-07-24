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
#ifndef __HOST_OS_H__
#define __HOST_OS_H__

/************************************************************************/
/* basic class of host                                              */
/************************************************************************/
namespace host
{
    class OS;
    class OSTime;
    class OSFile;
    class OSThread;
    class OSSemaphore;
    class OSLock;
    class OSLockSection;
    class OSSock;
    class OSSockUDP;
    class OSSockTCP;

    /*
    * class of OS
    */
    class OS
    {
    public:
        OS();
        ~OS();

    public:
        static float GetMips();
        static void  Sleep(int sec);
        static void  SleepMs(int ms);

    public:
        static void Seed(int seed)
        {
             srand(seed);
        };
        static int  Rand()
        {
            return rand();
        };

    protected:
        static bool   sys_initialized;
        static float  cpu_bogomips;
    };

    /*
    * class of OSThread
    */
    class OSThread:public OS
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

        OSThread(OSProc pp=NULL);
        ~OSThread();

        virtual bool Load(size_t stackSize=0);
        virtual bool Term(bool bSoftly=true);
        virtual bool Wait();
        virtual bool Proc();
        virtual bool Main();

        virtual void OnLoad(){};
        virtual void OnTerm(){};

        virtual bool SetSched(Priority  priority);
        virtual bool GetSched(Priority &priority);

    public:
        static bool SetPriority(Priority  priority,unsigned long handle=0);
        static bool GetPriority(Priority &priority,unsigned long handle=0);
        static bool SetStackSize(unsigned long  size);
        static bool GetStackSize(unsigned long &size);
        static bool SetAffinity(unsigned  mask=0x01);
        static bool GetAffinity(unsigned &mask);

        static void*RunProc(OSThread *self);

    protected:
        OSProc        proc;
        bool          running;
        void         *sem;
        unsigned long handle;
    };

    /*
    * class of OSTime
    */
    class OSTime:public OS
    {
    public:
        OSTime(long sec,long usec)
        {tv_sec  = sec;tv_usec = usec;}
        OSTime(OSTime& time)
        {tv_sec  = time.tv_sec;tv_usec = time.tv_usec;}
        OSTime()
        {GetTime();}

    public:

        OSTime &operator =(OSTime &time)
        {
            tv_sec  = time.tv_sec;
            tv_usec = time.tv_usec;
            return *this;
        };

        OSTime &operator -=(OSTime &time)
        {
            tv_sec -= time.tv_sec;
            tv_usec-= time.tv_usec;
            if (tv_usec < 0) 
            {
                --tv_sec;tv_usec += 1000000;
            }
            return *this;
        };

        OSTime &operator +=(OSTime &time)
        {
            tv_sec += time.tv_sec;
            tv_usec+= time.tv_usec;
            if (tv_usec >= 1000000)
            {
                ++tv_sec;tv_usec -= 1000000;
            }
            return *this;
        };

        long GetSecond() 
        {return tv_sec;};
        long GetMSecond()
        {return tv_sec*1000+tv_usec/1000;};
        long GetUSecond()
        {return tv_sec*1000*1000+tv_usec;};

    public:
        void GetLocalTime();
        void GetGreenwichTime();
        void GetTime(){GetLocalTime();};

    public:
        static unsigned long GetSystemTick(void);

    public:
        long tv_sec;
        long tv_usec;
    };


    /*
    * class of OSFile
    */
    class OSFile:public OS
    {
    public:
        enum seektype
        {
            beg = 0x80000000,
            cur = 0x80000001,
            end = 0x80000002,
        };

    public:

        OSFile(const char *path=NULL,const char * mode="r");
        ~OSFile();

        bool   open (const char *path,const char * mode="r");
        bool   close(void);
        size_t seek (seektype type,int pos=0);
        size_t seek (int pos);
        size_t read (void* buff,size_t size);
        size_t write(void* buff,size_t size);
        bool   getline(char* str,size_t size);
        size_t putline(const char* fmt,...);
        size_t total(void);
        size_t tell(void);
        void   flush(void);

        OSFile &operator<<(const char* str)
        {
            putline(str);
            return *this;
        };
        OSFile &operator>>(char* str)
        {
            getline(str,4096);
            return *this;
        };

    public:
        static bool exist(const char * path);
        static bool path_dir(const char * full_path,char *dir_path);
        static bool path_file(const char * full_path,char *file_path);

    public:
        bool is_open()
        {return handle!=NULL;};
        bool is_writeable()
        {return writeable;};

        operator FILE*()
        {return handle;};

    protected:
        FILE *handle;
        bool  writeable;
    };

    /*
    * class of OSSemaphore
    */
    class OSSemaphore:public OS
    {
    public:
        OSSemaphore();
        ~OSSemaphore();

    public:
        bool Reset(int count=0);
        bool Wait(int timeout=1000);
        bool Post();

    protected:
        void *handle;
    };

    /*
    * class of OSLock
    */
    class OSLock:public OS
    {
    public:
        OSLock();
        ~OSLock();

    public:
        bool Lock(int timeout=0);
        bool TryLock();
        bool LockRead();
        bool TryLockRead();
        bool LockWrite();
        bool TryLockWrite();

        bool Unlock();

    protected:
        void *handle;
    };

    /*
    * class of OSLockSection
    */
    class OSLockSection:public OS
    {
    public:
        OSLockSection(OSLock & lock):section_lock(&lock)
        {
            section_lock->Lock();
        }
        ~OSLockSection()
        {
            section_lock->Unlock();
        }
    protected:
        OSLock * section_lock;
    };

    /*
    * class of OSSock
    */
    class OSSock:public OS
    {
    public:
        enum Type
        {
            ST_RAW        =1,               /* raw-protocol interface */
            ST_UDP        =2,               /* datagram socket */
            ST_TCP        =3,               /* stream socket */
        };

    public:
        OSSock(void *ref=NULL);
        ~OSSock();

    public:
        static unsigned int   Htonl(unsigned int   hostlong);
        static unsigned short Htons(unsigned short hostshort);

        static unsigned int   Ntohl(unsigned int   netlong);
        static unsigned short Ntohs(unsigned short netshort);

        static unsigned long  IPv4(const char*strIP);
        static const    char *IPv4(unsigned long nIP);

        static bool Select(OSSock *skset[],int nr,int timeout=0);

    public:
        virtual bool Create(int nType,const char*strIP = NULL,unsigned nPort = 0);
        virtual void Close();

        virtual bool Connect(const char* strIP,unsigned nPort);
        virtual bool Bind   (const char* strIP,unsigned nPort);
        virtual bool Listen ();
        virtual bool Accept (OSSock *&sock);

        virtual int  Recv(void* lpBuf, int nBufLen,unsigned long*addIP,unsigned*nPort);
        virtual int  Recv(void* lpBuf, int nBufLen)
        {return Recv(lpBuf,nBufLen,NULL,NULL);}

        virtual int  Send(void* lpBuf, int nBufLen,unsigned long addIP,unsigned nPort);
        virtual int  Send(void* lpBuf, int nBufLen)
        {return Send(lpBuf,nBufLen,0,0);};
        virtual int  SendTo(void* lpBuf, int nBufLen)
        {return Send(lpBuf,nBufLen,nPeerAddr,nPeerPort);};

        virtual bool SetBuffer(int nSend,int nRecv);
        virtual bool GetBuffer(int&nSend,int&nRecv);

        virtual bool SetTimeout(int nSend,int nRecv);
        virtual bool GetTimeout(int&nSend,int&nRecv);

        virtual bool SetBlock(bool blocking);
        virtual bool GetBlock(bool&blocking);

        bool IsOpen()
        {return handle!=NULL;};

    public:
        unsigned long  nPeerAddr;
        unsigned short nPeerPort;
        unsigned long  nThisAddr;
        unsigned short nThisPort;

    protected:
        void   * handle;
    };

    class OSSockUDP:public OSSock
    {
    public:
        OSSockUDP();
        ~OSSockUDP();

    };

    class OSSockTCP:public OSSock
    {
    public:
        OSSockTCP();
        ~OSSockTCP();

    };

};/*end of host*/
/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*__HOST_OS_H__*/
