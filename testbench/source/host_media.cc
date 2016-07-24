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

/*for internal*/
#include "pesq_api.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace host
{

    /************************************************************************/
    /* class of WaveFile                                                    */
    /************************************************************************/
    WaveFile::WaveFile()
    {
        memset(&wav_hdr,0,sizeof(wav_hdr));
    }

    WaveFile::WaveFile(const char *path)
    {
        WaveFile();
        open(path,"rb+");
    }

    WaveFile::WaveFile(const char *path,int channel,int sample_rate,int sample_precision,int byte_rate,int codec)
    {
        WaveFile();
        Create(path,channel,sample_rate,byte_rate,sample_precision,codec);
    }

    WaveFile::WaveFile(const char *path,int channel,int sample_rate,int sample_precision,int codec)
    {
        WaveFile();
        Create(path,channel,sample_rate,sample_precision,(sample_rate*(sample_precision/2)*channel),codec);
    }

    WaveFile::WaveFile(WaveFile&src)
    {
        WaveFile();
        wav_hdr = src.wav_hdr;
    }

    WaveFile::~WaveFile()
    {
        /*update anyway*/
        Update();
    }

    bool WaveFile::Create(const char *path,int channel,int sample_rate,int sample_precision,int byte_rate,int codec)
    {
        /*for safe*/
        wav_hdr.codec_type  = codec;
        wav_hdr.channels    = channel;
        wav_hdr.sample_rate = sample_rate;
        wav_hdr.byte_rate   = byte_rate;
        wav_hdr.precision   = sample_precision;
        wav_hdr.data_payload= 0;
        wav_hdr.head_size   = sizeof(wav_hdr);

        /*try open*/
        open(path,"wb+");

        if(is_open())
            return (wave_write_header(*this,&wav_hdr)==0);
        else
            return false;
    }
    bool WaveFile::Create(const char *path,int channel,int sample_rate,int sample_precision,int codec)
    {
        return Create(path,channel,sample_rate,sample_precision,(sample_rate*(sample_precision/2)*channel),codec);
    }

    bool WaveFile::Clone(const char *path,WaveFile&src)
    {
        return Create(path,src.Wav_CodeType(),src.Wav_Channels(),
            src.Wav_SampleRate(),src.Wav_BytesRate(),src.Wav_BitsPerSample());
    }

    bool WaveFile::Open(const char *path)
    {
        /*try open*/
        open(path,"rb");

        if(is_open())
            return (wave_read_header(*this,&wav_hdr)==0);
        else
            return false;
    }
    void WaveFile::Close()
    {
        close();
    }

    void WaveFile::Info()
    {
        printf("is_open     = %d\n",is_open());
        printf("codec_type  = %d\n",wav_hdr.codec_type);
        printf("channels    = %d\n",wav_hdr.channels);
        printf("sample_rate = %d\n",wav_hdr.sample_rate);
        printf("byte_rate   = %d\n",wav_hdr.byte_rate);
        printf("precision   = %d\n",wav_hdr.precision);
        printf("file_size   = %d\n",wav_hdr.file_size);
        printf("head_size   = %d\n",wav_hdr.head_size);
        printf("data_payload= %d\n",wav_hdr.data_payload);
    }

    bool  WaveFile::Seek (int pos)
    {
        if(is_open() && pos>=0 && pos<wav_hdr.data_payload)
        {
            seek(OSFile::beg,(int)(wav_hdr.head_size+pos));
            return true;
        }
        return false;
    }

    int  WaveFile::Total()
    {
        if(is_open())
            return wav_hdr.data_payload;
        else
            return 0;
    }

    int  WaveFile::TotalFile()
    {
        return total();
    }

    int  WaveFile::TotalSample()
    {
        if(is_open())
            return (wav_hdr.data_payload/((wav_hdr.precision/8)*wav_hdr.channels));
        else
            return 0;
    }

    int  WaveFile::Read(void* l_buff,void* r_buff,int sample)
    {
        char   elem_data[32];
        char  *lbuff = (char*)l_buff;
        char  *rbuff = (char*)r_buff;
        size_t total = 0;
        size_t pbyte = wav_hdr.precision/8;
        size_t rbyte = pbyte*wav_hdr.channels;

        if(pbyte<=0 || pbyte>4)
            return -EFAULT;

        while(sample-- > 0 && tell()<Wav_OffsetEnd())
        {
            if(read(elem_data,rbyte)!=rbyte)
                break;

            if(l_buff)
            {
                memcpy(lbuff,elem_data+0,pbyte);
                lbuff+=pbyte;
            }

            if(r_buff)
            {
                if(wav_hdr.channels>1)
                    memcpy(rbuff,elem_data+pbyte,pbyte);
                else
                    memcpy(rbuff,elem_data+0    ,pbyte);
                rbuff+=pbyte;
            }

            total++;
        }

        return total;
    }

    int  WaveFile::Write(void* l_buff,void* r_buff,int sample)
    {
        char   elem_data[32];
        char  *lbuff = (char*)l_buff;
        char  *rbuff = (char*)r_buff;
        size_t total = 0;
        size_t pbyte = wav_hdr.precision/8;
        size_t wbyte = 0;

        if(pbyte<=0 || pbyte>4)
            return -EFAULT;

        while(sample-- > 0)
        {
            wbyte = 0;
            if(l_buff)
            {
                memcpy(elem_data+0,lbuff,pbyte);
                lbuff+=pbyte;
                wbyte+=pbyte;
            }

            if(r_buff && wav_hdr.channels>1)
            {
                memcpy(elem_data+pbyte,rbuff,pbyte);
                rbuff+=pbyte;
                wbyte+=pbyte;
            }

            if(write(elem_data,wbyte)<wbyte)
                break;

            total++;
        }

        return total;
    }

    int   WaveFile::Update(void)
    {
        wav_hdr.file_size = TotalFile();

        if(wav_hdr.file_size>0 && is_writeable())
        {
            size_t current = tell();
            wave_write_header(*this,&wav_hdr);
            seek(OSFile::beg,current);
            flush();
            return true;
        }
        return -EFAULT;
    }

    int  WaveFile::Update(int channel,int sample_rate,int sample_precision,int byte_rate,int codec)
    {
        wav_hdr.file_size   = TotalFile();
        wav_hdr.codec_type  = codec;
        wav_hdr.channels    = channel;
        wav_hdr.sample_rate = sample_rate;
        wav_hdr.byte_rate   = byte_rate;
        wav_hdr.precision   = sample_precision;
        wav_hdr.data_payload= 0;
        wav_hdr.head_size   = 0;

        if(wav_hdr.file_size>0 && is_writeable())
        {
            size_t current = tell();
            wave_write_header(*this,&wav_hdr);
            seek(OSFile::beg,current);
            return true;
        }
        flush();
        return -EFAULT;
    }

    int  WaveFile::ReadLoop(void* l_buff,void* r_buff,int sample,bool bLoop)
    {
        if(bLoop)
        {
            if(Read(l_buff,r_buff,sample)<sample)
            {
                Seek(0);
                return Read(l_buff,r_buff,sample);
            }
            return sample;
        }
        else
        {
            return Read(l_buff,r_buff,sample);
        }
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    bool WaveFile::PESQ(const char*refPath,const char*degPath,
        double &pesqMOS,double &mapMOS,int wb)
    {
        PESQ_t         pesq;
        PESQ_Wave_t   *ref_info = &pesq.ref_info;
        PESQ_Wave_t   *deg_info = &pesq.deg_info;
        PESQ_Result_t *err_info = &pesq.err_info;
        WaveFile       refFile;
        WaveFile       degFile;
        int  fs;
        bool ok;

        pesqMOS = 0.0;
        mapMOS  = 0.0;

        /*
        * open file
        */
        if(!refFile.Open(refPath))
            return false;
        if(!degFile.Open(degPath))
            return false;
        
        if(wb<0)
        {
            if(refFile.Wav_SampleRate()!=degFile.Wav_SampleRate())
                return false;

            fs = refFile.Wav_SampleRate();
        }
        else
        {
            fs = wb?16000:8000;
        }

        /*
        * load source
        */
        if(pesq_init(&pesq,fs)<0)
            return false;

        /*
        * config
        */
        ref_info->input_filter = 1;
        deg_info->input_filter = 1;
        err_info->mode         = fs==16000?PESQ_WB_MODE:PESQ_NB_MODE;

        ok = (pesq_load_data(&pesq,refFile,degFile)==0);
        if(ok)
        {
            /*
            * do measure
            */
            ok = (pesq_measure(&pesq,ref_info, deg_info, err_info)==0);
            if(ok)
            {
                pesqMOS = err_info->pesq_mos;
                mapMOS  = err_info->mapped_mos;
            }
        }

        /*
        * do cleanup 
        */
        pesq_free(&pesq);

        return ok;
    }


    /************************************************************************/
    /* class of WaveBuff                                                    */
    /************************************************************************/
    WaveBuff::WaveBuff()
    {
        Init();
    }
    WaveBuff::WaveBuff(const char *path)
    {
        Init();
        Open(path);
    }
    WaveBuff::~WaveBuff()
    {
        Free();
    }

    int WaveBuff::Init(void)
    {
        for(int i=0;i<(int)TABLESIZE(sample_buff);i++)
        {
            sample_buff[i]  = NULL;
            sample_blen[i]  = 0;
            sample_count[i] = 0;
        }
        return 0;
    }

    int WaveBuff::Load(int samples,int channel)
    {
        int retval;

        /*free safe*/
        Free();

        if(samples<=0)
            samples = TotalSample();

        if(channel<0 || channel>wav_hdr.channels)
            return -EFAULT;

        /*alloc buffer*/
        sample_count[channel]= samples;
        sample_blen[channel] = samples*(wav_hdr.precision/2);
        sample_buff[channel] = malloc(sample_blen[channel]+16);
        if(!sample_buff[channel])
            return -ENOMEM;

        if(samples <= TotalSample())
        {
            if(channel==0)
                retval = Read(sample_buff[channel],NULL,TotalSample());
            else
                retval = Read(NULL,sample_buff[channel],TotalSample());

            if(retval<0)
                return -EFAULT;
        }

        return 0;
    }

    int WaveBuff::Free(void)
    {
        for(int i=0;i<(int)TABLESIZE(sample_buff);i++)
        {
            if(sample_buff[i])
                free(sample_buff[i]);

            sample_buff[i]  = NULL;
            sample_blen[i]  = 0;
            sample_count[i] = 0;
        }
        return 0;
    }

    int WaveBuff::Save(void)
    {
        int sample ;

        if(!sample_buff[0] && !sample_buff[1])
            return 0;

        if(sample_buff[0] && !sample_buff[1])
            sample = sample_count[0];
        if(sample_buff[1] && !sample_buff[0])
            sample = sample_count[1];
        if(sample_buff[1] &&  sample_buff[0])
            sample = std::min(sample_count[0],sample_count[1]);

        Seek(0);

        if(Write(sample_buff[0],sample_buff[1],sample)>0)
        {
            Update();
        }

        return wav_hdr.data_payload;
    }

    int  WaveBuff::Count(int channel)
    {
        assert(channel==0 || channel==1);

        return sample_count[channel];
    }

    void *WaveBuff::ToSample(int pos,int channel,int sbw)
    {
        char * sample;

        if(channel<0 || channel>(int)TABLESIZE(sample_buff))
            return NULL;

        if(sample_buff[channel]==NULL)
            return NULL;
        if(pos<0 || pos>=sample_count[channel])
            return NULL;

        sample = (char*)sample_buff[channel];
        return (void*)(sample+(pos*sbw/8));
    }

    int WaveBuff::GetSample(int pos,void*sb,int sblen,int channel,int sbw)
    {
        void *start;
        void *ending;

        start = ToSample(pos,channel,sbw);
        if(start==NULL)
            return -EINVAL;

        ending = ToSample(pos+sblen,channel,sbw);
        if(ending==NULL)
            return -EINVAL;

        memcpy(sb,start,sblen*(sbw/8));

        return sblen;
    }

    int WaveBuff::PutSample(int pos,const void*sb,int sblen,int channel,int sbw)
    {
        void *start;
        void *ending;

        start = ToSample(pos,channel,sbw);
        if(start==NULL)
            return -EINVAL;

        ending = ToSample(pos+sblen,channel,sbw);
        if(ending==NULL)
            return -EINVAL;

        memcpy(start,sb,sblen*(sbw/8));

        return sblen;
    }

    /************************************************************************/
    /* class of RingBuff                                                    */
    /************************************************************************/
    RingBuff::RingBuff(size_t count,size_t size)
    {
        assert(count!=0);
        assert(size!=0);

        elem_read =0;
        elem_write=0;
        elem_count=count;
        elem_size =size;
        elem_data =new char[elem_count*elem_size];

        Reset();
    }
    RingBuff::~RingBuff()
    {
        if(elem_data)
            delete[] elem_data;
    }

    void   RingBuff::Reset()
    {   
        elem_wrap = SAME_WRAP;
        elem_read = 0;
        elem_write= 0;
        if(elem_data)
            memset(elem_data,0,elem_count*elem_size);
    }

    size_t RingBuff::Read(void *buff,size_t size)
    {
        const size_t avail_read    = ReadAvailable();
        const size_t avail_write   = elem_count - avail_read;
        const size_t read_elements = (avail_read < size ?avail_read : size);
        const size_t read_margin   = elem_count - elem_read;
        void*  buf_ptr_1       = NULL;
        void*  buf_ptr_2       = NULL;
        size_t buf_ptr_bytes_1 = 0;
        size_t buf_ptr_bytes_2 = 0;

        // Check to see if read is not contiguous.
        if (read_elements > read_margin) 
        {
            // Write elem_data in two blocks that wrap the buffer.
            buf_ptr_1       = elem_data + elem_read * elem_size;
            buf_ptr_bytes_1 = read_margin * elem_size;
            buf_ptr_2       = elem_data;
            buf_ptr_bytes_2 = (read_elements - read_margin) * elem_size;
        }
        else
        {
            buf_ptr_1       = elem_data + elem_read * elem_size;
            buf_ptr_bytes_1 = read_elements * elem_size;
            buf_ptr_2       = NULL;
            buf_ptr_bytes_2 = 0;
        }

        if (buf_ptr_bytes_2 > 0)
        {
            // We have a wrap around when reading the buffer. Copy the buffer elem_data to
            // |elem_data| and point to it.
            memcpy(buff, buf_ptr_1, buf_ptr_bytes_1);
            memcpy(((char*) buff) + buf_ptr_bytes_1, buf_ptr_2, buf_ptr_bytes_2);
        }
        else
        {
            // No wrap, but a memcpy was requested.
            memcpy(buff, buf_ptr_1, buf_ptr_bytes_1);
        }

        // Update read position
        // We need to be able to take care of negative changes, hence use "int"
        // instead of "size_t".
        int move_read_pos      = (int) elem_read;
        int move_read_count    = (int) read_elements;

        if (move_read_count > (int)avail_read) 
        {
            move_read_count = (int)avail_read;
        }
        if (move_read_count < -((int)avail_write)) 
        {
            move_read_count = -((int)avail_write);
        }

        move_read_pos += move_read_count;
        if (move_read_pos > (int) elem_count)
        {
            // Buffer wrap around. Restart read position and wrap indicator.
            move_read_pos -= (int) elem_count;
            elem_wrap = SAME_WRAP;
        }
        if (move_read_pos < 0) 
        {
            // Buffer wrap around. Restart read position and wrap indicator.
            move_read_pos += (int) elem_count;
            elem_wrap = DIFF_WRAP;
        }

        elem_read = (size_t) move_read_pos;

        return move_read_count;
    }

    size_t RingBuff::Write(void *buff,size_t size)
    {
        const size_t free_elements  = WriteAvailable();
        const size_t write_elements = (free_elements < size ? free_elements: size);

        size_t n      = write_elements;
        size_t margin = elem_count - elem_write;

        if (write_elements > margin) 
        {
            // Buffer wrap around when writing.
            memcpy(elem_data + elem_write * elem_size,
                buff, margin * elem_size);

            elem_write = 0;
            n -= margin;
            elem_wrap = DIFF_WRAP;
        }
        memcpy(elem_data + elem_write * elem_size,
            ((const char*) buff) + ((write_elements - n) * elem_size),
            n * elem_size);

        elem_write += n;

        return write_elements;
    }
    size_t RingBuff::ReadAvailable()
    {
        if (elem_wrap == SAME_WRAP) 
            return elem_write - elem_read;
        else
            return elem_count - elem_read + elem_write;
    }
    size_t RingBuff::WriteAvailable()
    {
        return elem_count - ReadAvailable();
    }

    /************************************************************************/
    /* class of WaveDump&WaveDumpList                                       */
    /************************************************************************/
    WaveDump::WaveDump(const void *framebuff,size_t framesize,const char*path,int scduleHz)
    {
        assert(framebuff);
        assert(framesize>0);

        pcmframe    = framebuff;
        pcmframesize= framesize;
        strcpy(pcmPath,path);

        if(!Create(path,1,framesize*scduleHz))
            pcmframe = NULL;

        assert(pcmframe!=NULL);
    }

    WaveDump::WaveDump(const char *path,int samplerate)
    {
        assert(path);
        assert(samplerate>0);

        strcpy(pcmPath,path);
        Create(pcmPath,1,samplerate);

        assert(is_open());
    }

    WaveDump::WaveDump(const char *path)
    {
        assert(path);

        strcpy(pcmPath,path);
        Open(pcmPath);

        assert(is_open());
    }

    WaveDump::~WaveDump()
    {
        /*flush at last*/
        if(is_open())
            Update();
    }

    bool WaveDump::Equal(const char* path)
    {
        return (strcmp(pcmPath,path)==0);
    }

    bool WaveDump::Dump(void)
    {
        if(!pcmframe || pcmframesize<=0)
            return false;

        Write((void*)pcmframe,NULL,pcmframesize);
        return true;
    }

    WaveDumpList::WaveDumpList()
    {

    }

    WaveDumpList::~WaveDumpList()
    {
        Clear();
    }

    bool WaveDumpList::Add(const void *framebuff,size_t framesize,const char*path,int scduleHz)
    {
        WaveDump * dump;

        assert(framebuff!=NULL);
        assert(framesize>=0);
        assert(path != NULL);
        assert(scduleHz>0);

        /*check path duplicated*/
        for (iterator it = begin();it!=end();it++)
        {
            if((*it)->Equal(path))
                return false;
        }

        /*create one*/
        dump = new WaveDump(framebuff,framesize,path,scduleHz);

        /*add to list*/
        push_back(dump);

        return true;
    }

    bool WaveDumpList::Add(const char *path,int samplerate)
    {
        WaveDump * dump;

        assert(path!=NULL);
        assert(samplerate>=0);

        /*check path duplicated*/
        for (iterator it = begin();it!=end();it++)
        {
            if((*it)->Equal(path))
                return false;
        }

        /*create one*/
        dump = new WaveDump(path,samplerate);

        /*add to list*/
        push_back(dump);

        return true;
    }

    bool WaveDumpList::Delete(const char*path)
    {
        if(path)
        {
            for (iterator it = begin();it!=end();it++)
            {
                if((*it)->Equal(path))
                {
                    delete *it;
                    erase(it);
                    return false;
                }
            }
        }
        else
        {
            Clear();
        }
        return false;
    }

    bool WaveDumpList::Clear()
    {
        for (iterator it = begin();it!=end();it++)
        {
            if(*it)
                delete *it;
        }
        clear();
        return true;
    }

    bool WaveDumpList::Dump()
    {
        for (iterator it = begin();it!=end();it++)
        {
            (*it)->Dump();
        }

        return true;
    }

    bool WaveDumpList::Dump(const char *path,int samplerate,void*sample,int sampleNr)
    {
        WaveDump * dump = NULL;

        /*find by path*/
        for (iterator it = begin();it!=end();it++)
        {
            if((*it)->Equal(path))
            {
                dump = *it;
                break;
            }
        }

        /*create one*/
        if(!dump)
        {
            dump = new WaveDump(path,samplerate);
            assert(dump!=NULL);
            push_back(dump);
        }

        /*write data*/
        if(sample && sampleNr>0)
            return (dump->Write(sample,NULL,sampleNr)==sampleNr);
        else
            return false;
    }

    WaveReadList::WaveReadList()
    {

    }

    WaveReadList::~WaveReadList()
    {

    }

    bool WaveReadList::Delete(const char*path)
    {
        if(path)
        {
            for (iterator it = begin();it!=end();it++)
            {
                if((*it)->Equal(path))
                {
                    delete *it;
                    erase(it);
                    return false;
                }
            }
        }
        else
        {
            for (iterator it = begin();it!=end();it++)
            {
                if(*it)
                    delete *it;
            }
            clear();
        }
        return true;
    }

    bool WaveReadList::Read(const char *path,int samplerate,void*sample,int sampleNr)
    {
        WaveDump * dump = NULL;

        /*find by path*/
        for (iterator it = begin();it!=end();it++)
        {
            if((*it)->Equal(path))
            {
                dump = *it;
                break;
            }
        }

        /*create one*/
        if(!dump)
        {
            dump = new WaveDump(path);
            assert(dump!=NULL);
            push_back(dump);
        }

        /*read data*/
        if(sample && sampleNr>0)
            return (dump->Read(sample,NULL,sampleNr)==sampleNr);
        else
            return false;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
}; /*end of host*/


/************************************************************************/
/*                                                                      */
/************************************************************************/

