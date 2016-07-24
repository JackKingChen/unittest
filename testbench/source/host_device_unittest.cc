/********************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2012-04-21
*
*******************************************************************/

/*local*/
#include "unittest.h"

namespace unittest
{
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    class MyAudio:public OSAudioManager
    {
    public:
        MyAudio(int freq=16000,int bw=16):OSAudioManager(freq,2,bw)
        {
            dolooping = false;
            OSAudioManager::InitDevice();
        };
        ~MyAudio()
        {
            capture_file.Update();
        };

        virtual int OnCapture(OSAudioFrame &frames)
        {

            capture_file.Write(frames[0],frames[1],frames.GetAvailable());

            return 0;
        };

        virtual int OnPlayout(OSAudioFrame &frames)
        {
            int retval = playout_file.Read(frames[0],frames[1],frames.GetAvailable());
            if(retval<(int)frames.GetAvailable())
            {
                playout_file.Seek(0);
                retval = playout_file.Read(frames[0],frames[1],frames.GetAvailable());
            }
            return retval;
        };

        virtual int OnFrames(OSAudioFrame &capture_frames,OSAudioFrame &playout_frames)
        {
            if(dolooping)
            {
                playout_frames = capture_frames;
            }
            else
            {
                int samples;
                int retval;

                /*save one frame*/
                samples = capture_frames.GetAvailable();
                retval  = capture_file.Write(capture_frames[0],capture_frames[1],samples);

                /*play one frame*/
                samples = playout_frames.GetAvailable();
                retval  = playout_file.Read(playout_frames[0],playout_frames[1],samples);
                if(retval<samples)
                {
                    playout_file.Seek(0);
                    retval  = playout_file.Read(playout_frames[0],playout_frames[1],samples);
                }
            }
            return 0;
        }

    public:
        bool RecordWave(const char *path,int ms=0)
        {
            capture_file.Create(path,GetChan(),GetFreq(),GetBitW());

            SetDev(0);
            Record(ms);
            SetCaptureVolume(90);

            return true;
        }

        bool PlayWave(const char *path,int ms=0)
        {
            playout_file.Open(path);

            SetDev(0);
            Play(ms);
            SetPlayoutVolume(100);

            return true;
        }
        bool FramePCM(const char *pfile,const char *cfile,int ms=0)
        {
            capture_file.Create(cfile,GetChan(),GetFreq(),GetBitW());
            playout_file.Open(pfile);

            dolooping = false;

            Start(ms);
            SetPlayoutVolume(100);
            SetCaptureVolume(100);

            return true;
        }
        bool LoopPCM(int ms=0)
        {
            dolooping = true;

            Start(ms);
            SetPlayoutVolume(100);
            SetCaptureVolume(100);

            return true;
        }
    protected:
        bool     dolooping;
        WaveFile playout_file;
        WaveFile capture_file;
    };

    /************************************************************************/
    /* test for enum audio device                                           */
    /************************************************************************/
    UNITTEST_HIDE(adev_list)
    {
        MyAudio dev;
        char    name[512];

        for (int i=0;i<dev.GetDevice().GetPlayoutDevices();i++)
        {
            dev.GetDevice().GetPlayoutDeviceName(i,name,NULL);
            printf("playout-%d:%s\n",i,name);
        }

        for (int i=0;i<dev.GetDevice().GetCaptureDevices();i++)
        {
            dev.GetDevice().GetCaptureDeviceName(i,name,NULL);
            printf("capture-%d:%s\n",i,name);
        }
    }

    UNITTEST_HIDE(adev_rec)
    {
        MyAudio dev(16000,16);

        dev.RecordWave("./rec.wav",4*1000);

        OSTTY::getkey();
    }

    UNITTEST_HIDE(adev_play)
    {
        MyAudio dev(16000,16);

        dev.PlayWave("./runtest16.wav");

        OSTTY::getkey();
    }

    UNITTEST_HIDE(adev_loop)
    {
        MyAudio dev(16000,16);

        dev.LoopPCM();

        OSTTY::getkey();
    }

    UNITTEST_HIDE(adev)
    {
        MyAudio dev(16000,16);

        dev.FramePCM("./runtest16.wav","./rec.wav",10*1000);

        OSTTY::getkey();
    }

    /************************************************************************/
    /* test for remote audio device                                         */
    /************************************************************************/
    class ReAudio:
        public OSAudioRemote
    {
    public:
        ReAudio(const char* strIP,int freq=16000,int chan=2,int bw=16)
            :OSAudioRemote(strIP,freq,chan,bw)
        {
            
        };
        ~ReAudio()
        {
            
        };

    public:
        virtual int OnFramesRX(RTAudioFrame &frame)
        {
            unsigned int chan      = ((frame.ptype)&RTAF_PT_CHAN_MASK)>>RTAF_PT_CHAN_SHITF;
            unsigned int inte      = !!((frame.ptype)&RTAF_PT_DATA_INTERLEAVE);
            unsigned int seqno     = OSSock::Ntohs(frame.seqNumber);
            unsigned int timestamp = OSSock::Ntohl(frame.timeStamp);
            unsigned int ssrc      = OSSock::Ntohl(frame.ssrc);

            assert(chan==deviceChannel);
            assert(!inte);

            if(data_tx_seq%100==0)
            {
                unsigned int net_delay = ((data_tx_ts-timestamp)/data_fs)*10;
                unsigned int tx_delay  = GetPlayoutDelay()+net_delay;
                unsigned int rx_delay  = GetCaptureDelay()+net_delay;

                printf("Recv:ssrc=0x%08x,seq=%d,ts=%d,TXdelay=%d ms,RXdelay=%d ms\n",
                    ssrc,seqno,timestamp,tx_delay,rx_delay);
            }

            return 0;
        };
        virtual int OnFramesTX(RTAudioFrame &frame)
        {
            return 0;
        };
    };

    UNITTEST_HIDE(rdev_remote)
    {
        ReAudio re("127.0.0.1");

        re.Start();

        OSTTY::getkey();
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    class LoAudio:
        public OSAudioManager
    {
    public:
        LoAudio(const char* strIP,int freq=16000,int chan=2,int bw=16)
            :OSAudioManager(freq,chan,bw)
        {

            /*init local audio*/
            OSAudioManager::InitDevice(strIP,false);
        };
        ~LoAudio()
        {

        };

    public:
        virtual int OnFrames(OSAudioFrame &capture_frames,OSAudioFrame &playout_frames)
        {

            printf("OnFrames() SpkDelay=%d,MicDelay=%d...\n",
                GetPlayoutDelay(),GetCaptureDelay());

            //memcpy(playout_frames[0],capture_frames[0],(deviceFreq/100)*2);

            return 0;
        };

    protected:

    };

    UNITTEST_HIDE(rdev_local)
    {
        LoAudio lo("10.2.10.84",16000,1);

        lo.SetLatency(60);
        lo.Start();

        OSTTY::getkey();
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    class LoAudioVolume:
        public OSAudioManager
    {
    public:
        LoAudioVolume(const char* strIP, int freq=16000, int chan=2, int bw=16)
            :OSAudioManager(freq,chan,bw),again_type(1),dgain_type(2),
            min_test_volume(-3),max_test_volume(105)
            
        {
            /*init local audio*/
            OSAudioManager::InitDevice(strIP,false);
            multi_test = 0;
            capture_again = 0;
            capture_dgain = 0;
            playout_again = 0;
            playout_dgain = 0;
        };
        ~LoAudioVolume()
        {
        };

    public:

        bool Init(const char* playout_file_name, const char* capture_file_name)
        {
            bool res;

            res = playout_file.Open(playout_file_name);

            if (!res)
            {
                printf("fail to open the wave file: %s\n", playout_file_name);
                return false;
            }

            res = capture_file.Create(capture_file_name,GetChan(),GetFreq(),GetBitW());

            if (!res)
            {
                printf("fail to create the wave file: %s\n", capture_file_name);
                return false;
            }

            return true;
        }

        void exit()
        {
            playout_file.close();
            capture_file.close();
        }

        virtual int OnFrames(OSAudioFrame &capture_frames,OSAudioFrame &playout_frames)
        {

            unsigned int read_size;
            unsigned int write_size;

            if (multi_test)
            {
                memcpy(playout_frames[0],capture_frames[0],(deviceFreq/100)*2);
                return 0;
            }

            read_size = playout_file.read(playout_frames[0],(deviceFreq/100)*2);

            if (read_size == 0)
            {
                playout_file.close();
                playout_file.Open("./runtest16.wav");
            }

            write_size = capture_file.write(capture_frames[0],(deviceFreq/100)*2);

            return 0;
        };

        virtual void OnSWHook(int hook)
        {
            printf("on hook %s\n",(hook == 0) ? "handfree mode":"handset mode");
        }

        void CaptureVolumeUp(int gain_type)
        {
            if (capture_again < max_test_volume && gain_type == again_type)
            {
                capture_again++;
            }
            if (capture_dgain < max_test_volume && gain_type == dgain_type)
            {
                capture_dgain++;
            }
            
            
            SetCaptureVolume(capture_again, capture_dgain);
            printf("Set capture again :%d dgain :%d \n", capture_again, capture_dgain);
        }

        void CaptureVolumeDown(int gain_type)
        {
            if (capture_again > min_test_volume && gain_type == again_type)
            {
                capture_again--;
            }
            if (capture_dgain > min_test_volume && gain_type == dgain_type)
            {
                capture_dgain--;
            }
            
            
            SetCaptureVolume(capture_again, capture_dgain);
            printf("Set capture again :%d dgain :%d \n", capture_again, capture_dgain);
        }

        void PlayoutVolumeUp(int gain_type)
        {
            if (playout_again < max_test_volume && gain_type == again_type)
            {
                playout_again++;
            }
            if (playout_dgain < max_test_volume && gain_type == dgain_type)
            {
                playout_dgain++;
            }
            
            
            SetPlayoutVolume(playout_again, playout_dgain);
            printf("Set playout again :%d dgain :%d\n", playout_again,playout_dgain);
        }

        void PlayoutVolumeDown(int gain_type)
        {
            if (playout_again > min_test_volume && gain_type == again_type)
            {
                playout_again--;
            }
            if (playout_dgain > min_test_volume && gain_type == dgain_type)
            {
                playout_dgain--;
            }
            
            
            SetPlayoutVolume(playout_again, playout_dgain);
            printf("Set play again :%d dgain :%d\n", playout_again, playout_dgain);
        }

        void SetMultiTest(bool multi_switch)
        {
            multi_test = multi_switch;
            printf("multi test capture and playout {%s}\n",multi_switch? "YES":"NO");
        }

    protected:
        int       capture_again;
        int       capture_dgain;
        int       playout_again;
        int       playout_dgain;
        bool      multi_test;
        WaveFile  playout_file;
        WaveFile  capture_file;
        
        const int again_type;
        const int dgain_type;
        const int min_test_volume;  /*include boundary test -1~-3*/
        const int max_test_volume; /*include boundary test 101 ~ 105*/

    };

    UNITTEST_HIDE(rdev_volume)
    {
    #define KEY_UP     128
    #define KEY_DOWN   129
    #define KEY_LEFT   131
    #define KEY_RIGHT  130
    #define KEY_2      50
    #define KEY_4      52
    #define KEY_6      54
    #define KEY_8      56
    #define KEY_M      109
    #define KEY_S      115

    #define TEST_SAMPLE_RATE_8K  8000
    #define TEST_SAMPLE_RATE_16K 16000
    #define TEST_SAMPLE_RATE_32K 32000


        int  freq;
        int  key;
        bool res;
        const int again_type = 1;
        const int dgain_type = 2;
        if (argv[0] != NULL)
        {
            freq = atoi(argv[0]);
        }
        else
        {
            freq = 16000;
        }

        if (freq != TEST_SAMPLE_RATE_8K && 
            freq != TEST_SAMPLE_RATE_16K && 
            freq != TEST_SAMPLE_RATE_32K)
        {
            printf("abnormal test freq %d\n", freq);
        }


        LoAudioVolume lo_volume("10.2.10.51",freq,1);

        res = lo_volume.Init("./runtest16.wav","./volume_rec.wav");

        if (!res)
        {
            printf("init fail\n");
            return;
        }

        lo_volume.SetLatency(60);
        lo_volume.Start();

        while(OSTTY::getkey(key))
        {

            switch(key)
            {
            case KEY_UP: /*palyout volume up   '¡ü' key*/
                lo_volume.PlayoutVolumeUp(again_type);
                break;

            case KEY_DOWN: /*playout volume down '¡ý' key*/
                lo_volume.PlayoutVolumeDown(again_type);
                break;

            case KEY_LEFT: /*capture volume up   '¡û' key*/
                lo_volume.CaptureVolumeUp(again_type);
                break;

            case KEY_RIGHT: /*capture volume down '¡ú' key*/
                lo_volume.CaptureVolumeDown(again_type);
                break;

            case KEY_8: /*Playout dgain up '8' key*/
                lo_volume.PlayoutVolumeUp(dgain_type);
                break;

            case KEY_2: /*Playout dgain down '2' key*/
                lo_volume.PlayoutVolumeDown(dgain_type);
                break;

            case KEY_4: /*capture volume up   '4' key*/
                lo_volume.CaptureVolumeUp(dgain_type);
                break;

            case KEY_6: /*capture volume down  '6' key*/
                lo_volume.CaptureVolumeDown(dgain_type);
                break;

            case KEY_M: /*sound loop test     'm' key*/
                lo_volume.SetMultiTest(true);
                break;

            case KEY_S: /*single way test     's' key*/
                lo_volume.SetMultiTest(false);
                break;
            default:
                printf("\nplayout again{¡ü ¡ý} capture again{¡û ¡ú}\n");
                printf("playout dgain{'8','2'} capture dgain{'4','6'}\n");
                printf("playfile{'s'} multi{'m'} quit{'esc'}\n");
                break;
            }
        }
        lo_volume.exit();
    }
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    UNITTEST_HIDE(extphone_remote)
    {
        OSAudioPhone ExtPhone("127.0.0.1",20);

        ExtPhone.Start();

        OSTTY::getkey();
    }

    UNITTEST(localtime)
    {
    
        while(OSTTY::getkey())
        {
            OSTime now;

            printf("sec=%u,usec=%u\n",now.tv_sec,now.tv_usec);
        }

    }
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};
/************************************************************************/
/*                                                                      */
/************************************************************************/

