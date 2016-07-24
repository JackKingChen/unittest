/********************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2013-08-21
*
*******************************************************************/

#include "host.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace host
{
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/

    OSAudioFrame::OSAudioFrame()
    {
        frameFS   = 0;
        frameBW   = 0;
        frameChan = 0;

        frameMaxBytes  = 0;
        frameMaxSample = 0;
        frameAvailable = 0;
        memset(frameSample,0,sizeof(frameSample));
    }
    OSAudioFrame::~OSAudioFrame()
    {
        Destroy();
    }

    bool OSAudioFrame::Create(int fs,int bw,int chan)
    {
        assert(fs>0);
        assert(bw==16||bw==24||bw==32);
        assert(chan>=0 && chan<MAX_CHAN);

        Destroy();

        frameFS   = fs;
        frameBW   = bw;
        frameChan = chan;

        /*
        * (16000/100)*(16/8) = 160*2=320 + pad
        */
        frameMaxSample= frameFS;
        frameMaxBytes = (frameMaxSample)*(frameBW/8);

        /*
        * alloc buffer
        */
        for (int i=0;i<(int)frameChan;i++)
        {
            frameSample[i] = new unsigned char[frameMaxBytes+MAX_PAD];
            memset(frameSample[i],0,frameMaxBytes);
        }
        return true;
    }

    bool OSAudioFrame::Destroy()
    {
        for (int i=0;i<MAX_CHAN;i++)
        {
            if(frameSample[i])
                delete[] frameSample[i];
            frameSample[i] = NULL;
        }

        return true;
    }

    bool OSAudioFrame::DoInterleave(void *sample,int sample_nr,int chan_nr) const
    {
        int  max_chan  = std::min(chan_nr,  (int)frameChan);
        int  max_sample= std::min(sample_nr,(int)frameMaxSample);
        int  max_step  = frameBW/8;
        int  src_pos   = 0;
        int  dst_pos   = 0;

        assert(sample!=NULL);
        assert(sample_nr>0);
        assert(chan_nr>0 && chan_nr<MAX_CHAN);

        for (int k=0;k<max_sample;k++)
        {
            for (int i=0;i<max_chan;i++)
            {
                assert(frameSample[i]!=NULL);

                memcpy(((unsigned char*)sample)+dst_pos,((unsigned char*)frameSample[i])+src_pos,max_step);
                dst_pos+=max_step;
            }
            src_pos+=max_step;
        }

        return true;
    }

    bool OSAudioFrame::DeInterleave(void *sample,int sample_nr,int chan_nr)
    {
        int  max_sample= std::min(sample_nr,(int)frameFS);
        int  max_step  = frameBW/8;
        int  src_pos   = 0;
        int  dst_pos   = 0;

        assert(sample!=NULL);
        assert(sample_nr>0);
        assert(chan_nr>0 && chan_nr<=(int)frameChan);

        for (int k=0;k<max_sample;k++)
        {
            for (int i=0;i<(int)frameChan;i++)
            {
                assert(frameSample[i]!=NULL);

                if(i<chan_nr)
                    /*have proper data*/
                    memcpy(((unsigned char*)frameSample[i])+dst_pos,((unsigned char*)sample)+src_pos,max_step);
                else
                    /*set mute data*/
                    memset(((unsigned char*)frameSample[i])+dst_pos,0x00,max_step);
                src_pos+=max_step;
            }
            dst_pos+=max_step;
        }

        frameAvailable = max_sample;

        return true;
    }

    bool OSAudioFrame::DoBlock(void *sample,int sample_nr,int chan_nr) const
    {
        size_t chanNr   = std::min(chan_nr, (int)frameChan);
        size_t framefs  = (sample_nr/chan_nr)*(frameBW/8);

        for (size_t i=0;i<chanNr;i++)
        {
            memcpy(((unsigned char*)sample)+framefs*i,
                (unsigned char*)frameSample[i],framefs);
        }

        return true;
    }

    bool OSAudioFrame::DeBlock(void *sample,int sample_nr,int chan_nr) 
    {
        chan_nr  = std::min(chan_nr, (int)frameChan);

        frameAvailable = sample_nr/chan_nr;

        for (int i=0;i<chan_nr;i++)
        {
            memcpy((unsigned char*)frameSample[i],
                ((unsigned char*)sample)+frameAvailable*i,frameAvailable*(frameBW/8));
        }

        return true;
    }
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    OSAudioManager::OSAudioManager(int Freq,int Chan,int bitW,int Hz)
    {
        device     = NULL;
        deviceHz   = Hz;
        deviceFreq = Freq;
        deviceBW   = bitW;

        deviceIndex  =0;
        deviceChannel=Chan;    /*1 or 2*/
        deviceLatency=1000/Hz; /*latency as one frame(probably 10ms)*/

        if(deviceHz>OSAudioFrame::MAX_HZ)
            deviceHz = OSAudioFrame::MAX_HZ;

        if(deviceBW!=16 && bitW!=24 && bitW!=32)
            deviceBW = 16;
        if(deviceChannel!=1 && deviceChannel!=2)
            deviceChannel = 2;
        
        memset(deviceIP,0,sizeof(deviceIP));
    }

    OSAudioManager::~OSAudioManager()
    {
        ExitDevice();
    }

    bool OSAudioManager::InitDevice(const char*IP,bool bMaster)
    {
        bool success;

        /*
        * destroy old one
        */
        ExitDevice();

        /*
        * alloc platform device
        */
        if(IP)
        {
            SetDevIP(IP);
            success = OSAudioDevice::CreateRemoteIPDevice(device,*this);
        }
        else
        {
            success = OSAudioDevice::CreatePlatformDevice(device,*this);
        }

        assert(success==true);
        assert(device!=NULL);

        /*
        * alloc frame buffer
        */
        capture_frame.Create(deviceFreq/deviceHz,deviceBW,deviceChannel);
        playout_frame.Create(deviceFreq/deviceHz,deviceBW,deviceChannel);

        return true;
    }

    bool OSAudioManager::ExitDevice(void)
    {
        if(device)
        {
            OSAudioDevice::DestroyPlatformDevice(device);
            OSAudioDevice::DestroyRemoteIPDevice(device);
        }

        /*
        * alloc frame buffer
        */
        capture_frame.Destroy();
        playout_frame.Destroy();

        return true;
    }

    bool OSAudioManager::Start(int ms)
    {
        assert(device!=NULL);

        device->StartFraming(ms);

        return true;
    }
    bool OSAudioManager::Record(int ms)
    {
        assert(device!=NULL);

        if(device->StartCapture(ms))
        {
            SetCaptureVolume(100);
            return true;
        }
        return false;
    }
    bool OSAudioManager::Play(int ms)
    {
        assert(device!=NULL);

        if(device->StartPlayout(ms))
        {
            SetPlayoutVolume(100);
            return true;
        }
        return false;
    }
    bool OSAudioManager::Stop()
    {
        if(device)
        {
            device->StopFraming();
            device->StopPlayout();
            device->StopCapture();
        }
        return true;
    }
    bool OSAudioManager::Wait()
    {
        if(device)
        {
            device->WaitFraming();
            device->WaitPlayout();
            device->WaitCapture();
        }
        return true;
    }

    int OSAudioManager::PlayoutPCM(void* sample,int sample_nr)
    {
        int samples = sample_nr/deviceChannel;

        /*call event*/
        playout_frame.frameAvailable = samples;
        if(OnPlayout(playout_frame)<0)
            return -1;

        /*save data to frame buffer*/
        playout_frame.DoInterleave(sample,samples,deviceChannel);

        return sample_nr;
    }

    int OSAudioManager::CapturePCM(void* sample,int sample_nr)
    {
        /*mark time*/
        OSTime::GetTime();
        capture_frame.frameSec  = tv_sec;
        capture_frame.frameUSec = tv_usec;

        /*save data to frame buffer*/
        capture_frame.DeInterleave(sample,sample_nr,deviceChannel);

        /*call event*/
        if(OnCapture(capture_frame)<0)
            return -1;

        return sample_nr;
    }

    int  OSAudioManager::FramesPCM(void* playout,void* capture,int sample_nr,bool bInterleaved)
    {
        int samples = sample_nr/deviceChannel;

        /*mark time*/
        OSTime::GetTime();
        capture_frame.frameSec  = tv_sec;
        capture_frame.frameUSec = tv_usec;

        /*
        * save data to capture
        */
        if(bInterleaved)
        {
            capture_frame.DeInterleave(capture,sample_nr,deviceChannel);
        }
        else
        {
            capture_frame.DeBlock(capture,sample_nr,deviceChannel);
        }

        /*
        * call in event
        */
        playout_frame.frameAvailable = samples;
        if(OnFrames(capture_frame,playout_frame)<0)
            return -1;

        /*
        * copy data to playout
        */
        if(bInterleaved)
        {
            playout_frame.DoInterleave(playout,sample_nr,deviceChannel);
        }
        else
        {
            playout_frame.DoBlock(playout,sample_nr,deviceChannel);
        }

        return 0;
    }

    OSAudioDevice& OSAudioManager::GetDevice()
    {
        assert(device!=NULL);
        return *device;
    }

    OSAudioDevice& OSAudioManager::SetDevice(OSAudioDevice &dev)
    {
        device= &dev;
        return *device;
    }

    int OSAudioManager::SetHz(int Hz)
    {
        int old = deviceHz;
        deviceHz= Hz;
        if(device)
            device->Init();
        return old;
    }

    int OSAudioManager::GetHz()
    {
        return deviceHz;
    }

    int OSAudioManager::SetFreq(int Freq)
    {
        int old   = deviceFreq;
        deviceFreq= Freq;
        if(device)
            device->Init();
        return old;
    }

    int OSAudioManager::GetFreq()
    {
        return deviceFreq;
    }

    int OSAudioManager::SetBitW(int bitW)
    {
        int old = deviceBW;
        deviceBW= bitW;
        if(device)
            device->Init();
        return old;
    }

    int OSAudioManager::GetBitW()
    {
        return deviceBW;
    }

    int OSAudioManager::SetDev(int devIndex)
    {
        int old    = deviceIndex;
        deviceIndex= devIndex;
        if(device)
            device->Init();
        return old;
    }

    int OSAudioManager::GetDev()
    {
        return deviceIndex;
    }

    int OSAudioManager::SetChan(int devIndex)
    {
        int old      = deviceChannel;
        deviceChannel= devIndex;
        if(device)
            device->Init();
        return old;
    }

    int OSAudioManager::GetChan()
    {
        return deviceChannel;
    }

    int  OSAudioManager::SetLatency(int devLatency)
    {
        int old      = deviceLatency;
        deviceLatency= devLatency;
        if(device)
            device->Init();
        return old;
    }

    int  OSAudioManager::GetLatency()
    {
        return deviceLatency;
    }
    int  OSAudioManager::SetDevIP(const char* IP)
    {
        assert(IP!=NULL);
        strcpy(deviceIP,IP);
        return 0;
    }
    int  OSAudioManager::GetDevIP(char* IP)
    {
        assert(IP!=NULL);
        strcpy(IP,deviceIP);
        return 0;
    }

    bool OSAudioManager::SetDevMaster(bool bMaster)
    {
        deviceMaster = bMaster;
        return deviceMaster;
    }
    bool OSAudioManager::GetDevMaster()
    {
        return deviceMaster;
    }

    int  OSAudioManager::GetPlayoutVolume()
    {
        int volume;
        assert(device!=NULL);
        if(device->GetPlayoutVolume(&volume))
            return volume;
        else
            return -1;
    }

    int  OSAudioManager::SetPlayoutVolume(int again, int dgain)
    {
        assert(device != NULL);
        device->SetPlayoutVolume(again, dgain);
        return 0;
    }
    int  OSAudioManager::GetCaptureVolume()
    {
        int volume;
        assert(device!=NULL);
        if(device->GetCaptureVolume(&volume))
            return volume;
        else
            return -1;
    }

    int  OSAudioManager::SetCaptureVolume(int again, int dgain)
    {
        assert(device != NULL);
        device->SetCaptureVolume(again, dgain);
        return 0;
    }
    int  OSAudioManager::GetPlayoutDelay()
    {
        assert(device!=NULL);
        return device->GetPlayoutDelay();
    }
    int  OSAudioManager::GetCaptureDelay()
    {
        assert(device!=NULL);
        return device->GetCaptureDelay();
    }
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    OSAudioRemote::OSAudioRemote(const char* strIP,int freq,int chan,int bw)
        :OSAudioManager(freq,chan,bw)
    {
        data_fs     = freq/100;
        data_bytes  = data_fs*(bw/8);
        data_missed = 0;
        data_pkt    = (RTAudioFrame*)malloc(1500);
        data_total  = data_bytes*chan;
        data_tx_seq = 0;
        data_tx_ssrc= OS::Rand();
        data_tx_ts  = 0;

        /*init local audio*/
        OSAudioManager::InitDevice();

        /*init local socket*/
        if(!ctrl_sock.Create(OSSock::ST_UDP,strIP,IPDEV_RE_CTRL_PORT))
            printf("OSAudioRemote:failed to create control socket!\n");
        ctrl_sock.Connect(strIP,IPDEV_LO_CTRL_PORT);
        ctrl_sock.SetBlock(false);

        if(!data_sock.Create(OSSock::ST_UDP,strIP,IPDEV_RE_DATA_PORT))
            printf("OSAudioRemote:failed to create data socket!\n");
        data_sock.Connect(strIP,IPDEV_LO_DATA_PORT);
        data_sock.SetBlock(false);
    };
    OSAudioRemote::~OSAudioRemote()
    {
        free(data_pkt);
        ctrl_sock.Close();
        data_sock.Close();
    }

    int OSAudioRemote::OnFrames(OSAudioFrame &capture_frames,OSAudioFrame &playout_frames)
    {
        int retval;

        /*
        * handle data socket
        */
        /*get one frame  from remote*/
        retval = data_sock.Recv(data_pkt,1500);
        if(retval>0)
        {
            OnFramesRX(*data_pkt);

            for (size_t i=0;i<deviceChannel;i++)
            {
                memcpy(playout_frames[i],((char*)(data_pkt+1))+i*data_bytes,data_bytes);
            }

            /*clear missed*/
            data_missed = 0;
        }
        else
        {
            if(data_missed++==0)
            {
                printf("OSAudioRemote£ºrecv:missing one frame\n");
            }

            memset(playout_frames[0],0,data_bytes);
        }

        /*send one frame to remote*/
        data_pkt->version   = 0x80;
        data_pkt->ptype     = (2<<RTAF_PT_CHAN_SHITF);
        data_pkt->seqNumber = OSSock::Htons(data_tx_seq);
        data_pkt->timeStamp = OSSock::Htonl(data_tx_ts);
        data_pkt->ssrc      = OSSock::Htonl(data_tx_ssrc);

        for (size_t i=0;i<deviceChannel;i++)
        {
            memcpy(((char*)(data_pkt+1))+i*data_bytes,capture_frames[i],data_bytes);
        }

        retval = data_sock.SendTo(data_pkt,sizeof(RTAudioFrame)+data_total);
        if(retval != (int)(sizeof(RTAudioFrame)+data_total))
        {
            printf("ReAudio:failed to send data,size=%d,err=%d\n",
                sizeof(RTAudioFrame)+data_total,retval);
        }
        
        OnFramesRX(*data_pkt);

        /*
        * next frame
        */
        data_tx_seq++;
        data_tx_ts+=data_fs;

        /*
        * handle control socket
        */

        return 0;
    };

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
}; /*end of host*/


/************************************************************************/
/*                                                                      */
/************************************************************************/

