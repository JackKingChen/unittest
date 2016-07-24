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
#ifndef __DEVICE_AUDIO_H__
#define __DEVICE_AUDIO_H__

#include "host_device.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace host
{

    /*
    * class of OSAudioDevice thread helper
    */
    class OSAudioThread : public OSThread
    {
    public:
        enum
        {
            TASK_PLAYOUT = 0,
            TASK_CAPTURE = 1,
            TASK_FRAMING = 2,
        };

    public:
        OSAudioThread(OSAudioDevice * dev,int type)
        {
            device       = dev;
            deviceType   = type;
            time_inMS    = 0;
        };
        virtual bool Load(unsigned long ms=0)
        {
            /*
            * stack size must be greater than 128K!!
            */
            time_inMS = ms;
            return OSThread::Load(128*1024);
        };
        virtual void OnLoad()
        {
            switch(deviceType)
            {
            case TASK_PLAYOUT:
                device->PCM_BeforePlayout();break;
            case TASK_CAPTURE:
                device->PCM_BeforeCapture();break;
            case TASK_FRAMING:
                device->PCM_BeforeFraming();break;
            }
            time_begin.GetTime();

            /*set priority*/
            SetPriority(PriorityMax);

            /*set affinity*/
            SetAffinity(0x01);
        };
        virtual void OnTerm()
        {
            switch(deviceType)
            {
            case TASK_PLAYOUT:
                device->PCM_AfterPlayout();break;
            case TASK_CAPTURE:
                device->PCM_AfterCapture();break;
            case TASK_FRAMING:
                device->PCM_AfterFraming();break;
            }
        }
        virtual bool Main()
        {
            if(time_inMS!=0)
            {
                OSTime now;

                now -= time_begin;

                if(now.GetMSecond()>=(long)time_inMS)
                    return false;
            }

            switch(deviceType)
            {
            case TASK_PLAYOUT:
                return device->PCM_Playout();
            case TASK_CAPTURE:
                return device->PCM_Capture();
            case TASK_FRAMING:
                return device->PCM_Framing();
            }
            return false;
        };

    private:
        OSAudioDevice *device;
        int            deviceType;
        OSTime         time_begin;
        unsigned long  time_inMS;
    };

}; /*end of host*/
/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*__DEVICE_AUDIO_H__*/
