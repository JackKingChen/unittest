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
#ifdef HAVE_ALSA

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
#include <sys/time.h>

#include <alsa/asoundlib.h>
#include <sys/soundcard.h>

#include "host.h"
#include "device_audio.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace host
{
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    class OSAudioALSA;
    class OSAudioThread;

    struct BUFFHDR 
    {
        size_t frameTotal;  /*in samples,including all channels*/
        size_t frameLeft;   /*in samples*/
        char   data[4];
    };

    typedef std::list<BUFFHDR*> WaveBuffList;

    /*
    * class of OSAudioALSA
    */
    class OSAudioALSA:public OSAudioDevice
    {
    public:
        enum
        {
            FUNC_GET_DEVICE_COUNT            =0,
            FUNC_GET_DEVICE_NAME             =1,
            FUNC_GET_DEVICE_NAME_FOR_AN_ENUM =2,
        };
        enum
        {
            /**/
            MAX_LATENCY = 40*1000,

            /*on frame for buffer ,and 50ms delay max*/
            MAX_BUFHDR  = 5,
            /*96000,10Hz,100ms,32bit sample*/
            MAX_BUFFER  = (96000/10)*sizeof(int),
        };

    public:

        OSAudioALSA(OSAudioManager& mng);
        ~OSAudioALSA();

    public:
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

    protected:
        int  ALSA_GetControlName(char *controlName,const char *deviceName);
        int  ALSA_GetDevicesInfo(int fun,bool playback,int enumDeviceNo=0,char* enumDeviceName=NULL,int ednLen=0);
        int  ALSA_ErrorRecovery(snd_pcm_t* deviceHandle,int error,bool restart=true);

        int  ALSA_Open (bool playout);
        int  ALSA_Close(bool playout);

        int  ALSA_MixerOpen (bool playout);
        int  ALSA_MixerClose(bool playout);

        bool ALSA_MixerGetVolume(bool playout,int&step);
        bool ALSA_MixerSetVolume(bool playout,int step);

        /*data*/
    protected:

        OSAudioManager  *manager;
        int              schduleDelay;
        OSAudioThread   *schduleThread;
        bool             schduleStarted;

        OSAudioThread   *playout_thread;
        snd_pcm_t       *playout_handle;
        snd_mixer_t     *playout_mixer;
        snd_mixer_elem_t*playout_mixerElement;
        int              playout_index;
        int              playout_channel;
        char             playout_name[MAX_DEVNAME];
        bool             playout_started;

        /*buffer data for playout*/
        WaveBuffList     playout_holdList;
        WaveBuffList     playout_freeList;
        WaveBuffList     playout_busyList;
        size_t           playout_frameInTick;
        size_t           playout_frameInByte;

        OSAudioThread   *capture_thread;
        snd_pcm_t       *capture_handle;
        snd_mixer_t     *capture_mixer;
        snd_mixer_elem_t*capture_mixerElement;
        int              capture_index;
        int              capture_channel;
        char             capture_name[MAX_DEVNAME];
        bool             capture_started;

        /*buffer data for capture*/
        WaveBuffList     capture_holdList;
        WaveBuffList     capture_freeList;
        WaveBuffList     capture_busyList;
        size_t           capture_frameInTick;
        size_t           capture_frameInByte;
    };

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    bool OSAudioDevice::CreatePlatformDevice(OSAudioDevice *&pdev,OSAudioManager& mng)
    {
        pdev = new OSAudioALSA(mng);
        return pdev!=NULL;
    }

    bool OSAudioDevice::DestroyPlatformDevice(OSAudioDevice *&pdev)
    {
        OSAudioALSA * palsa = dynamic_cast<OSAudioALSA*>(pdev);
        if(palsa)
        {
            delete palsa;
            pdev = NULL;
            return true;
        }
        else
        {
            return false;
        }
    }

    /************************************************************************/
    /* class fun                                                           */
    /************************************************************************/
    OSAudioALSA::OSAudioALSA(OSAudioManager& mng)
    {
        manager = &mng;

        schduleDelay = (1000/mng.GetHz())/2;
        if(schduleDelay<=0)
            schduleDelay = 5;
        schduleThread       = new OSAudioThread(this,OSAudioThread::TASK_FRAMING);
        schduleStarted      = false;

        playout_thread      = new OSAudioThread(this,OSAudioThread::TASK_PLAYOUT);
        playout_handle      = NULL;
        memset(playout_name,0,sizeof(playout_name));
        playout_started     = false;
        playout_frameInTick = 0;
        playout_frameInByte = 0;
        playout_mixer       = NULL;
        playout_mixerElement= NULL;

        capture_thread      = new OSAudioThread(this,OSAudioThread::TASK_CAPTURE);
        capture_handle      = NULL;
        memset(capture_name,0,sizeof(capture_name));
        capture_started     = false;
        capture_frameInTick = 0;
        capture_frameInByte = 0;
        capture_mixer       = NULL;
        capture_mixerElement= NULL;

        /*
        * alloc frame buffer as max-prossible!
        */
        for (int i=0;i<MAX_BUFHDR;i++)
        {
            BUFFHDR * new_buff;

            new_buff = (BUFFHDR *)malloc(sizeof(BUFFHDR)+MAX_BUFFER);
            assert(new_buff!=NULL);
            playout_holdList.push_back(new_buff);

            new_buff = (BUFFHDR *)malloc(sizeof(BUFFHDR)+MAX_BUFFER);
            assert(new_buff!=NULL);
            capture_holdList.push_back(new_buff);
        }

        Init();
    }

    OSAudioALSA::~OSAudioALSA()
    {
        StopFraming();
        StopPlayout();
        StopCapture();

        for (WaveBuffList::iterator it=playout_holdList.begin();it!=playout_holdList.end();it++)
        {
            free(*it);
        }
        for (WaveBuffList::iterator it=capture_holdList.begin();it!=capture_holdList.end();it++)
        {
            free(*it);
        }
    }

    bool OSAudioALSA::Init()
    {
        assert(manager!=NULL);

        playout_index   = manager->GetDev();
        playout_channel = manager->GetChan();
        capture_index   = manager->GetDev();
        capture_channel = manager->GetChan();

        return true;
    }

    int  OSAudioALSA::GetPlayoutDevices()
    {
        return ALSA_GetDevicesInfo(FUNC_GET_DEVICE_COUNT, true);
    }

    int  OSAudioALSA::GetCaptureDevices()
    {
        return ALSA_GetDevicesInfo(FUNC_GET_DEVICE_COUNT, false);
    }

    int  OSAudioALSA::GetPlayoutDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN])
    {
        int nDevices;

        nDevices = GetPlayoutDevices();
        if ((index > (nDevices-1)) || (name == NULL))
            return -1;

        memset(name, 0, MAX_DEVNAME);

        if (guid != NULL)
            memset(guid, 0, MAX_DEVNAME);

        return ALSA_GetDevicesInfo(FUNC_GET_DEVICE_NAME, true, index, name, MAX_DEVNAME);
    }

    int  OSAudioALSA::GetCaptureDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN])
    {
        int nDevices;

        nDevices = GetPlayoutDevices();
        if ((index > (nDevices-1)) || (name == NULL))
            return -1;

        memset(name, 0, MAX_DEVNAME);

        if (guid != NULL)
            memset(guid, 0, MAX_DEVNAME);

        return ALSA_GetDevicesInfo(FUNC_GET_DEVICE_NAME, false, index, name, MAX_DEVNAME);
    }

    bool OSAudioALSA::IsPlayoutOpen()
    {
        return playout_handle!=NULL;
    }

    bool OSAudioALSA::IsCaptureOpen()
    {
        return capture_handle!=NULL;
    }

    bool OSAudioALSA::IsPlayoutRunning()
    {
        return playout_started;
    }
    bool OSAudioALSA::IsCaptureRunning()
    {
        return capture_started;
    }

    bool OSAudioALSA::OpenPlayoutDevice()
    {
        ClosePlayoutDevice();
        if(ALSA_Open(true)==0)
        {
            return ALSA_MixerOpen(true)==0;
        }
        return false;
    }

    bool OSAudioALSA::OpenCaptureDevice()
    {
        CloseCaptureDevice();
        if(ALSA_Open(false)==0)
        {
            return ALSA_MixerOpen(false)==0;
        }
        return false;
    }

    bool OSAudioALSA::ClosePlayoutDevice()
    {
        ALSA_MixerClose(true);
        return ALSA_Close(true)==0;
    }

    bool OSAudioALSA::CloseCaptureDevice()
    {
        ALSA_MixerClose(false);
        return ALSA_Close(false)==0;
    }

    bool OSAudioALSA::StartPlayout(unsigned long ms)
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

    bool OSAudioALSA::StartCapture(unsigned long ms)
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

    bool OSAudioALSA::StopPlayout()
    {
        assert(playout_thread!=NULL);

        playout_started = false;

        /*kill thread*/
        playout_thread->Term();

        /*close device*/
        ClosePlayoutDevice();

        return true;
    }

    bool OSAudioALSA::StopCapture()
    {
        assert(capture_thread!=NULL);

        capture_started = false;

        /*kill thread*/
        capture_thread->Term();

        /*close device*/
        CloseCaptureDevice();

        return true;
    }

    bool OSAudioALSA::WaitPlayout()
    {
        if(playout_thread!=NULL)
            return playout_thread->Wait();
        else
            return false;
    }
    bool OSAudioALSA::WaitCapture()
    {
        if(capture_thread!=NULL)
            return capture_thread->Wait();
        else
            return false;
    }

    bool OSAudioALSA::StartFraming(unsigned long ms)
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

    bool OSAudioALSA::StopFraming()
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

    bool OSAudioALSA::WaitFraming()
    {
        if(schduleThread!=NULL)
            return schduleThread->Wait();
        else
            return false;
    }

    /*Volume control*/
    bool OSAudioALSA::GetPlayoutVolume(int *again, int *dgain)
    {
		assert(again);
        return ALSA_MixerGetVolume(true, *again);
    }

    bool OSAudioALSA::SetPlayoutVolume(int again, int dgain)
    {
        return ALSA_MixerSetVolume(true, again);
    }

    bool OSAudioALSA::GetCaptureVolume(int*again, int*dgain)
    {
		assert(again);
        return ALSA_MixerGetVolume(false, *again);
    }

    bool OSAudioALSA::SetCaptureVolume(int again, int dgain)
    {
        return ALSA_MixerSetVolume(false, again);
    }
    int  OSAudioALSA::GetPlayoutDelay(void)
    {
        return playout_busyList.size()*10;
    }
    int  OSAudioALSA::GetCaptureDelay(void)
    {
        return capture_busyList.size()*10;
    }
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    int  OSAudioALSA::ALSA_GetControlName(char *controlName,const char *deviceName)
    {
        // Example
        // deviceName: "front:CARD=Intel,DEV=0"
        // controlName: "hw:CARD=Intel"
        const char* pos1 = strchr(deviceName, ':');
        const char* pos2 = strchr(deviceName, ',');
        if (!pos2)
        {
            // Can also be default:CARD=Intel
            pos2 = &deviceName[strlen(deviceName)];
        }
        if (pos1 && pos2)
        {
            strcpy(controlName, "hw");
            int nChar = (int) (pos2 - pos1);
            strncpy(&controlName[2], pos1, nChar);
            controlName[2 + nChar] = '\0';
        } else
        {
            strcpy(controlName, deviceName);
        }
        return 0;
    }

    int  OSAudioALSA::ALSA_GetDevicesInfo(int fun,bool playback,int enumDeviceNo,char* enumDeviceName,int ednLen)
    {
        // Device enumeration based on libjingle implementation
        // by Tristan Schmelcher at Google Inc.

        const char *type = playback ? "Output" : "Input";
        // dmix and dsnoop are only for playback and capture, respectively, but ALSA
        // stupidly includes them in both lists.
        const char *ignorePrefix = playback ? "dsnoop:" : "dmix:" ;
        // (ALSA lists many more "devices" of questionable interest, but we show them
        // just in case the weird devices may actually be desirable for some
        // users/systems.)

        int err;
        int enumCount(0);
        bool keepSearching(true);

        // From Chromium issue 95797
        // Loop through the sound cards to get Alsa device hints.
        // Don't use snd_device_name_hint(-1,..) since there is a access violation
        // inside this ALSA API with libasound.so.2.0.0.
        int card = -1;
        while (!(snd_card_next(&card)) && (card >= 0) && keepSearching)
        {
            void **hints;

            err = snd_device_name_hint(card, "pcm", &hints);
            if (err != 0)
            {
                printf("ALSA_GetDevicesInfo - device name hint error: %s\n",snd_strerror(err));
                return -1;
            }

            enumCount++; // default is 0
            if ((fun == FUNC_GET_DEVICE_NAME || fun == FUNC_GET_DEVICE_NAME_FOR_AN_ENUM)
                && enumDeviceNo == 0)
            {
                strcpy(enumDeviceName, "default");

                err = snd_device_name_free_hint(hints);
                if (err != 0)
                {
                    printf("ALSA_GetDevicesInfo - device name free hint error: %s\n",snd_strerror(err));
                }

                return 0;
            }

            for (void **list = hints; *list != NULL; ++list)
            {
                char *actualType = snd_device_name_get_hint(*list, "IOID");
                if (actualType)
                {   // NULL means it's both.
                    bool wrongType = (strcmp(actualType, type) != 0);
                    free(actualType);
                    if (wrongType)
                    {
                        // Wrong type of device (i.e., input vs. output).
                        continue;
                    }
                }

                char *name = snd_device_name_get_hint(*list, "NAME");
                if (!name)
                {
                    printf("ALSA_GetDevicesInfo - Device has no name\n");
                    // Skip it.
                    continue;
                }

                // Now check if we actually want to show this device.
                if (strcmp(name, "default") != 0 &&
                    strcmp(name, "null") != 0 &&
                    strcmp(name, "pulse") != 0 &&
                    strncmp(name, ignorePrefix, strlen(ignorePrefix)) != 0)
                {
                    // Yes, we do.
                    char *desc = snd_device_name_get_hint(*list, "DESC");
                    if (!desc)
                    {
                        // Virtual devices don't necessarily have descriptions.
                        // Use their names instead.
                        desc = name;
                    }

                    if ((FUNC_GET_DEVICE_NAME == fun) &&
                        (enumDeviceNo == enumCount))
                    {
                        // We have found the enum device, copy the name to buffer.
                        strncpy(enumDeviceName, desc, ednLen);
                        enumDeviceName[ednLen-1] = '\0';
                        keepSearching = false;
                        // Replace '\n' with '-'.
                        char * pret = strchr(enumDeviceName, '\n'/*0xa*/); //LF
                        if (pret)
                            *pret = '-';
                    }
                    if ((FUNC_GET_DEVICE_NAME_FOR_AN_ENUM == fun) &&
                        (enumDeviceNo == enumCount))
                    {
                        // We have found the enum device, copy the name to buffer.
                        strncpy(enumDeviceName, name, ednLen);
                        enumDeviceName[ednLen-1] = '\0';
                        keepSearching = false;
                    }

                    if (keepSearching)
                        ++enumCount;

                    if (desc != name)
                        free(desc);
                }

                free(name);

                if (!keepSearching)
                    break;
            }

            err = snd_device_name_free_hint(hints);
            if (err != 0)
            {
                printf("ALSA_GetDevicesInfo - device name free hint error: %s\n",snd_strerror(err));
                // Continue and return true anyway, since we did get the whole list.
            }
        }

        if (FUNC_GET_DEVICE_COUNT == fun)
        {
            if (enumCount == 1) // only default?
                enumCount = 0;
            return enumCount; // Normal return point for function 0
        }

        if (keepSearching)
        {
            // If we get here for function 1 and 2, we didn't find the specified
            // enum device.
            printf("ALSA_GetDevicesInfo - Could not find device name or numbers\n");
            return -1;
        }

        return 0;
    }

    int  OSAudioALSA::ALSA_ErrorRecovery(snd_pcm_t* deviceHandle,int error,bool restart)
    {
        printf("ALSA_ErrorRecovery - Trying to recover %s from error: %s (%d) (state %d)\n",
            (snd_pcm_stream(deviceHandle) == SND_PCM_STREAM_CAPTURE) ?"capture" : "playout",
            snd_strerror(error), error, snd_pcm_state(deviceHandle));

        // It is recommended to use snd_pcm_recover for all errors. If that function
        // cannot handle the error, the input error code will be returned, otherwise
        // 0 is returned. From snd_pcm_recover API doc: "This functions handles
        // -EINTR (4) (interrupted system call), -EPIPE (32) (playout overrun or
        // capture underrun) and -ESTRPIPE (86) (stream is suspended) error codes
        // trying to prepare given stream for next I/O."

        /** Open */
        //    SND_PCM_STATE_OPEN = 0,
        /** Setup installed */
        //    SND_PCM_STATE_SETUP,
        /** Ready to start */
        //    SND_PCM_STATE_PREPARED,
        /** Running */
        //    SND_PCM_STATE_RUNNING,
        /** Stopped: underrun (playback) or overrun (capture) detected */
        //    SND_PCM_STATE_XRUN,= 4
        /** Draining: running (playback) or stopped (capture) */
        //    SND_PCM_STATE_DRAINING,
        /** Paused */
        //    SND_PCM_STATE_PAUSED,
        /** Hardware is suspended */
        //    SND_PCM_STATE_SUSPENDED,
        //  ** Hardware is disconnected */
        //    SND_PCM_STATE_DISCONNECTED,
        //    SND_PCM_STATE_LAST = SND_PCM_STATE_DISCONNECTED

        // snd_pcm_recover isn't available in older alsa, e.g. on the FC4 machine
        // in Sthlm lab.

        int res = snd_pcm_recover(deviceHandle, error, 1);
        if (0 == res)
        {
            if ((error == -EPIPE || error == -ESTRPIPE) && restart)
            {
                // For capture streams we also have to repeat the explicit start() to get
                // data flowing again.
                int err = snd_pcm_start(deviceHandle);
                if (err != 0)
                {
                    printf("ALSA_ErrorRecovery - snd_pcm_start error:%s\n",snd_strerror(err));
                    return -1;
                }
            }

            printf("ALSA_ErrorRecovery - snd_pcm_recover OK\n");

            return 0;
        }
        else 
        {
            printf("ALSA_ErrorRecovery - Unrecoverable\n");
        }

        return res;
    }

    int  OSAudioALSA::ALSA_Close(bool playout)
    {
        snd_pcm_t *&deviceHandle = playout?playout_handle:capture_handle;

        /* Start by closing any existing wave-output devices*/
        if (deviceHandle != NULL)
        {
            if(snd_pcm_avail_update(deviceHandle)>0)
            {
                int error = snd_pcm_drop(deviceHandle);
                if (error < 0)
                {
                    printf("ALSA_Close - Error snd_pcm_drop: %s",
                        snd_strerror(error));
                }
            }
            snd_pcm_close(deviceHandle);
        }
        deviceHandle = NULL;

        return 0;
    }

    int  OSAudioALSA::ALSA_Open(bool playout)
    {
        snd_pcm_t      *&deviceHandle = playout?playout_handle:capture_handle;
        char           * deviceName   = playout?playout_name:capture_name;
        int              deviceIndex  = playout?playout_index:capture_index;
        int              channel      = playout?playout_channel:capture_channel;;
        snd_pcm_format_t format;
        int              error;
        int              samples;
        int              samples_size;
        float            latency;

        switch(manager->GetBitW())
        {
        case 16:format = SND_PCM_FORMAT_S16_LE;break;
        case 24:format = SND_PCM_FORMAT_S24_LE;break;
        case 32:format = SND_PCM_FORMAT_S32_LE;break;
        default:return -EINVAL;
        }

        /*for safe*/
        ALSA_Close(playout);

        /*open*/
        GetCaptureDeviceName(deviceIndex, deviceName,NULL);
        error = snd_pcm_open(&deviceHandle,deviceName,
            playout?SND_PCM_STREAM_PLAYBACK:SND_PCM_STREAM_CAPTURE,SND_PCM_NONBLOCK);

        if(error<0)
        {
            printf("ALSA_Open - error snd_pcm_open\n");
            return -EFAULT;
        }

        /*config*/
        error = snd_pcm_set_params(deviceHandle,
            format,                         //format
            SND_PCM_ACCESS_RW_INTERLEAVED,  //access
            channel,                        //channels
            manager->GetFreq(),             //rate
            1,                              //soft_resample
            MAX_LATENCY                     //latency in us
            );
        if (error< 0)
        {
            ALSA_Close(playout);
            printf("ALSA_Open - error snd_pcm_set_params\n");
            return -EFAULT;
        }
        /*config HW*/
        

        
        /* 
        * Set rec buffer size and create buffer
        */
        samples     = manager->GetFreq()/manager->GetHz();
        samples_size= manager->GetBitW()/8;
        latency     = (float)manager->GetLatency()/(float)(1000/manager->GetHz());
        if(playout)
        {
            playout_frameInTick  = samples;
            playout_frameInByte  = snd_pcm_frames_to_bytes(deviceHandle, samples);
        }
        else
        {
            capture_frameInTick  = samples;
            capture_frameInByte  = snd_pcm_frames_to_bytes(deviceHandle, samples);
        }

        return 0;
    }

    int  OSAudioALSA::ALSA_MixerOpen(bool playout)
    {
        snd_mixer_t     *&mixer       =playout?playout_mixer:capture_mixer;
        snd_mixer_elem_t*&mixerElement=playout?playout_mixerElement:capture_mixerElement;
        const char      * mixerDevice =playout?playout_name:capture_name;
        char              mixerName[MAX_DEVNAME];
        int               error;

        /*for safe*/
        ALSA_MixerClose(playout);

        /*
        * do open
        */
        ALSA_GetControlName(mixerName,mixerDevice);

        error = snd_mixer_open(&mixer, 0);
        if (error < 0)
        {
            printf("ALSA_MixerOpen - Error snd_mixer_open: %s\n",
                snd_strerror(error));
            return -1;
        }

        error = snd_mixer_attach(mixer, mixerName);
        if (error < 0)
        {
            printf("ALSA_MixerOpen - Error snd_mixer_attach: %s\n",
                snd_strerror(error));
            return -1;
        }

        error = snd_mixer_selem_register(mixer, NULL, NULL);
        if (error < 0)
        {
            printf("ALSA_MixerOpen - Error snd_mixer_selem_register: %s\n",
                snd_strerror(error));
            return -1;
        }

        error = snd_mixer_load(mixer);
        if (error < 0)
        {
            printf("ALSA_MixerOpen - Error snd_mixer_load: %s\n",
                snd_strerror(error));
            return -1;
        }

        snd_mixer_elem_t *elem  = NULL;
        snd_mixer_elem_t *elem1 = NULL;
        snd_mixer_elem_t *elem2 = NULL;
        snd_mixer_elem_t *elem3 = NULL;
        unsigned    mixerIdx  = 0;
        const char *selemName = NULL;

        // Find and store handles to the right mixer elements
        for (elem = snd_mixer_first_elem(mixer); elem; elem = snd_mixer_elem_next(elem), mixerIdx++)
        {
            if (snd_mixer_selem_is_active(elem))
            {
                selemName = snd_mixer_selem_get_name(elem);
                /*
                printf("ALSA_MixerOpen - %d(%s)snd_mixer_selem_get_name %d: %s\n", 
                playout,mixerName,mixerIdx,selemName);
                */
                
                if(playout)
                {
                    // "Master", "PCM", "Wave", "Master Mono", "PC Speaker", "PCM", "Wave"
                    if (strcmp(selemName, "Speaker") == 0
                        || strcmp(selemName, "PCM") == 0)
                    {
                        elem1 = elem;
                    } else if (strcmp(selemName, "Master") == 0)
                    {
                        elem2 = elem;
                    }
                }
                else
                {
                    if (strcmp(selemName, "Capture") == 0
                        || strcmp(selemName, "PCM") == 0)
                    {
                        elem1 = elem;
                    } else if (strcmp(selemName, "Mic") == 0)
                    {
                        elem2 = elem;
                    } 
                }
            }
        }

        if (!elem1 && !elem2 && !elem3)
        {
            printf("ALSA_MixerOpen - no elem we want!!\n");
            return -1;
        }

        /*
        * fixup
        */
        if(elem1)
            mixerElement = elem1;
        else if(elem2)
            mixerElement = elem2;
        else if(elem3)
            mixerElement = elem3;
        else
        {
            printf("ALSA_MixerOpen - no proper elem we want!!\n");
            return -1;
        }
        return 0;
    }

    int  OSAudioALSA::ALSA_MixerClose(bool playout)
    {
        snd_mixer_t     *&mixer       =playout?playout_mixer:capture_mixer;
        snd_mixer_elem_t*&mixerElement=playout?playout_mixerElement:capture_mixerElement;
        const char      * mixerDevice =playout?playout_name:capture_name;
        char              mixerName[MAX_DEVNAME];
        int               error;

        return 0;
        if(mixer)
        {
            ALSA_GetControlName(mixerName,mixerDevice);

            snd_mixer_free(mixer);

            error = snd_mixer_detach(mixer, mixerName);
            if (error < 0)
            {
                printf("ALSA_MixerClose - Error dsnd_mixer_detach: %s",
                    snd_strerror(error));
            }
            error = snd_mixer_close(mixer);
            if (error < 0)
            {
                printf("ALSA_MixerClose - Error snd_mixer_close: %s",
                    snd_strerror(error));
            }
        }
        mixer = NULL;
        mixerElement = NULL;

        return 0;
    }
    bool OSAudioALSA::ALSA_MixerGetVolume(bool playout,int&step)
    {
        snd_mixer_t     *&mixer       =playout?playout_mixer:capture_mixer;
        snd_mixer_elem_t*&mixerElement=playout?playout_mixerElement:capture_mixerElement;

        if(mixer && mixerElement)
        {
            long volume     = 0;
            long volume_min = 0;
            long volume_max = 0;
            int error  = 0;

            if(playout)
            {
                error = snd_mixer_selem_get_playback_volume_range(mixerElement,
                    &volume_min,
                    &volume_max);

                if(error < 0)
                    return false;

                error = snd_mixer_selem_get_playback_volume(
                    mixerElement,
                    (snd_mixer_selem_channel_id_t) 0,
                    &volume);

                if(error < 0)
                    return false;
            }
            else
            {
                error = snd_mixer_selem_get_capture_volume_range(mixerElement,
                    &volume_min,
                    &volume_max);

                if(error < 0)
                    return false;

                error = snd_mixer_selem_get_capture_volume(
                    mixerElement,
                    (snd_mixer_selem_channel_id_t) 0,
                    &volume);

                if(error < 0)
                    return false;
            }

            step = MIN_VOLUME+volume*(MAX_VOLUME-MIN_VOLUME)/(volume_max-volume_min);

            return true;
        }
        return false;
    }

    bool OSAudioALSA::ALSA_MixerSetVolume(bool playout,int step)
    {
        snd_mixer_t     *&mixer       =playout?playout_mixer:capture_mixer;
        snd_mixer_elem_t*&mixerElement=playout?playout_mixerElement:capture_mixerElement;

        if(mixer && mixerElement 
            && step>=MIN_VOLUME && step<=MAX_VOLUME)
        {
            long volume     = 0;
            long volume_min = 0;
            long volume_max = 0;
            int  error  = 0;

            if(playout)
            {
                error = snd_mixer_selem_get_playback_volume_range(mixerElement,
                    &volume_min,
                    &volume_max);

                if(error < 0)
                {
                    printf("ALSA_MixerSetVolume - snd_mixer_selem_get_playback_volume_range error\n");
                    return false;
                }

                volume = volume_min+step*(volume_max-volume_min)/(MAX_VOLUME-MIN_VOLUME);

                error = snd_mixer_selem_set_playback_volume(
                    mixerElement,
                    (snd_mixer_selem_channel_id_t) 0,
                    volume);

                if(error < 0)
                {
                    printf("ALSA_MixerSetVolume - snd_mixer_selem_set_playback_volume error\n");
                    return false;
                }
            }
            else
            {
                error = snd_mixer_selem_get_capture_volume_range(mixerElement,
                    &volume_min,
                    &volume_max);

                if(error < 0)
                {
                    printf("ALSA_MixerSetVolume - snd_mixer_selem_get_capture_volume_range error\n");
                    return false;
                }

                volume = volume_min+step*(volume_max-volume_min)/(MAX_VOLUME-MIN_VOLUME);

                error = snd_mixer_selem_set_capture_volume(
                    mixerElement,
                    (snd_mixer_selem_channel_id_t) 0,
                    volume);

                if(error < 0)
                {
                    printf("ALSA_MixerSetVolume - snd_mixer_selem_set_capture_volume error\n");
                    return false;
                }
            }

            return true;
        }
        return false;

    }

    bool OSAudioALSA::PCM_BeforePlayout()
    {
        assert(playout_handle!=NULL);

        int error;

        /*
        * prepare buffer
        * we just need one frame!!
        */
        playout_freeList.clear();
        playout_busyList.clear();

        for (WaveBuffList::iterator it = playout_holdList.begin();it != playout_holdList.end();it++)
        {
            BUFFHDR * phdr = *it;

            /*fill silence sample*/
            memset(phdr,0,sizeof(BUFFHDR)+playout_frameInByte);
            phdr->frameTotal  = playout_frameInTick*playout_channel;
            phdr->frameLeft   = playout_frameInTick;
            playout_busyList.push_back(phdr);
        }

        /*start device*/
        error = snd_pcm_prepare(playout_handle);
        if (error < 0)
        {
            printf("StartPlayout - snd_pcm_prepare failed (%s)\n",
                snd_strerror(error));
            return false;
        }

        return true;
    }
    bool OSAudioALSA::PCM_BeforeCapture()
    {
        assert(capture_handle!=NULL);

        int error;

        /*
        * prepare buffer
        * we just need one frame!!
        */
        capture_freeList.clear();
        capture_busyList.clear();

        for (WaveBuffList::iterator it = capture_holdList.begin();it != capture_holdList.end();it++)
        {
            BUFFHDR * phdr = *it;

            /*fill silence sample*/
            memset(phdr,0,sizeof(BUFFHDR)+capture_frameInByte);
            phdr->frameTotal  = capture_frameInTick*capture_channel;
            phdr->frameLeft   = capture_frameInTick;
            capture_busyList.push_back(phdr);
        }

        /*start device*/
        error = snd_pcm_prepare(capture_handle);
        if (error < 0)
        {
            printf("StartCapture - snd_pcm_prepare failed (%s)\n",
                snd_strerror(error));
            return false;
        }

        error = snd_pcm_start(capture_handle);
        if (error < 0)
        {
            printf("StartCapture - snd_pcm_start failed (%s)\n",
                snd_strerror(error));

            return false;
        }

        return true;
    }
    bool OSAudioALSA::PCM_BeforeFraming()
    {
        assert(playout_handle!=NULL);
        assert(capture_handle!=NULL);

        PCM_BeforeCapture();
        PCM_BeforePlayout();

        return true;
    }

    bool OSAudioALSA::PCM_AfterPlayout(void)
    {
        if(playout_handle)
            snd_pcm_drop(playout_handle);

        return true;
    }

    bool OSAudioALSA::PCM_AfterCapture(void)
    {
        if(capture_handle)
            snd_pcm_drop(capture_handle);

        return true;
    }

    bool OSAudioALSA::PCM_AfterFraming(void)
    {
        PCM_AfterPlayout();
        PCM_AfterCapture();

        return true;
    }

    bool OSAudioALSA::PCM_Playout(bool wait)
    {
        snd_pcm_sframes_t needs_frames;

        assert(playout_handle);

        /*
        * check available
        *
        * return a positive number of frames ready otherwise a negative error code
        */
        needs_frames = snd_pcm_avail_update(playout_handle);
        if (needs_frames < 0)
        {
            printf("PCM_Playout - snd_pcm_avail_update error: %s\n",
                snd_strerror(needs_frames));

            ALSA_ErrorRecovery(playout_handle,needs_frames);
        }
        else if(needs_frames > 0)
        {
            /*
            * handle busy buffer
            */
            while (needs_frames>0 && playout_busyList.size())
            {
                BUFFHDR*phdr        = *playout_busyList.begin();
                int  this_frames = std::min((size_t)needs_frames,phdr->frameLeft);
                int  left_bytes  = snd_pcm_frames_to_bytes(playout_handle,phdr->frameLeft);

                this_frames = snd_pcm_writei(playout_handle,
                    &phdr->data[playout_frameInByte - left_bytes],this_frames);

                if(this_frames <= 0)
                {
                    printf("PCM_Playout - snd_pcm_writei error!\n");

                    if(ALSA_ErrorRecovery(playout_handle,this_frames,true)!=0)
                        return false;
                    break;
                }
                /*this frame is over??then move to free list*/
                phdr->frameLeft -= this_frames;
                if(phdr->frameLeft==0)
                {
                    playout_busyList.pop_front();
                    playout_freeList.push_back(phdr);
                }

                /*the available is over??then break out*/
                needs_frames -= this_frames;
            }

            if(wait)
            {
                /*
                * handle free buffer
                */
                while(playout_freeList.size()>0)
                {
                    BUFFHDR * phdr = *playout_freeList.begin();

                    /*we want a full frame!!*/
                    if(manager->PlayoutPCM(phdr->data,phdr->frameTotal)<0)
                        return false;

                    phdr->frameLeft = playout_frameInTick;

                    /*move to busy list*/
                    playout_freeList.pop_front();
                    playout_busyList.push_back(phdr);
                }
            }
        }
        else
        {
            if(wait)
            {
                /*maximum tixe in milliseconds to wait, a negative value means infinity*/
                snd_pcm_wait(playout_handle, schduleDelay);
            }
        }
        return true;
    }

    bool OSAudioALSA::PCM_Capture(bool wait)
    {
        snd_pcm_sframes_t avail_frames;

        assert(capture_handle);

        /*
        * check available
        */
        /*return a positive number of frames ready otherwise a negative error code*/
        avail_frames = snd_pcm_avail_update(capture_handle);
        if (avail_frames < 0)
        {
            printf("PCM_Capture - snd_pcm_avail_update error: %s\n",
                snd_strerror(avail_frames));

            if(ALSA_ErrorRecovery(capture_handle,avail_frames,true)!=0)
                return false;
        }
        else if (avail_frames > 0)
        {
            /*
            * handle busy buffer
            */
            while (avail_frames>0 && capture_busyList.size()>0)
            {
                BUFFHDR*phdr        = *capture_busyList.begin();
                int  this_frames = std::min((size_t)avail_frames,phdr->frameLeft);
                int  left_bytes  = snd_pcm_frames_to_bytes(capture_handle,phdr->frameLeft);

                this_frames = snd_pcm_readi(capture_handle,&phdr->data[capture_frameInByte-left_bytes],this_frames);
                if (this_frames <= 0)
                {
                    printf("PCM_Capture - snd_pcm_readi error: %s\n",
                        snd_strerror(this_frames));

                    if(ALSA_ErrorRecovery(capture_handle,this_frames,true)!=0)
                        return false;
                    break;
                }

                /*this frame is full??then move to busy list*/
                phdr->frameLeft -= this_frames;
                if(phdr->frameLeft==0)
                {
                    capture_busyList.pop_front();
                    capture_freeList.push_back(phdr);
                }

                /*the available is over??then break out*/
                avail_frames -= this_frames;
            }

            if(wait)
            {
                /*
                * handle free buffer
                */
                while(capture_freeList.size()>0)
                {
                    BUFFHDR * phdr = *capture_freeList.begin();

                    /*we want a full frame!!*/
                    if(manager->CapturePCM(phdr->data,phdr->frameTotal)<0)
                        return false;

                    phdr->frameLeft = capture_frameInTick;

                    /*move to busy list*/
                    capture_freeList.pop_front();
                    capture_busyList.push_back(phdr);
                }
            }
        }
        else
        {
            if(wait)
            {
                /*maximum time in milliseconds to wait, a negative value means infinity*/
                snd_pcm_wait(capture_handle,schduleDelay);
            }
        }
        return true;
    }

    bool OSAudioALSA::PCM_Framing(bool wait)
    {
        snd_pcm_sframes_t avail_frames;
        snd_pcm_sframes_t needs_frames;

        assert(playout_handle!=NULL);
        assert(capture_handle!=NULL);

        /*
        * -----------------------------------
        * check capture buffer
        * -----------------------------------
        */
        PCM_Capture(false);

        /*
        * -----------------------------------
        * check playout buffer
        * -----------------------------------
        */
        PCM_Playout(false);

        /*
        * -----------------------------------
        * if got a whole frame buffer
        * -----------------------------------
        */
        while(playout_freeList.size()>0 && capture_freeList.size()>0)
        {
            BUFFHDR * phdr_p = *playout_freeList.begin();
            BUFFHDR * phdr_c = *capture_freeList.begin();

            //assert(playout_busyList.size()>0);
            //assert(capture_busyList.size()>0);

            /*
            * get more one frame
            */
            phdr_p->frameLeft = playout_frameInTick;
            phdr_c->frameLeft = playout_frameInTick;

            /*
            * get one frame
            */
            assert(playout_frameInByte==capture_frameInByte);
            assert(playout_frameInTick==capture_frameInTick);
            assert(phdr_p->frameTotal==phdr_c->frameTotal);

            if(manager->FramesPCM(phdr_p->data,phdr_c->data,phdr_p->frameTotal)<0)
                return false;

            /*
            * put back playout-buffer
            */
            playout_freeList.pop_front();
            playout_busyList.push_back(phdr_p);
            /*
            * put back capture-buffer
            */
            capture_freeList.pop_front();
            capture_busyList.push_back(phdr_c);
        }

        /*
        * -----------------------------------
        * else wait both data
        * -----------------------------------
        */
        avail_frames  = snd_pcm_avail_update(capture_handle);
        needs_frames  = snd_pcm_avail_update(playout_handle);
        struct pollfd  fds[2];
        size_t         fds_nr;
        int            fds_out;
        int            fds_in;

        fds_nr = 0;
        fds_out= -1;
        fds_in = -1;

        if(avail_frames==0)
        {
            snd_pcm_poll_descriptors(capture_handle,fds+fds_nr,1);
            fds_nr++;
            fds_in=0;
        }
        if(needs_frames==0)
        {
            snd_pcm_poll_descriptors(playout_handle,fds+fds_nr,1);
            fds_nr++;
            if(avail_frames==0)
                fds_out = 1;
            else
                fds_out = 0;
        }

        /*poll*/
        if(fds_nr>0)
        {
            while(1)
            {
                unsigned short event;

                poll(fds, fds_nr, -1);

                /*check event*/
                if(fds_in!=-1)
                {
                    snd_pcm_poll_descriptors_revents(capture_handle, fds+fds_in , 1, &event);
                    if (event & POLLERR)
                        return false;
                    if (event & POLLIN)
                        return true;
                }

                if(fds_out!=-1)
                {
                    snd_pcm_poll_descriptors_revents(playout_handle, fds+fds_out, 1, &event);
                    if (event & POLLERR)
                        return false;
                    if (event & POLLOUT)
                        return true;
                }
            }
        }
        return true;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*HAVE_ALSA*/
/************************************************************************/
/*                                                                      */
/************************************************************************/

