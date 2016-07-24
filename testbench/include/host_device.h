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
#ifndef __HOST_DEVICE_H__
#define __HOST_DEVICE_H__


/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace host
{
    /************************************************************************/
    /* For Audio Stuff                                                      */
    /************************************************************************/
    class OSAudioFrame;
    class OSAudioDevice;
    class OSAudioManager;

    /*
    * struct of RTAudioFrame(Real-Time Audio Frame)
    * we all working on network byte order
    */
    typedef struct 
    {
        /*
        * section-1:map to ${RTP} packet
        */
        unsigned char    version;              /*MUST be 0x80*/
        unsigned char    ptype;                /*user define payload type*/
        /*bit0-bit3 :channel number in data*/
#define RTAF_PT_CHAN_MASK          0x0F
#define RTAF_PT_CHAN_SHITF         0

        /*bit0-bit3 :channel data type*/
#define RTAF_PT_DATA_MASK           0xF0
#define RTAF_PT_DATA_SHIFT          4
#define RTAF_PT_DATA_INTERLEAVE     (1<<4)     /*channel data is interleaved,or else is in block*/

        unsigned short   seqNumber;             /*sequence number*/
        unsigned int     timeStamp;             /*time stamp for one PCM channel*/
        unsigned int     ssrc;                  /*SSRC ,should be fix from startup*/

        /*
        * section-2:data
        */
        /*unsigned char    data[4];*/
    }RTAudioFrame;

    typedef struct 
    {
        unsigned int      ssrc;
        unsigned int      msg;
#define RTAF_MSG_NONE       0x0000  
#define RTAF_MSG_HOOK       0x0001  /*from device to host*/
#define RTAF_MSG_MODE       0x0002  /*from device to host*/
#define RTAF_MSG_SPKGAIN    0x0003  /*from host to device*/
#define RTAF_MSG_MICGAIN    0x0004  /*from host to device*/
#define RTAF_MSG_SAMFREQ    0x0005  /*from host to device*/
        unsigned int      wparam;
        unsigned int      lparam;
    }RTAudioCtrl;

    /*
    * class of OSAudioFrame
    */
    class OSAudioFrame
    {
    public:
        enum
        {
            MAX_HZ   = 100,
            MAX_PAD  = 160,
            MAX_CHAN = 6,
        };

    public:
        OSAudioFrame();
        ~OSAudioFrame();
    public:
        bool Create(int fs,int bw,int chan);
        bool Destroy();

        bool DeInterleave(void *sample,int sample_nr,int chan_nr) ;
        bool DoInterleave(void *sample,int sample_nr,int chan_nr) const;

        bool DeBlock(void *sample,int sample_nr,int chan_nr) ;
        bool DoBlock(void *sample,int sample_nr,int chan_nr) const;

        size_t GetAvailable() const 
        {return frameAvailable;};
        size_t GetMaxBytes() const
        {return frameMaxBytes;};
        size_t GetMaxSamples() const
        {return frameMaxSample;};
        void** GetBuffer() const 
        {return (void**)frameSample;};
        void*  GetSample(int chan)
        {
            assert(chan>=0 && chan<MAX_CHAN);
            return frameSample[chan];
        };
        void* operator [](int chan)
        {
            return GetSample(chan);
        };
        OSAudioFrame& operator =(OSAudioFrame &frame)
        {
            assert(frameFS==frame.frameFS);
            assert(frameBW==frame.frameBW);
            assert(frameChan==frame.frameChan);
            for (unsigned int i=0;i<frameChan && frameSample[i]!=NULL;i++)
                memcpy(frameSample[i],frame.frameSample[i],frame.frameAvailable*(frameBW/8));
            return *this;
        };
        OSAudioFrame& operator =(void *frame)
        {
            for (unsigned int i=0;i<frameChan && frameSample[i]!=NULL;i++)
                memcpy(frameSample[i],frame,frameMaxBytes);
            return *this;
        };

    public:
        unsigned long   frameSec;
        unsigned long   frameUSec;
        unsigned int    frameFS;
        unsigned int    frameBW;
        unsigned int    frameChan;

        /*playout*/
        size_t          frameMaxBytes;    /*in bytes*/
        size_t          frameMaxSample;   /*in samples*/
        size_t          frameAvailable;   /*in samples*/
        unsigned char  *frameSample[MAX_CHAN];
    };

    /*
    * class of OSAudioManager
    */
    class OSAudioManager:public OSTime
    {
    public:
        enum
        {
            IPDEV_RE_CTRL_PORT = 22222,/*remote connect control port*/
            IPDEV_RE_DATA_PORT = 22223,/*remote connect data port*/
            IPDEV_LO_CTRL_PORT = 22224,/*local control port,might NOT used*/
            IPDEV_LO_DATA_PORT = 22225,/*local data port,might NOT used*/

            PHONE_RE_DATA_PORT = 50126,/*remote phone data port,extended side*/
            PHONE_LO_DATA_PORT = 50127,/*local phone data port,host side*/
        };

    public:
        OSAudioManager(int Freq=16000,int Chan=2,int bitW=16,int Hz=100/*10ms*/);
        ~OSAudioManager();

    public:
        /*create/destroy*/
        virtual bool InitDevice(const char*IP=NULL,bool bMaster=false);
        virtual bool ExitDevice(void);

        /*data control*/
        virtual bool Start(int ms=0);
        virtual bool Record(int ms=0);
        virtual bool Play(int ms=0);
        virtual bool Stop();
        virtual bool Wait();

        virtual int  PlayoutPCM(void* sample,int sample_nr);
        virtual int  CapturePCM(void* sample,int sample_nr);
        virtual int  FramesPCM (void* playout,void* capture,int sample_nr,bool bInterleaved=true);

        /*get&set*/
        OSAudioDevice& GetDevice();
        OSAudioDevice& SetDevice(OSAudioDevice &device);

        int  SetHz(int Hz);
        int  GetHz();
        int  SetFreq(int Freq);
        int  GetFreq();
        int  SetBitW(int bitW);
        int  GetBitW();
        int  SetDev(int devIndex);
        int  GetDev();
        int  SetChan(int devIndex);
        int  GetChan();
        int  SetLatency(int devLatency);
        int  GetLatency();
        int  SetDevIP(const char* IP);
        int  GetDevIP(char* IP);
        bool SetDevMaster(bool bMaster);
        bool GetDevMaster();

        int  GetPlayoutVolume();
        int  SetPlayoutVolume(int again, int dgain = -1);
        int  GetCaptureVolume();
        int  SetCaptureVolume(int again, int dgain = -1);
        int  GetPlayoutDelay();
        int  GetCaptureDelay();

    public:

        virtual int  OnPlayout(OSAudioFrame &playout_frames){return 0;};
        virtual int  OnCapture(OSAudioFrame &capture_frames){return 0;};
        virtual int  OnFrames (OSAudioFrame &capture_frames,OSAudioFrame &playout_frames){return 0;};
        virtual void OnSWHook (int hook){};
        virtual void OnSWMode (int mode){};

    protected:

        OSAudioDevice  *device;
        unsigned int    deviceHz;
        unsigned int    deviceFreq;
        unsigned int    deviceBW;
        unsigned int    deviceIndex;
        unsigned int    deviceChannel;
        unsigned int    deviceLatency;
        char            deviceIP[256];
        bool            deviceMaster;

        /*frame buffer*/
        OSAudioFrame    capture_frame;
        OSAudioFrame    playout_frame;
    };

    /*
    * class of OSAudioDevice
    */
    class OSAudioDevice
    {
    public:
        enum Limits
        {
            MAX_DEVNAME = 128,
            MAX_GUIDLEN = 128,
        };
        enum Volume
        {
            MIN_VOLUME  = 0,
            MAX_VOLUME  = 100,
        };

    public:
        static bool CreatePlatformDevice (OSAudioDevice *&pdev,OSAudioManager& mng);
        static bool DestroyPlatformDevice(OSAudioDevice *&pdev);

        static bool CreateRemoteIPDevice (OSAudioDevice *&pdev,OSAudioManager& mng);
        static bool DestroyRemoteIPDevice(OSAudioDevice *&pdev);

    public:
        /*init*/
        virtual bool Init()=0;

        /* Device enumeration*/
        virtual int  GetPlayoutDevices()=0;
        virtual int  GetCaptureDevices()=0;

        virtual int  GetPlayoutDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN])=0;
        virtual int  GetCaptureDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN])=0;

        /*Device status*/
        virtual bool IsPlayoutOpen()=0;
        virtual bool IsCaptureOpen()=0;
        virtual bool IsPlayoutRunning()=0;
        virtual bool IsCaptureRunning()=0;

        /*Device open&close*/
        virtual bool OpenPlayoutDevice()=0;
        virtual bool OpenCaptureDevice()=0;

        virtual bool ClosePlayoutDevice()=0;
        virtual bool CloseCaptureDevice()=0;

        /*Device start&stop*/
        virtual bool StartPlayout(unsigned long ms=0)=0;
        virtual bool StartCapture(unsigned long ms=0)=0;
        virtual bool StopPlayout()=0;
        virtual bool StopCapture()=0;
        virtual bool WaitPlayout()=0;
        virtual bool WaitCapture()=0;

        virtual bool StartFraming(unsigned long ms=0)=0;
        virtual bool StopFraming()=0;
        virtual bool WaitFraming()=0;

        /*Volume control*/
        virtual bool GetPlayoutVolume(int*again, int*dgain = NULL)=0;
        virtual bool SetPlayoutVolume(int again, int dgain = -1)  =0;
        virtual bool GetCaptureVolume(int*again, int*dgain = NULL)=0;
        virtual bool SetCaptureVolume(int again, int dgain = -1)  =0;

        virtual int  GetPlayoutDelay(void)=0;
        virtual int  GetCaptureDelay(void)=0;

        /*thread callback*/
        virtual bool PCM_BeforePlayout(void)=0;
        virtual bool PCM_BeforeCapture(void)=0;
        virtual bool PCM_BeforeFraming(void)=0;
        virtual bool PCM_AfterPlayout(void)=0;
        virtual bool PCM_AfterCapture(void)=0;
        virtual bool PCM_AfterFraming(void)=0;
        virtual bool PCM_Playout(bool wait=true)=0;
        virtual bool PCM_Capture(bool wait=true)=0;
        virtual bool PCM_Framing(bool wait=true)=0;
    };

    /*
    * class of OSAudioRemote
    */
    class OSAudioRemote:
        public OSAudioManager
    {
    public:
        OSAudioRemote(const char* strIP,int freq=16000,int chan=2,int bw=16);
        ~OSAudioRemote();

    public:
        virtual int OnFrames(OSAudioFrame &capture_frames,OSAudioFrame &playout_frames);
        virtual int OnFramesRX(RTAudioFrame &frame){return 0;};
        virtual int OnFramesTX(RTAudioFrame &frame){return 0;};
    
    protected:
        OSSock          ctrl_sock;
        OSSock          data_sock;
        size_t          data_total;
        size_t          data_fs;
        size_t          data_bytes;
        size_t          data_missed;

        RTAudioFrame   *data_pkt;
        unsigned short  data_tx_seq;
        unsigned int    data_tx_ssrc;
        unsigned int    data_tx_ts;
    };

    
    /*
    * class of OSAudioPhone
    */
    class OSAudioPhone:
        public OSAudioManager
    {
    public:
        enum Limits
        {
            MAX_FREQ  = 32000,
            MIN_PTIME = 10,
            MAX_PTIME = 40,
            MAX_BUFFER= (MAX_FREQ/100)*MAX_PTIME,
        };
    public:
        OSAudioPhone(const char* strIP,int ptime,int freq=16000,const char *remoteDeviceIP=NULL);
        ~OSAudioPhone();

        bool Start();
        bool Stop();


    protected:
        virtual int OnFrames(OSAudioFrame &capture_frames,OSAudioFrame &playout_frames);
    
    protected:
        OSSock          data_sock;
        RTAudioFrame   *data_frame;
        size_t          data_framesize;
        size_t          data_sequence;

        short           rx_buffer[MAX_BUFFER+160];
        size_t          rx_buffer_wr;
        size_t          rx_buffer_rd;
        size_t          rx_buffer_ptime;
        size_t          rx_downflow;
        size_t          rx_overflow;

        short           tx_buffer[MAX_BUFFER+160];
        size_t          tx_buffer_wr;
        size_t          tx_buffer_ptime;
        size_t          tx_buffer_ts;
        size_t          tx_missing;
    };

    /************************************************************************/
    /* For Video Stuff                                                      */
    /************************************************************************/
    /*
    * class of OSVideoDevice
    */
    class OSVideoDevice
    {
    public:
        enum Limits
        {
            MAX_DEVNAME = 128,
            MAX_GUIDLEN = 128,
        };

    public:
        static bool CreatePlatformDevice (OSVideoDevice *&pdev,OSAudioManager& mng);
        static bool DestroyPlatformDevice(OSVideoDevice *&pdev);

    public:

        /* Device enumeration*/
        virtual int  GetPlayoutDevices()=0;
        virtual int  GetCaptureDevices()=0;

        virtual int  GetPlayoutDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN])=0;
        virtual int  GetCaptureDeviceName(int index,char name[MAX_DEVNAME],char guid[MAX_GUIDLEN])=0;

        /*Device open&close*/
        virtual bool OpenPlayoutDevice()=0;
        virtual bool OpenCaptureDevice()=0;

        virtual bool ClosePlayoutDevice()=0;
        virtual bool CloseCaptureDevice()=0;
    };

}; /*end of host*/
/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*__HOST_DEVICE_H__*/
