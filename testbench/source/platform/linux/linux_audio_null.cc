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
#ifndef HAVE_ALSA
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
    class OSAudioNULL;
    class OSAudioThread;


    /*
    * class of OSAudioNULL
    */
    class OSAudioNULL:public OSAudioDevice
    {

    public:

        OSAudioNULL(OSAudioManager& mng){};
        ~OSAudioNULL(){};

    public:
        /*init*/
        virtual bool Init(){return false;};

        /* Device enumeration*/
        virtual int GetPlayoutDevices(){return -1;};
        virtual int GetCaptureDevices(){return -1;};

        virtual int GetPlayoutDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN]){return -1;};
        virtual int GetCaptureDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN]){return -1;};

        /*Device status*/
        virtual bool IsPlayoutOpen(){return false;};
        virtual bool IsCaptureOpen(){return false;};
        virtual bool IsPlayoutRunning(){return false;};
        virtual bool IsCaptureRunning(){return false;};

        /*Device open&close*/
        virtual bool OpenPlayoutDevice(){return false;};
        virtual bool OpenCaptureDevice(){return false;};

        virtual bool ClosePlayoutDevice(){return false;};
        virtual bool CloseCaptureDevice(){return false;};

        /*Device start&stop*/
        virtual bool StartPlayout(unsigned long ms=0){return false;};
        virtual bool StartCapture(unsigned long ms=0){return false;};
        virtual bool StopPlayout(){return false;};
        virtual bool StopCapture(){return false;};
        virtual bool WaitPlayout(){return false;};
        virtual bool WaitCapture(){return false;};
        virtual bool StartFraming(unsigned long ms=0){return false;};
        virtual bool StopFraming(){return false;};
        virtual bool WaitFraming(){return false;};

        /*Volume control*/
        virtual bool GetPlayoutVolume(int*again, int*dgain = NULL){return false;};
        virtual bool SetPlayoutVolume(int again, int dgain = -1)  {return false;};
        virtual bool GetCaptureVolume(int*again, int*dgain = NULL){return false;};
        virtual bool SetCaptureVolume(int again, int dgain = -1)  {return false;};
        virtual int  GetPlayoutDelay(void){return 0;};
        virtual int  GetCaptureDelay(void){return 0;};

        /*thread callback*/
        virtual bool PCM_BeforePlayout(void){return false;};
        virtual bool PCM_BeforeCapture(void){return false;};
        virtual bool PCM_BeforeFraming(void){return false;};
        virtual bool PCM_AfterPlayout(void){return false;};
        virtual bool PCM_AfterCapture(void){return false;};
        virtual bool PCM_AfterFraming(void){return false;};
        virtual bool PCM_Playout(bool wait=true){return false;};
        virtual bool PCM_Capture(bool wait=true){return false;};
        virtual bool PCM_Framing(bool wait=true){return false;};

        /*data*/
    protected:


    };

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    bool OSAudioDevice::CreatePlatformDevice(OSAudioDevice *&pdev,OSAudioManager& mng)
    {
        pdev = new OSAudioNULL(mng);
        return pdev!=NULL;
    }

    bool OSAudioDevice::DestroyPlatformDevice(OSAudioDevice *&pdev)
    {
        OSAudioNULL * palsa = dynamic_cast<OSAudioNULL*>(pdev);
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

