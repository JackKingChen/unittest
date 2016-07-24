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

#include <windows.h>
#include <process.h>
#include <mmsystem.h>

#include "host.h"
#include "device_audio.h"


#pragma comment( lib, "winmm.lib" )

namespace host
{
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    class OSAudioWin32;
    class OSAudioThread;

    typedef std::list<WAVEHDR*> WaveBuffList;

    /*
    * class of OSAudioWin32
    */
    class OSAudioWin32:public OSAudioDevice
    {
    public:
        enum
        {
            SEM_CAPTURE  = 0,
            SEM_PLAYOUT  = 1,

            /**/
            MAX_LATENCY = 40*1000,

            /*on frame for buffer ,and 50ms delay max*/
            MAX_BUFHDR  = 5,
            /*96000,10Hz,100ms,32bit sample*/
            MAX_BUFFER  = (96000/10)*sizeof(int),
        };

    public:

        OSAudioWin32(OSAudioManager& mng);
        ~OSAudioWin32();

    public:
        /*init*/
        virtual bool Init();

        /* Device enumeration*/
        virtual int GetPlayoutDevices();
        virtual int GetCaptureDevices();

        virtual int GetPlayoutDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN]);
        virtual int GetCaptureDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN]);

        /*Device status*/
        virtual bool IsPlayoutOpen();
        virtual bool IsCaptureOpen();
        virtual bool IsPlayoutRunning();
        virtual bool IsCaptureRunning();

        /*Device open&close*/
        virtual bool OpenPlayoutDevice();
        virtual bool OpenCaptureDevice();

        virtual bool ClosePlayoutDevice();
        virtual bool CloseCaptureDevice();

        /*Device start&stop*/
        virtual bool StartPlayout(unsigned long ms=0);
        virtual bool StartCapture(unsigned long ms=0);
        virtual bool StopPlayout();
        virtual bool StopCapture();
        virtual bool WaitPlayout();
        virtual bool WaitCapture();
        virtual bool StartFraming(unsigned long ms=0);
        virtual bool StopFraming();
        virtual bool WaitFraming();

        /*Volume control*/
        virtual bool GetPlayoutVolume(int*again, int*dgain = NULL);
        virtual bool SetPlayoutVolume(int again, int dgain = -1);
        virtual bool GetCaptureVolume(int*again, int*dgain = NULL);
        virtual bool SetCaptureVolume(int again, int dgain = -1);
        virtual int  GetPlayoutDelay(void);
        virtual int  GetCaptureDelay(void);

        /*thread callback*/
        virtual bool PCM_BeforePlayout(void);
        virtual bool PCM_BeforeCapture(void);
        virtual bool PCM_BeforeFraming(void);
        virtual bool PCM_AfterPlayout(void);
        virtual bool PCM_AfterCapture(void);
        virtual bool PCM_AfterFraming(void);
        virtual bool PCM_Playout(bool wait=true);
        virtual bool PCM_Capture(bool wait=true);
        virtual bool PCM_Framing(bool wait=true);

        /*data*/
    protected:

        OSAudioManager      *manager;
        int                  schduleDelay;
        OSAudioThread       *schduleThread;
        bool                 schduleStarted;
        HANDLE               schduleEvent[4];


        /*device handle*/
        OSAudioThread        *playout_thread;
        int                   playout_index;
        int                   playout_channel;
        bool                  playout_started;
        HWAVEOUT              playout_handle;

        /*buffer data for playout*/
        WaveBuffList          playout_holdList;
        WaveBuffList          playout_freeList;
        WaveBuffList          playout_busyList;
        size_t                playout_frameInTick;
        size_t                playout_frameInByte;

        OSAudioThread        *capture_thread;
        int                   capture_index;
        int                   capture_channel;
        bool                  capture_started;
        HWAVEIN               capture_handle;

        /*buffer data for capture*/
        WaveBuffList          capture_holdList;
        WaveBuffList          capture_freeList;
        WaveBuffList          capture_busyList;
        size_t                capture_frameInTick;
        size_t                capture_frameInByte;
    };

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    bool OSAudioDevice::CreatePlatformDevice(OSAudioDevice *&pdev,OSAudioManager& mng)
    {
        pdev = new OSAudioWin32(mng);
        return pdev!=NULL;
    }

    bool OSAudioDevice::DestroyPlatformDevice(OSAudioDevice *&pdev)
    {
        OSAudioWin32 * pwin32 = dynamic_cast<OSAudioWin32*>(pdev);
        if(pwin32)
        {
            delete pwin32;
            pdev = NULL;
            return true;
        }
        else
        {
            return false;
        }
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    OSAudioWin32::OSAudioWin32(OSAudioManager& mng)
    {
        /*
        * construct
        */
        manager = &mng;

        /*
        * 0.5 frame delay at most!!5ms typically!
        */
        schduleDelay = (1000/mng.GetHz())*3/2;
        if(schduleDelay<=0)
            schduleDelay = 10+5;
        schduleThread       = new OSAudioThread(this,OSAudioThread::TASK_FRAMING);
        schduleStarted      = false;

        for (size_t i=0;i<TABLESIZE(schduleEvent);i++)
        {
            schduleEvent[i] = CreateEvent(NULL,0,0,NULL);
        }

        playout_thread      = new OSAudioThread(this,OSAudioThread::TASK_PLAYOUT);
        playout_handle      = NULL;
        playout_started     = false;
        playout_frameInTick = 0;
        playout_frameInByte = 0;

        capture_thread      = new OSAudioThread(this,OSAudioThread::TASK_CAPTURE);
        capture_handle      = NULL;
        capture_started     = false;
        capture_frameInTick = 0;
        capture_frameInByte = 0;

        /*
        * alloc frame buffer as max-prossible!
        */
        for (int i=0;i<MAX_BUFHDR;i++)
        {
            WAVEHDR * new_buff;

            new_buff = (WAVEHDR *)malloc(sizeof(WAVEHDR)+MAX_BUFFER);
            assert(new_buff!=NULL);
            playout_holdList.push_back(new_buff);

            new_buff = (WAVEHDR *)malloc(sizeof(WAVEHDR)+MAX_BUFFER);
            assert(new_buff!=NULL);
            capture_holdList.push_back(new_buff);
        }

        Init();
    }

    OSAudioWin32::~OSAudioWin32()
    {
        StopFraming();
        StopPlayout();
        StopCapture();

        for (size_t i=0;i<TABLESIZE(schduleEvent);i++)
        {
            CloseHandle(schduleEvent[i]);
        }

        for (WaveBuffList::iterator it=playout_holdList.begin();it!=playout_holdList.end();it++)
        {
            free(*it);
        }
        for (WaveBuffList::iterator it=capture_holdList.begin();it!=capture_holdList.end();it++)
        {
            free(*it);
        }
    }

    bool OSAudioWin32::Init()
    {
        assert(manager!=NULL);

        playout_index   = manager->GetDev();
        playout_channel = manager->GetChan();
        capture_index   = manager->GetDev();
        capture_channel = manager->GetChan();

        if(playout_index<0)
            playout_index = WAVE_MAPPER;
        if(capture_index<0)
            capture_index = 0;

        return true;
    }

    int  OSAudioWin32::GetPlayoutDevices()
    {
        return waveOutGetNumDevs();
    }

    int  OSAudioWin32::GetCaptureDevices()
    {
        return waveInGetNumDevs();
    }

    int  OSAudioWin32::GetPlayoutDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN])
    {
        assert(index>=0 && index<GetPlayoutDevices());

        WAVEOUTCAPS caps;

        if(waveOutGetDevCaps(index,&caps,sizeof(WAVEOUTCAPS))==0)
        {
            if(name)
                strncpy(name,caps.szPname,MAX_DEVNAME);
            if(guid)
                strncpy(guid,caps.szPname,MAX_GUIDLEN);

            return true;
        }

        return false;
    }

    int  OSAudioWin32::GetCaptureDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN])
    {
        assert(index>=0 && index<GetCaptureDevices());

        WAVEINCAPS caps;

        if(waveInGetDevCaps(index,&caps,sizeof(WAVEINCAPS))==0)
        {
            if(name)
                strncpy(name,caps.szPname,MAX_DEVNAME);
            if(guid)
                strncpy(guid,caps.szPname,MAX_GUIDLEN);

            return true;
        }

        return false;
    }

    bool OSAudioWin32::IsPlayoutOpen()
    {
        return playout_handle!=NULL;
    }

    bool OSAudioWin32::IsCaptureOpen()
    {
        return capture_handle!=NULL;
    }

    bool OSAudioWin32::IsPlayoutRunning()
    {
        return playout_started;
    }
    bool OSAudioWin32::IsCaptureRunning()
    {
        return capture_started;
    }

    bool OSAudioWin32::OpenPlayoutDevice()
    {
        ClosePlayoutDevice();

        WAVEFORMATEX waveFormat;

        /*do open*/
        waveFormat.wFormatTag      = WAVE_FORMAT_PCM;
        waveFormat.nChannels       = playout_channel;
        waveFormat.nSamplesPerSec  = manager->GetFreq();
        waveFormat.wBitsPerSample  = manager->GetBitW();
        waveFormat.nBlockAlign     = waveFormat.nChannels * (waveFormat.wBitsPerSample/8);
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize          = 0;

        if(waveOutOpen(&playout_handle,
            playout_index,
            &waveFormat,
            (DWORD_PTR)schduleEvent[SEM_PLAYOUT],
            0,
            CALLBACK_EVENT)!=MMSYSERR_NOERROR)
        {
            printf("OpenPlayoutDevice - waveOutOpen error!\n");
            return false;
        }

        playout_frameInTick  = (manager->GetFreq()/manager->GetHz())*capture_channel;
        playout_frameInByte  = playout_frameInTick*(manager->GetBitW()/8);

        /*must set default volume*/
        SetPlayoutVolume(MAX_VOLUME);
        return true;
    }

    bool OSAudioWin32::OpenCaptureDevice()
    {
        CloseCaptureDevice();

        WAVEFORMATEX waveFormat;

        waveFormat.wFormatTag      = WAVE_FORMAT_PCM;
        waveFormat.nChannels       = capture_channel;
        waveFormat.nSamplesPerSec  = manager->GetFreq();
        waveFormat.wBitsPerSample  = manager->GetBitW();
        waveFormat.nBlockAlign     = waveFormat.nChannels * (waveFormat.wBitsPerSample/8);
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize          = 0;

        if(waveInOpen(&capture_handle,capture_index, &waveFormat, (DWORD_PTR)schduleEvent[SEM_CAPTURE], 0,CALLBACK_EVENT)!=0)
        {
            printf("OpenCaptureDevice - waveInOpen error!\n");
            return false;
        }

        capture_frameInTick  = (manager->GetFreq()/manager->GetHz())*capture_channel;
        capture_frameInByte  = capture_frameInTick*(manager->GetBitW()/8);

        /*must set default volume*/
        SetCaptureVolume(MAX_VOLUME);
        return true;
    }

    bool OSAudioWin32::ClosePlayoutDevice()
    {
        if(playout_handle)
        {
            PCM_AfterPlayout();
            waveOutClose(playout_handle);
        }
        playout_handle = NULL;

        return false;
    }

    bool OSAudioWin32::CloseCaptureDevice()
    {
        if(capture_handle)
        {
            PCM_AfterCapture();
            waveInClose(capture_handle);
        }
        capture_handle = NULL;

        return false;
    }

    bool OSAudioWin32::StartPlayout(unsigned long ms)
    {
        /*make sure it's opened*/
        if(!IsPlayoutOpen())
            if(!OpenPlayoutDevice())
                return false;

        assert(playout_handle!=NULL);

        /*start thread*/
        playout_started = true;
        return playout_thread->Load(ms);
    }

    bool OSAudioWin32::StartCapture(unsigned long ms)
    {
        /*make sure it's opened*/
        if(!IsCaptureOpen())
            if(!OpenCaptureDevice())
                return false;

        assert(capture_handle!=NULL);

        /*start thread*/
        capture_started = true;
        return  capture_thread->Load(ms);
    }

    bool OSAudioWin32::StopPlayout()
    {
        assert(playout_thread!=NULL);

        playout_started = false;

        /*kill thread*/
        playout_thread->Term();

        /*close device*/
        ClosePlayoutDevice();

        return true;
    }

    bool OSAudioWin32::StopCapture()
    {
        assert(capture_thread!=NULL);

        capture_started = false;

        /*kill thread*/
        capture_thread->Term();

        /*close device*/
        CloseCaptureDevice();

        return true;
    }

    bool OSAudioWin32::WaitPlayout()
    {
        if(playout_thread!=NULL)
            return playout_thread->Wait();
        else
            return false;
    }
    bool OSAudioWin32::WaitCapture()
    {
        if(capture_thread!=NULL)
            return capture_thread->Wait();
        else
            return false;
    }

    bool OSAudioWin32::StartFraming(unsigned long ms)
    {
        /*make sure it's opened*/
        if(!IsCaptureOpen())
            if(!OpenCaptureDevice())
                return false;
        if(!IsPlayoutOpen())
            if(!OpenPlayoutDevice())
                return false;

        assert(playout_handle!=NULL);
        assert(capture_handle!=NULL);

        /*start thread*/
        schduleStarted = true;
        return schduleThread->Load(ms);
    }

    bool OSAudioWin32::StopFraming()
    {
        assert(schduleThread!=NULL);

        schduleStarted = false;

        /*kill thread*/
        schduleThread->Term();

        /*close device*/
        StopCapture();
        StopPlayout();
        return true;
    }

    bool OSAudioWin32::WaitFraming()
    {
        if(schduleThread!=NULL)
            return schduleThread->Wait();
        else
            return false;
    }

    /*Volume control*/
    bool OSAudioWin32::GetPlayoutVolume(int*again, int*dgain)
    {
        assert(playout_handle!=NULL);
		assert(again != NULL);

        DWORD volume;
        waveOutGetVolume(playout_handle,&volume);
        *again = static_cast<int>(MIN_VOLUME+((float)volume/(0xFFFFFFFF-1))*MAX_VOLUME);
        return true;
    }

    bool OSAudioWin32::SetPlayoutVolume(int again, int dgain)
    {
        assert(playout_handle!=NULL);
        DWORD volume = static_cast<DWORD>((int)(0xFFFFFFFF-1)*(((float)again-MIN_VOLUME)/MAX_VOLUME));

        return waveOutSetVolume(playout_handle,volume)==0;
    }

    bool OSAudioWin32::GetCaptureVolume(int*again, int*dgain)
    {
        return true;
    }

    bool OSAudioWin32::SetCaptureVolume(int again, int dgain)
    {
        return true;
    }
    int  OSAudioWin32::GetPlayoutDelay(void)
    {
        return playout_freeList.size()*10;
    }
    int  OSAudioWin32::GetCaptureDelay(void)
    {
        return capture_freeList.size()*10;
    }
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    bool OSAudioWin32::PCM_BeforePlayout(void)
    {
        assert(playout_handle!=NULL);

        /*
        * prepare buffer
        * put half(50%) buffer to ready as dummmy frame at beginning!
        */
        playout_freeList.clear();
        playout_busyList.clear();

        for (WaveBuffList::iterator it = playout_holdList.begin();it != playout_holdList.end();it++)
        {
            WAVEHDR * phdr = *it;

            /*fill silence sample*/
            memset(phdr,0,sizeof(WAVEHDR)+playout_frameInByte);
            phdr->lpData         = (char*)(phdr+1);
            phdr->dwBufferLength = playout_frameInByte;
            phdr->dwBytesRecorded= 0;

            if(playout_busyList.size() < (MAX_BUFHDR/2))
            {
                if(waveOutPrepareHeader(playout_handle,phdr,sizeof(WAVEHDR))!=0)
                {
                    printf("PCM_AfterFraming - waveOutPrepareHeader error!\n");
                    break;
                }

                if(waveOutWrite(playout_handle,phdr,sizeof(WAVEHDR))!=0)
                {
                    printf("PCM_AfterFraming - waveOutWrite error!\n");
                    break;
                }
                playout_busyList.push_back(phdr);
            }
            else
            {
                playout_freeList.push_back(phdr);
            }
        }

        /*
        * start device
        */
        ResetEvent(schduleEvent[SEM_PLAYOUT]);
        if(waveOutRestart(playout_handle)!=0)
        {
            printf("StartPlayout - waveOutRestart error!\n");
            return false;
        }

        return true;
    }

    bool OSAudioWin32::PCM_BeforeCapture(void)
    {
        assert(capture_handle!=NULL);

        /*
        * prepare buffer
        * put half(50%) buffer to ready!
        */
        capture_freeList.clear();
        capture_busyList.clear();

        for (WaveBuffList::iterator it = capture_holdList.begin();it != capture_holdList.end();it++)
        {
            WAVEHDR * phdr = *it;

            /*fill silence sample*/
            memset(phdr,0,sizeof(WAVEHDR)+capture_frameInByte);
            phdr->lpData         = (char*)(phdr+1);
            phdr->dwBufferLength = capture_frameInByte;
            phdr->dwBytesRecorded= 0;

            if(waveInPrepareHeader(capture_handle,phdr,sizeof(WAVEHDR))!=0)
            {
                printf("OpenCaptureDevice - waveInPrepareHeader error!\n");
                return false;
            }
            if(waveInAddBuffer(capture_handle,phdr,sizeof(WAVEHDR))!=0)
            {
                printf("OpenCaptureDevice - waveInAddBuffer error!\n");
                return false;
            }

            capture_busyList.push_back(phdr);
        }

        /*
        * start device
        */
        ResetEvent(schduleEvent[SEM_CAPTURE]);
        if(waveInStart(capture_handle)!=0)
        {
            printf("StartPlayout - waveInStart error!\n");
            return false;
        }
        return true;
    }

    bool OSAudioWin32::PCM_BeforeFraming(void)
    {
        assert(playout_handle!=NULL);
        assert(capture_handle!=NULL);

        PCM_BeforeCapture();
        PCM_BeforePlayout();

        return true;
    }

    bool OSAudioWin32::PCM_AfterPlayout(void)
    {
        for (WaveBuffList::iterator it = playout_busyList.begin();it!=playout_busyList.end();it++)
        {
            waveOutUnprepareHeader(playout_handle,*it,sizeof(WAVEHDR));
        }
        waveOutReset(playout_handle);
        playout_freeList.clear();
        playout_busyList.clear();
        return true;
    }

    bool OSAudioWin32::PCM_AfterCapture(void)
    {
        waveInStop(capture_handle);
        for (WaveBuffList::iterator it = capture_busyList.begin();it!=capture_busyList.end();it++)
        {
            waveInUnprepareHeader(capture_handle,*it,sizeof(WAVEHDR));
        }
        capture_freeList.clear();
        capture_busyList.clear();
        return true;
    }

    bool OSAudioWin32::PCM_AfterFraming(void)
    {
        PCM_AfterPlayout();
        PCM_AfterCapture();

        return true;
    }

    bool OSAudioWin32::PCM_Playout(bool wait)
    {
        assert(playout_handle!=NULL);

        /*
        * wait
        */
        if(playout_busyList.size()>0)
        {
            DWORD waitResult = WaitForSingleObject(schduleEvent[SEM_PLAYOUT],schduleDelay);
            switch (waitResult)
            {
            case WAIT_OBJECT_0 + 0:
                break;
            case WAIT_TIMEOUT:
                //printf("PCM_Playout - Wait timeout!\n");
                break;
            default:
                printf("PCM_Playout - Wait error!\n");
                return false;
            }
        }

        /*
        * handle busy buffer
        */
        while (playout_busyList.size())
        {
            WAVEHDR * phdr = *playout_busyList.begin();

            if(phdr->dwFlags & WHDR_DONE)
            {
                if(waveOutUnprepareHeader(playout_handle,phdr,sizeof(WAVEHDR))!=0)
                {
                    printf("PCM_Playout - waveOutUnprepareHeader error!\n");
                    break;
                }

                /*
                * switch to free list
                */
                playout_busyList.pop_front();
                playout_freeList.push_back(phdr);
            }
            else
            {
                break;
            }
        }

        /*
        * handle free buffer
        */
        while(playout_freeList.size()>0)
        {
            WAVEHDR * phdr = *playout_freeList.begin();

            assert(playout_busyList.size()>0);

            /*
            * get more one frame
            */
            memset(phdr,0,sizeof(WAVEHDR));
            phdr->lpData         = (char*)(phdr+1);
            phdr->dwBufferLength = playout_frameInByte;

            if(manager->PlayoutPCM(phdr->lpData,playout_frameInTick)<0)
                return false;

            /*
            * put back buffer
            */
            if(waveOutPrepareHeader(playout_handle,phdr,sizeof(WAVEHDR))!=0)
            {
                printf("PCM_Playout - waveOutPrepareHeader error!\n");
                break;
            }

            if(waveOutWrite(playout_handle,phdr,sizeof(WAVEHDR))!=0)
            {
                printf("PCM_Playout - waveOutWrite error!\n");
                break;
            }

            /*
            * switch to busy list
            */
            playout_freeList.pop_front();
            playout_busyList.push_back(phdr);
        }

        return true;
    }

    bool OSAudioWin32::PCM_Capture(bool wait)
    {
        assert(capture_handle!=NULL);

        /*
        * wait
        */
        if(capture_busyList.size()>0)
        {
            DWORD waitResult = WaitForSingleObject(schduleEvent[SEM_CAPTURE],schduleDelay);
            switch (waitResult)
            {
            case WAIT_OBJECT_0 + 0:
                break;
            case WAIT_TIMEOUT:
                //printf("PCM_Capture - Wait timeout!\n");
                return true;
            default:
                printf("PCM_Capture - Wait error!\n");
                return false;
            }
        }

        /*
        * handle busy buffer
        */
        while (capture_busyList.size()>0)
        {
            WAVEHDR * phdr = *capture_busyList.begin();

            if(phdr->dwFlags & WHDR_DONE)
            {
                if(waveInUnprepareHeader(capture_handle,phdr,sizeof(WAVEHDR))!=0)
                {
                    printf("PCM_Capture - waveInUnprepareHeader error!\n");
                    break;
                }

                assert(phdr->dwBufferLength == phdr->dwBytesRecorded);

                /*
                * switch to free list
                */
                capture_busyList.pop_front();
                capture_freeList.push_back(phdr);
            }
            else
            {
                break;
            }
        }

        /*
        * handle free buffer
        */
        while (capture_freeList.size())
        {
            WAVEHDR * phdr = *capture_freeList.begin();

            /*
            * got one frame
            */
            assert(phdr->dwBytesRecorded==capture_frameInByte);
            assert(capture_busyList.size()>0);

            if(manager->CapturePCM(phdr->lpData,capture_frameInTick)<0)
                return false;

            /*
            * put back buffer
            */
            if(waveInPrepareHeader(capture_handle,phdr,sizeof(WAVEHDR))!=0)
            {
                printf("PCM_Capture - waveInPrepareHeader error!\n");
                break;
            }

            if(waveInAddBuffer(capture_handle,phdr,sizeof(WAVEHDR))!=0)
            {
                printf("PCM_Capture - waveInAddBuffer error!\n");
                break;
            }
            /*
            * switch to busy list
            */
            capture_freeList.pop_front();
            capture_busyList.push_back(phdr);
        }

        return true;
    }

    bool OSAudioWin32::PCM_Framing(bool wait)
    {
        assert(playout_handle!=NULL);
        assert(capture_handle!=NULL);

        /*
        * wait
        */
        if(playout_busyList.size()>0 || capture_busyList.size()>0)
        {
            DWORD waitResult = WaitForMultipleObjects(2,schduleEvent,false,schduleDelay);
            switch (waitResult)
            {
            case WAIT_OBJECT_0 + 0:
            case WAIT_OBJECT_0 + 1:
                break;
            case WAIT_TIMEOUT:
                //printf("PCM_Framing - Wait timeout!\n");
                break;
            default:
                printf("PCM_Framing - Wait error!\n");
                return false;
            }
        }


        /*
        * handle capture-busy buffer
        */
        while (capture_busyList.size()>0)
        {
            WAVEHDR * phdr = *capture_busyList.begin();

            if(phdr->dwFlags & WHDR_DONE)
            {
                if(waveInUnprepareHeader(capture_handle,phdr,sizeof(WAVEHDR))!=0)
                {
                    printf("PCM_Framing - waveInUnprepareHeader error!\n");
                    break;
                }

                assert(phdr->dwBufferLength == phdr->dwBytesRecorded);

                /*
                * switch to free list
                */
                capture_busyList.pop_front();
                capture_freeList.push_back(phdr);
            }
            else
            {
                break;
            }
        }

        /*
        * handle playout-busy buffer
        */
        while (playout_busyList.size())
        {
            WAVEHDR * phdr = *playout_busyList.begin();

            if(phdr->dwFlags & WHDR_DONE)
            {
                if(waveOutUnprepareHeader(playout_handle,phdr,sizeof(WAVEHDR))!=0)
                {
                    printf("PCM_Framing - waveOutUnprepareHeader error!\n");
                    break;
                }

                /*
                * switch to free list
                */
                playout_busyList.pop_front();
                playout_freeList.push_back(phdr);
            }
            else
            {
                break;
            }
        }

        /*
        * handle free buffer
        */
        while(playout_freeList.size()>0 && capture_freeList.size()>0)
        {
            WAVEHDR * phdr_p = *playout_freeList.begin();
            WAVEHDR * phdr_c = *capture_freeList.begin();

            //assert(playout_busyList.size()>0);
            //assert(capture_busyList.size()>0);

            /*
            * get more one frame
            */
            memset(phdr_p,0,sizeof(WAVEHDR));
            phdr_p->lpData         = (char*)(phdr_p+1);
            phdr_p->dwBufferLength = playout_frameInByte;

            memset(phdr_c,0,sizeof(WAVEHDR));
            phdr_c->lpData         = (char*)(phdr_c+1);
            phdr_c->dwBufferLength = capture_frameInByte;

            /*
            * get one frame
            */
            assert(playout_frameInByte==capture_frameInByte);
            assert(playout_frameInTick==capture_frameInTick);

            if(manager->FramesPCM(phdr_p->lpData,phdr_c->lpData,playout_frameInTick)<0)
                return false;

            /*
            * put back playout-buffer
            */
            if(waveOutPrepareHeader(playout_handle,phdr_p,sizeof(WAVEHDR))!=0)
            {
                printf("PCM_Framing - waveOutPrepareHeader error!\n");
                break;
            }

            if(waveOutWrite(playout_handle,phdr_p,sizeof(WAVEHDR))!=0)
            {
                printf("PCM_Framing - waveOutWrite error!\n");
                break;
            }
            playout_freeList.pop_front();
            playout_busyList.push_back(phdr_p);

            /*
            * put back capture-buffer
            */
            if(waveInPrepareHeader(capture_handle,phdr_c,sizeof(WAVEHDR))!=0)
            {
                printf("PCM_Framing - waveInPrepareHeader error!\n");
                break;
            }

            if(waveInAddBuffer(capture_handle,phdr_c,sizeof(WAVEHDR))!=0)
            {
                printf("PCM_Framing - waveInAddBuffer error!\n");
                break;
            }
            capture_freeList.pop_front();
            capture_busyList.push_back(phdr_c);
        }

        return true;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};

