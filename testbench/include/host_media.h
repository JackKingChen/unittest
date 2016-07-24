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
#ifndef __HOST_MEDIA_H__
#define __HOST_MEDIA_H__

#include "host_wave.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace host
{
    /*
    * class of WaveFile
    */
    class WaveFile :public OSFile
    {
    public:
        WaveFile();
        WaveFile(const char *path);
        WaveFile(const char *path,int channel,int sample_rate,int sample_precision,int byte_rate,int codec=WAVE_PTYPE_PCM);
        WaveFile(const char *path,int channel,int sample_rate,int sample_precision=16,int codec=WAVE_PTYPE_PCM);
        WaveFile(WaveFile&src);
        ~WaveFile();

    public:
        bool Create(const char *path,int channel,int sample_rate,int sample_precision,int byte_rate,int codec=WAVE_PTYPE_PCM);
        bool Create(const char *path,int channel=1,int sample_rate=16000,int sample_precision=16,int codec=WAVE_PTYPE_PCM);
        bool Clone (const char *path,WaveFile&src);
        bool Open(const char *path);
        void Close();
        void Info();

        int  Total();
        int  TotalFile();
        int  TotalSample();

        /*file ops*/
        bool Seek (int pos);
        int  Read (void* l_buff,void* r_buff,int sample);
        int  Write(void* l_buff,void* r_buff,int sample);
        int  Update(void);
        int  Update(int channel,int sample_rate,int sample_precision,int byte_rate,int codec=WAVE_PTYPE_PCM);

        /*loop back mode*/
        int  ReadLoop (void* l_buff,void* r_buff,int sample,bool bLoop=true);
        int  WriteLoop(void* l_buff,void* r_buff,int sample,bool bLoop=true);

        /*info*/
        size_t Wav_CodeType()      {return wav_hdr.codec_type;};
        size_t Wav_Channels()      {return wav_hdr.channels;};
        size_t Wav_Samples()       {return ((wav_hdr.data_payload)/(wav_hdr.precision/8))/wav_hdr.channels;};
        size_t Wav_SampleRate()    {return wav_hdr.sample_rate;};
        size_t Wav_BytesRate()     {return wav_hdr.byte_rate;};
        size_t Wav_BitsPerSample() {return wav_hdr.precision;};
        size_t Wav_BytesPerSample(){return wav_hdr.precision/8;};
        size_t Wav_OffsetBegin()   {return wav_hdr.head_size;};
        size_t Wav_OffsetEnd()     {return wav_hdr.head_size+wav_hdr.data_payload;};

        /*static helper*/
        static bool PESQ(const char*refFile,const char*degFile,
            double &pesqMOS,double &mapMOS,int wb=-1);

        static double PESQMos(const char*refFile,const char*degFile)
        {
            double pesqMOS;
            double mapMOS;
            
            if(WaveFile::PESQ(refFile,degFile,pesqMOS,mapMOS,0))
            {
                return pesqMOS;
            }
            return 0.0;
        };

        static double MappedMos(const char*refFile,const char*degFile)
        {
            double pesqMOS;
            double mapMOS;

            if(WaveFile::PESQ(refFile,degFile,pesqMOS,mapMOS,1))
            {
                return mapMOS;
            }
            return 0.0;
        };

    protected:
        wave_t wav_hdr;
    };

    /*
    * class of WaveBuff
    */
    class WaveBuff:public WaveFile
    {
    public:
        WaveBuff(const char *path);
        WaveBuff();
        ~WaveBuff();

    public:
        int    Load(int samples=-1,int channel=0);
        int    Save(void);
        int    Count(int channel=0);

        void  *ToSample(int pos=0,int channel=0,int sbw=16);

        int   *ToSample32(int pos=0,int channel=0)
        {return (int*)ToSample(pos,channel,32);};
        short *ToSample16(int pos=0,int channel=0)
        {return (short*)ToSample(pos,channel,16);};

        int    GetSample(int pos,void*sb,int sblen,int channel=0,int sbw=16);
        int    PutSample(int pos,const void*sb,int sblen,int channel=0,int sbw=16);

        int    GetSample32(int pos,int*sbuff,int bufflen,int channel=0)
        {return GetSample(pos,sbuff,bufflen,channel,32);};
        int    GetSample16(int pos,short*sbuff,int bufflen,int channel=0)
        {return GetSample(pos,sbuff,bufflen,channel,16);};
        int    PutSample32(int pos,const int*sbuff,int bufflen,int channel=0)
        {return PutSample(pos,sbuff,bufflen,channel,32);};
        int    PutSample16(int pos,const short*sbuff,int bufflen,int channel=0)
        { return PutSample(pos,sbuff,bufflen,channel,16);};

    protected:
        int    Init(void);
        int    Free(void);

    protected:
        void   *sample_buff[2];
        int     sample_blen[2];
        int     sample_count[2];
    };

    /*
    * class of RingBuff
    */
    class RingBuff
    {
    public:
        enum
        {
            SAME_WRAP   =0,
            DIFF_WRAP   =1,
        };

    public:
        RingBuff(size_t count,size_t size);
        ~RingBuff();

    public:
        void   Reset();
        size_t Read (void *buff,size_t size);
        size_t Write(void *buff,size_t size);
        size_t ReadAvailable();
        size_t WriteAvailable();

    protected:
        int       elem_wrap;
        size_t    elem_read;
        size_t    elem_write;
        size_t    elem_count;
        size_t    elem_size;
        char     *elem_data;
    };

    /*
    * class of WaveDump
    */
    class WaveDump:public WaveFile
    {
    public:
        WaveDump(const void *framebuff,size_t framesize,const char*path,int scduleHz);
        WaveDump(const char *path,int samplerate);
        WaveDump(const char *path);
        ~WaveDump();

    public:
        bool Equal(const char* path);
        bool Dump(void);

    public:
        /*trace for frame buffer*/
        const void * pcmframe;
        int          pcmframesize;
        char         pcmPath[256];
    };

    class WaveDumpList: public std::list<WaveDump*>
    {
    public:
        WaveDumpList();
        ~WaveDumpList();

    public:

        bool Add(const void *framebuff,size_t framesize,const char*path,int scduleHz=100);
        bool Add(const char *path,int samplerate);
        bool Delete(const char*path);
        bool Clear();
        bool Dump();
        bool Dump(const char *path,int samplerate,void*sample,int sampleNr);
    };

    class WaveReadList: public std::list<WaveDump*>
    {
    public:
        WaveReadList();
        ~WaveReadList();

    public:
        bool Delete(const char*path);
        bool Read(const char *path,int samplerate,void*sample,int sampleNr);
    };

}; /*end of host*/
/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*__HOST_MEDIA_H__*/
