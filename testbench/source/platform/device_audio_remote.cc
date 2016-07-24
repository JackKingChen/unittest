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

#include "host.h"
#include "device_audio.h"

namespace host
{
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    class OSAudioIP;
    class OSAudioThread;

    class FrameFIFO
    {
    public:
        FrameFIFO (const unsigned int size);
        ~FrameFIFO();

        virtual bool Write(char *data_in, const unsigned int size);
        virtual bool Read (char *data_out,const unsigned int size);
        virtual bool Clear();

        virtual size_t ReadAvailable();
        virtual size_t WriteAvailable();

    private:
        char         *data_head;
        char         *data_tail;
        char         *fifo_head;
        char         *fifo_tail;

        unsigned int  fifo_size;
        unsigned int  data_empty;
        unsigned int  data_valid;
    };

    /*
    * class of OSAudioIP
    */
    class OSAudioIP:public OSAudioDevice
    {
    public:
        enum
        {
            /**/
            MAX_LATENCY = 40*1000,

            /*on frame for buffer ,and 50ms delay max*/
            MAX_BUFHDR  = 5,
            /*96000,10Hz,100ms,32bit sample*/
            MAX_BUFFER  = (96000/10)*sizeof(int),
            /*MAX_BUFFER=10ms x 4 = 40ms*/
            MAX_FIFOSIZE= MAX_BUFFER*4,

            /*delay setting,from 10ms->500ms,5ms for each step*/
            ONE_LATANCY = 5,
            MIN_LATANCY = 5,
            MAX_LATANCY = 500,
        };

    public:

        OSAudioIP(OSAudioManager& mng);
        ~OSAudioIP();

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

        /*data*/
    protected:

        OSAudioManager  *manager;
        OSAudioThread   *schduleThread;
        bool             schduleStarted;

        /*remote IP*/
        char            schduleIP[256];

        /*audio buffer for slave mode*/

        OSSock          slave_ctrl_sock;
        OSSock          slave_data_sock;
        RTAudioFrame   *slave_data_tx;
        RTAudioFrame   *slave_data_rx;
        unsigned int    slave_data_delay;
        unsigned int    slave_data_latency;
        unsigned int    slave_data_latencyPKT;
        unsigned int    slave_frame_tx;
        unsigned int    slave_frame_rx;
        unsigned int    slave_frame_ssrc;
        unsigned int    slave_frame_bytes;
        unsigned int    slave_frame_sample;
        FrameFIFO      *slave_frame_fifo_in;
        FrameFIFO      *slave_frame_fifo_out;
    };

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    bool OSAudioDevice::CreateRemoteIPDevice(OSAudioDevice *&pdev,OSAudioManager& mng)
    {
        pdev = new OSAudioIP(mng);
        return pdev!=NULL;
    }

    bool OSAudioDevice::DestroyRemoteIPDevice(OSAudioDevice *&pdev)
    {
        OSAudioIP * pIPdev = dynamic_cast<OSAudioIP*>(pdev);
        if(pIPdev)
        {
            delete pIPdev;
            pIPdev = NULL;
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
    /************************************************************************/
    /* FrameFIFO function                                                   */
    /************************************************************************/
    FrameFIFO::FrameFIFO(const unsigned int size)
    {
        fifo_head =(char *)malloc(size);
        assert(fifo_head!=NULL);

        fifo_tail = fifo_head + size;
        fifo_size = size;
        data_head = fifo_head;
        data_tail = fifo_head;
        data_empty = fifo_size;
        data_valid = 0;
    }

    FrameFIFO::~FrameFIFO()
    {
        free(fifo_head);
    }

    bool FrameFIFO::Read(char *data_out_p,const unsigned int size)
    {
        if (size > data_valid || size <= 0 || size > fifo_size)
        {/*make sure it will not underflow*/
            return false;
        }
        else
        {
            data_valid -= size;
            data_empty += size;
        }

        if((data_tail + size) > fifo_tail)/*make sure it will not over flow the fifo*/
        {
            unsigned int tail_length = fifo_tail - data_tail;
            memcpy(data_out_p, data_tail, tail_length);
            memcpy(data_out_p + tail_length, fifo_head, size - tail_length);
            data_tail = fifo_head + size - tail_length;
        }
        else
        {
            memcpy(data_out_p, data_tail, size);
            data_tail += size;
        }

        return true;
    }

    bool FrameFIFO::Write(char *data_int_p, unsigned int size)
    {
        if (size > data_empty || size <=0 || size > fifo_size)
        {/*make sure it will not overflow*/
            return false;
        }
        else
        {
            data_valid += size;
            data_empty -= size;
        }
        if (data_head + size > fifo_tail)
        {
            unsigned int tail_length = fifo_tail - data_head;
            memcpy(data_head, data_int_p, tail_length);
            memcpy(fifo_head, data_int_p + tail_length, (size - tail_length));
            data_head = fifo_head + (size - tail_length);
        }
        else
        {
            memcpy(data_head, data_int_p, size);
            data_head = data_head + size;
        }

        return true;
    }

    size_t FrameFIFO::ReadAvailable()
    {
        return data_valid;
    }
    size_t FrameFIFO::WriteAvailable()
    {
        return data_empty;
    }

    bool FrameFIFO::Clear()
    {
        data_head   = fifo_head;
        data_tail   = fifo_head;
        data_valid  = 0;
        data_empty  = fifo_size;

        memset(fifo_head,0,fifo_size);
        return true;
    }
    /************************************************************************/
    /* end FrameFile                                                        */
    /************************************************************************/
    /************************************************************************/
    /* OSAudioIP function                                                   */
    /************************************************************************/

    OSAudioIP::OSAudioIP(OSAudioManager& mng)
    {
        const char *localIP;

        manager = &mng;

        schduleThread  = new OSAudioThread(this,OSAudioThread::TASK_FRAMING);
        schduleStarted = false;

        /*create a frame fifo*/
        slave_frame_fifo_in  = new FrameFIFO(MAX_FIFOSIZE);
        slave_frame_fifo_out = new FrameFIFO(MAX_FIFOSIZE);
        slave_frame_sample   = 0;
        slave_frame_bytes    = 0;
        slave_frame_tx       = 0;
        slave_frame_rx       = 0;

        slave_data_tx        = (RTAudioFrame *)malloc(sizeof(RTAudioFrame)+MAX_BUFFER);
        slave_data_rx        = (RTAudioFrame *)malloc(sizeof(RTAudioFrame)+MAX_BUFFER);
        slave_data_delay     = 0;
        slave_data_latency   = 0;
        slave_data_latencyPKT= 0;

        /*
        * init sock
        */
        manager->GetDevIP(schduleIP);

        if(strcmp(schduleIP,"127.0.0.1")==0)
            localIP = schduleIP;
        else
            localIP = NULL;

        if(!slave_ctrl_sock.Create(OSSock::ST_UDP,localIP,OSAudioManager::IPDEV_LO_CTRL_PORT))
            printf("OSAudioIP:failed to create control socket!\n");
        slave_ctrl_sock.Connect(schduleIP,OSAudioManager::IPDEV_RE_CTRL_PORT);
        slave_ctrl_sock.SetBlock(false);

        if(!slave_data_sock.Create(OSSock::ST_UDP,localIP,OSAudioManager::IPDEV_LO_DATA_PORT))
            printf("OSAudioIP:failed to create data socket!\n");
        slave_data_sock.Connect(schduleIP,OSAudioManager::IPDEV_RE_DATA_PORT);
        slave_data_sock.SetBlock(true);

        /*
        * init others
        */
        Init();
    }

    OSAudioIP::~OSAudioIP()
    {
        StopPlayout();
        StopCapture();

        free(slave_data_tx);
        free(slave_data_rx);
    }

    bool OSAudioIP::Init()
    {
        RTAudioCtrl msg;
        assert(manager!=NULL);

        slave_frame_sample= manager->GetFreq()/100;
        slave_frame_bytes = slave_frame_sample*(manager->GetBitW()/8);
        slave_frame_ssrc  = 0;

        slave_data_delay     = 0;
        slave_data_latency   = manager->GetLatency();
        slave_data_latencyPKT= 0;

        /*fixup delay*/
        slave_data_latency= (slave_data_latency/ONE_LATANCY)*ONE_LATANCY;
        slave_data_latency= std::max(slave_data_latency,(unsigned int)MIN_LATANCY);
        slave_data_latency= std::min(slave_data_latency,(unsigned int)MAX_LATANCY);

        msg.ssrc   = slave_frame_ssrc;
        msg.msg    = RTAF_MSG_SAMFREQ;
        msg.wparam = manager->GetFreq();
        msg.lparam = 0;  /*init unused param*/

        if(slave_ctrl_sock.Send(&msg,sizeof(msg))==sizeof(msg))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    int  OSAudioIP::GetPlayoutDevices()
    {
        return 0;
    }

    int  OSAudioIP::GetCaptureDevices()
    {
        return 0;
    }

    int  OSAudioIP::GetPlayoutDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN])
    {


        return 0;
    }

    int  OSAudioIP::GetCaptureDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN])
    {


        return 0;
    }

    bool OSAudioIP::IsPlayoutOpen()
    {
        return true;
    }

    bool OSAudioIP::IsCaptureOpen()
    {
        return true;
    }

    bool OSAudioIP::IsPlayoutRunning()
    {
        return schduleStarted;
    }
    bool OSAudioIP::IsCaptureRunning()
    {
        return schduleStarted;
    }

    bool OSAudioIP::OpenPlayoutDevice()
    {

        return false;
    }

    bool OSAudioIP::OpenCaptureDevice()
    {

        return false;
    }

    bool OSAudioIP::ClosePlayoutDevice()
    {
        return true;
    }

    bool OSAudioIP::CloseCaptureDevice()
    {
        return true;
    }

    bool OSAudioIP::StartPlayout(unsigned long ms)
    {

        return true;
    }

    bool OSAudioIP::StartCapture(unsigned long ms)
    {
        return true;
    }

    bool OSAudioIP::StopPlayout()
    {
        assert(schduleThread!=NULL);

        schduleStarted = false;

        /*kill thread*/
        schduleThread->Term();

        /*close device*/
        ClosePlayoutDevice();

        return true;
    }

    bool OSAudioIP::StopCapture()
    {
        assert(schduleThread!=NULL);

        schduleStarted = false;

        /*kill thread*/
        schduleThread->Term();

        /*close device*/
        CloseCaptureDevice();

        return true;
    }

    bool OSAudioIP::WaitPlayout()
    {
        if(schduleThread!=NULL)
            return schduleThread->Wait();
        else
            return false;
    }
    bool OSAudioIP::WaitCapture()
    {
        if(schduleThread!=NULL)
            return schduleThread->Wait();
        else
            return false;
    }

    bool OSAudioIP::StartFraming(unsigned long ms)
    {
        /*make sure it's opened*/
        if(!IsCaptureOpen())
            if(!OpenCaptureDevice())
                return false;
        if(!IsPlayoutOpen())
            if(!OpenPlayoutDevice())
                return false;

        /*start thread*/
        schduleStarted = true;
        return schduleThread->Load(ms);
    }

    bool OSAudioIP::StopFraming()
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

    bool OSAudioIP::WaitFraming()
    {
        if(schduleThread!=NULL)
            return schduleThread->Wait();
        else
            return false;
    }

    /*Volume control*/
    bool OSAudioIP::GetPlayoutVolume(int*again, int*dgain)
    {
        if (again != NULL)
        {
            *again = 0;
        }

        if (dgain != NULL)
        {
            *dgain = 0;
        }
        return true;
    }

    bool OSAudioIP::SetPlayoutVolume(int again, int dgain)
    {
        RTAudioCtrl msg;

        again = std::min(again,(int)MAX_VOLUME);
        again = std::max(again,(int)MIN_VOLUME);
        dgain = std::min(dgain,(int)MAX_VOLUME);
        dgain = std::max(dgain,(int)MIN_VOLUME);


        msg.ssrc   = slave_frame_ssrc;
        msg.msg    = RTAF_MSG_SPKGAIN;
        msg.wparam = again;
        msg.lparam = dgain;

        if(slave_ctrl_sock.Send(&msg,sizeof(msg))==sizeof(msg))
            return true;
        else
            return false;
    }

    bool OSAudioIP::GetCaptureVolume(int*again, int *dgain)
    {
        if (again != NULL)
        {
            *again = 0;
        }
        
        if (dgain != NULL)
        {
            *dgain = 0;
        }
        
        return true;
    }

    bool OSAudioIP::SetCaptureVolume(int again, int dgain)
    {
        RTAudioCtrl msg;

        again = std::min(again,(int)MAX_VOLUME);
        again = std::max(again,(int)MIN_VOLUME);
        dgain = std::min(dgain,(int)MAX_VOLUME);
        dgain = std::max(dgain,(int)MIN_VOLUME);

        msg.ssrc   = slave_frame_ssrc;
        msg.msg    = RTAF_MSG_MICGAIN;
        msg.wparam = again;
        msg.lparam = dgain;

        if(slave_ctrl_sock.Send(&msg,sizeof(msg))==sizeof(msg))
            return true;
        else
            return false;
    }
    int  OSAudioIP::GetPlayoutDelay(void)
    {
        return slave_data_delay;
    }
    int  OSAudioIP::GetCaptureDelay(void)
    {
        return slave_data_delay;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    bool OSAudioIP::PCM_BeforePlayout()
    {

        return true;
    }
    bool OSAudioIP::PCM_BeforeCapture()
    {

        return true;
    }
    bool OSAudioIP::PCM_BeforeFraming()
    {

        PCM_BeforeCapture();
        PCM_BeforePlayout();

        return true;
    }

    bool OSAudioIP::PCM_AfterPlayout(void)
    {

        return true;
    }

    bool OSAudioIP::PCM_AfterCapture(void)
    {

        return true;
    }

    bool OSAudioIP::PCM_AfterFraming(void)
    {
        PCM_AfterPlayout();
        PCM_AfterCapture();

        return true;
    }

    bool OSAudioIP::PCM_Playout(bool wait)
    {


        return true;
    }

    bool OSAudioIP::PCM_Capture(bool wait)
    {


        return true;
    }

    bool OSAudioIP::PCM_Framing(bool wait)
    {
        RTAudioCtrl ctrl;
        int         retval;

        /*
        * handle data sock
        */
        retval = slave_data_sock.Recv(slave_data_rx,MAX_BUFFER);
        if(retval>0)
        {
            unsigned int  recv_chan = ((slave_data_rx->ptype)&RTAF_PT_CHAN_MASK)>>RTAF_PT_CHAN_SHITF;
            unsigned int  recv_inte = ((slave_data_rx->ptype)&RTAF_PT_DATA_INTERLEAVE);
            unsigned int  recv_size = retval-sizeof(RTAudioFrame);
            unsigned char buff_src[sizeof(RTAudioFrame)+MAX_BUFFER];
            unsigned char buff_dst[sizeof(RTAudioFrame)+MAX_BUFFER];

            (void)recv_chan;
            (void)recv_inte;
            (void)recv_size;

            /*save stats*/
            slave_frame_rx++;

            /*
            *-----------------------
            *save in one frame
            *-----------------------
            */
            if (!slave_frame_fifo_in->Write((char *)(slave_data_rx + 1), recv_size))
            {
                printf("push data to fifo failed\n");
            }

            /*
            *-----------------------
            * processing frame in 10ms
            *-----------------------
            */
            while(slave_frame_fifo_in->ReadAvailable() >= slave_frame_bytes)
            {
                if(!slave_frame_fifo_in->Read((char*)buff_src, slave_frame_bytes))
                {
                    printf("slave frame fifo_in pop data failed\n");
                }

                if(manager->FramesPCM(buff_dst, buff_src, slave_frame_sample,!!recv_inte) != 0)
                {
                    printf("process one frame failed\n");
                }

                if(!slave_frame_fifo_out->Write((char*)buff_dst, slave_frame_bytes))
                {
                    printf("slave frame fifo_out push data to fifo failed\n");
                }
            }

            /*
            *-----------------------
            *compute delay packets
            *-----------------------
            */
            if(slave_data_latencyPKT==0)
            {
                assert(recv_size>0);
                assert(recv_chan>0);

                /*compute delay*/
                slave_data_latencyPKT = (recv_size/recv_chan)/(manager->GetBitW()/8);
                slave_data_latencyPKT*=10;
                slave_data_latencyPKT/= manager->GetFreq()/manager->GetHz();
                slave_data_latencyPKT = slave_data_latency/slave_data_latencyPKT;

                /*save SSRC*/
                slave_frame_ssrc      = slave_data_rx->ssrc;
            }

            /*
            *-----------------------
            *send out one frame
            *-----------------------
            */
            if (slave_frame_fifo_out->ReadAvailable() >= recv_size)
            {
                slave_frame_fifo_out->Read((char*)(slave_data_tx + 1), recv_size);
            }
            else
            {
                /**clear data buff**/
                memset(slave_data_tx+1,0,recv_size);
            }

            /*write packet head*/
            slave_data_tx->version   = slave_data_rx->version;
            slave_data_tx->ptype     = slave_data_rx->ptype;
            slave_data_tx->seqNumber = slave_data_rx->seqNumber;
            slave_data_tx->ssrc      = slave_data_rx->ssrc;
            slave_data_tx->timeStamp = slave_data_rx->timeStamp;

            /*
            * send out one/more frame
            */

            while(slave_frame_tx < slave_frame_rx+slave_data_latencyPKT)
            {
                slave_frame_tx++;

                retval = slave_data_sock.SendTo(slave_data_tx,sizeof(RTAudioFrame)+recv_size);

                if(retval != (int)(sizeof(RTAudioFrame)+recv_size))
                {
                    printf("OSAudioIP:send frame failed %d,seq=%d\n",
                        retval,OSSock::Ntohs(slave_data_tx->seqNumber));
                }
            }
        }

        /*
        * handle control sock
        */
        retval = slave_ctrl_sock.Recv(&ctrl,sizeof(ctrl));
        if(retval > 0)
        {
            switch(ctrl.msg)
            {
            case RTAF_MSG_HOOK:
                {
                    manager->OnSWHook(ctrl.wparam);
                    break;
                }
            case RTAF_MSG_MODE:
                {
                    manager->OnSWMode(ctrl.wparam);
                    break;
                }
            default:
                {
                    printf("OSAudioIP:unkonwn message from device,0x%08x\n",
                        ctrl.msg);
                    break;
                }
            }
        }
        return true;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};
