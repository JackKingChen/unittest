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
    OSAudioPhone::OSAudioPhone(const char* strIP,int ptime,int freq,const char *remoteDeviceIP)
    {
        const char* srcIP;

        assert(freq==16000 || freq==32000);
        assert(ptime%10 == 0);
        assert(ptime>=MIN_PTIME && ptime<=MAX_PTIME);

        data_frame     = (RTAudioFrame*)malloc(1500);
        data_framesize = freq/100;
        rx_buffer_wr   = 0;
        rx_buffer_rd   = 0;
        rx_buffer_ptime= (ptime/10)*data_framesize;
        tx_buffer_ts   = 0;
        tx_buffer_wr   = 0;
        tx_buffer_ptime= (ptime/10)*data_framesize;

        OSAudioManager::SetFreq(freq);
        OSAudioManager::SetChan(1);
        OSAudioManager::InitDevice(remoteDeviceIP,false);

        /*init socket*/
        if(strcmp(strIP,"127.0.0.1")==0)
            srcIP = strIP;
        else
            srcIP = NULL;

        if(!data_sock.Create(OSSock::ST_UDP,srcIP,PHONE_RE_DATA_PORT))
            printf("OSAudioPhone:failed to create data socket!\n");
        data_sock.Connect(strIP,PHONE_LO_DATA_PORT);
        data_sock.SetBlock(false);
    }

    OSAudioPhone::~OSAudioPhone()
    {
        OSAudioManager::Stop();
        free(data_frame);
    }

    bool OSAudioPhone::Start()
    {
        rx_buffer_wr = 0;
        rx_buffer_rd = 0;
        tx_buffer_wr = 0;
        rx_downflow = 0;
        rx_overflow = 0;
        tx_missing  = 0;
        return OSAudioManager::Start();
    }

    bool OSAudioPhone::Stop()
    {

        return OSAudioManager::Stop();
    }

    int OSAudioPhone::OnFrames(OSAudioFrame &capture_frames,OSAudioFrame &playout_frames)
    {
        int total;

        /*
        * handle TX
        */
        if(MAX_BUFFER-tx_buffer_wr >= data_framesize)
        {
            memcpy(tx_buffer+tx_buffer_wr,capture_frames[0],data_framesize*sizeof(short));
            tx_buffer_wr+=data_framesize;
        }
        else
        {
            tx_missing++;
            printf("OSAudioPhone::TX missing,%d\n",tx_missing);
        }

        if(tx_buffer_wr==tx_buffer_ptime)
        {
            total = tx_buffer_ptime*sizeof(short);

            data_frame->version  = 0x80;
            data_frame->ptype    = 0;
            data_frame->seqNumber= OSSock::Htons(data_sequence);
            data_frame->timeStamp= OSSock::Htonl(tx_buffer_ts);
            data_frame->ssrc     = OSSock::Htonl((unsigned int)data_frame);
            memcpy(data_frame+1,tx_buffer,total);
            
            /*send out*/
            total += sizeof(RTAudioFrame);
            if(data_sock.Send(data_frame,total)!=total)
            {
                printf("OSAudioPhone::TX failed,%d\n",tx_missing);
            }

            /*next packet*/
            tx_buffer_ts+=tx_buffer_ptime;
            data_sequence++;

            /*seek back*/
            tx_buffer_wr = 0;
        }

        /*
        * handle RX
        */

        /*play one frame*/
        if(rx_buffer_rd!=rx_buffer_wr)
        {
            if(std::abs((int)rx_buffer_wr - (int)rx_buffer_rd) > (int)data_framesize)
            {
                memcpy(playout_frames[0],rx_buffer+rx_buffer_rd,data_framesize*sizeof(short));

                rx_buffer_rd += data_framesize;
                if(rx_buffer_rd >= MAX_BUFFER)
                    rx_buffer_rd = 0;
            }
            else
            {
                memset(playout_frames[0],0,data_framesize*sizeof(short));
                rx_buffer_rd=rx_buffer_wr;
            }
        }
        else
        {
            memset(playout_frames[0],0,data_framesize*sizeof(short));
        }

        /*get data from remote*/
        total = data_sock.Recv(data_frame,MAX_BUFFER);
        if(total>0)
        {
            total-=sizeof(RTAudioFrame);
  
            if(((total/sizeof(short)) % data_framesize) !=0 )
            {
                printf("OSAudioPhone:invalid timeStamp and size from remote!\n");
                return 0;
            }

            /*just cover the data*/
            total /= sizeof(short);
            if(rx_buffer_wr>=rx_buffer_rd)
                total  = std::min(total,(int)(MAX_BUFFER-rx_buffer_wr));
            else
                total  = std::min(total,(int)(rx_buffer_rd-rx_buffer_wr-data_framesize));

            memcpy(rx_buffer+rx_buffer_wr,data_frame+1,total*sizeof(short));

            if(rx_buffer_wr>=rx_buffer_rd)
            {
                rx_buffer_wr += total;
                if(rx_buffer_wr >= (MAX_BUFFER))
                    rx_buffer_wr = 0;
            }
            else
            {
                rx_buffer_wr += total;

                /*try drop one frame*/
                if(rx_buffer_wr>=rx_buffer_rd)
                    rx_buffer_rd = rx_buffer_wr+data_framesize;
            }
        }

        return 0;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};
