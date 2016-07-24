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
/************************************************************************/
/*Include                                                              */
/************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "host.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace host
{
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    NetProbability::NetProbability()
    {
        Reset(0);
    }

    NetProbability::~NetProbability()
    {
    }

    bool NetProbability::Ready()
    {
        if(persent!=0)
            return true;
        else
            return false;
    }

    bool NetProbability::Reset(int per)
    {
        if(per<0)
            per=0;
        if(per>100)
            per=100;

        persent  = per;
        pasitive = 0;
        counter  = 0;
        last     = false;
        current_ratio = 0.0;
        expect_ratio  = ((double)persent)/((double)100);
        return true;
    }

    bool NetProbability::Judge()
    {
        if(persent<=0)
            return false;
        else if(persent==100)
            return true;

        counter++;

        if(counter<100)
            /*low Probability as low amount*/
            current_ratio = ((double)pasitive)/((double)counter)*0.25;
        else
            /*normal Probability as normal amount*/
            current_ratio = ((double)pasitive)/((double)counter)*1.00;

        /*low Probability as previous is pasitive*/
        if(last==true)
            current_ratio = current_ratio*0.75;

        if(current_ratio < expect_ratio
            && rand()%100 < persent)
        {
            pasitive++;
            last = true;
            return true;
        }
        last = false;
        return false;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    JITBuffer::JITBuffer(int max_delay_ms,int MTU)
    {
        use_MTU         = 0;
        use_packet_max  = 0;
        use_jitter_max  = 0;

        packet_record = 0;
        packet_total.clear();
        packet_free.clear();
        packet_busy.clear();

        if(max_delay_ms>0)
            CreateBuffer(max_delay_ms,MTU);
    }

    JITBuffer::~JITBuffer()
    {
        DestroyBuffer();
    }

    bool JITBuffer::Input(void *pkt_data,int pkt_size)
    {
        assert(pkt_data!=NULL);
        assert(pkt_size<=(int)use_MTU);
        assert(packet_free.size()+packet_busy.size() == packet_total.size());

        packet_record++;

        if(packet_free.size()>0)
        {
            NetPacketList::iterator hdr = packet_free.begin();
            NetPacket       * pkt = static_cast<NetPacket*>(*hdr);

            /*setup info*/
            memcpy(pkt->data,pkt_data,pkt_size);
            pkt->length = pkt_size;
            pkt->sec    = 0;
            pkt->usec   = 0;

            /*if we buffer level is lower ,then just insert*/
            if(packet_busy.size() < (size_t)(((float)use_packet_max)*0.75)
                || packet_record < 100)
            {
                /*insert to back*/
                packet_busy.push_back(pkt);
                /*remove from free*/
                packet_free.pop_front();
            }
            else
            {
                /*
                * make judge if we need to drop it
                */
                if(packet_busy.size()+2 >= use_packet_max)
                {
                    if(drop_probability.Judge())
                    {
                        return true;
                    }
                }

                /*
                * make judge if we need to skew it
                */
                if(skew_probability.Judge())
                {
                    int skew_max = (int)packet_busy.size();
                    int skew_pos = 0;

                    do
                    {
                        skew_pos = ((size_t)rand())%(1+use_jitter_max);
                    }while(skew_pos<=0);

                    /*insert reverse*/
                    skew_pos = skew_max-skew_pos;
                    for(NetPacketList::iterator it=packet_busy.begin();it!=packet_busy.end();it++)
                    {
                        /*must avoid accumulation*/
                        if(--skew_pos == 0)
                        {
                            /*insert to skew_pos*/
                            packet_busy.insert(it,pkt);
                            break;
                        }
                    }

                    /*remove from free*/
                    packet_free.pop_front();
                }
                else
                {
                    /*insert to back*/
                    packet_busy.push_back(pkt);
                    /*remove from free*/
                    packet_free.pop_front();
                }
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    bool JITBuffer::Output(void *pkt_data,int&pkt_size)
    {
        if(packet_busy.size()>=use_packet_max)
        {
            NetPacketList::iterator hdr = packet_busy.begin();
            NetPacket       * pkt = static_cast<NetPacket*>(*hdr);

            memcpy(pkt_data,pkt->data,pkt->length);
            pkt_size = pkt->length;

            /*insert to back*/
            packet_free.push_back(pkt);
            /*remove from busy*/
            packet_busy.pop_front();

            return true;
        }
        else
        {
            return false;
        }
    }

    bool JITBuffer::Output(char**pkt_data,int *pkt_size)
    {
        assert(pkt_data!=NULL);
        assert(pkt_size!=NULL);

        if(packet_busy.size()>=use_packet_max)
        {
            NetPacketList::iterator hdr = packet_busy.begin();
            NetPacket       * pkt = static_cast<NetPacket*>(*hdr);

            *pkt_data = (char*)pkt->data;
            *pkt_size = (int)pkt->length;

            /*insert to back*/
            packet_free.push_back(pkt);
            /*remove from busy*/
            packet_busy.pop_front();

            return true;
        }
        else
        {
            return false;
        }
    }

    bool JITBuffer::Reset(int max_delay_ms,int MTU)
    {
        if(max_delay_ms)
            return CreateBuffer(max_delay_ms,MTU);
        else
        {
            packet_free.clear();
            packet_busy.clear();

            for(NetPacketList::iterator it=packet_total.begin();it!=packet_total.end();it++)
            {
                packet_free.push_back(*it);
            }
        }
        return true;
    }

    bool JITBuffer::Setup(int drop_precent,int skew_precent,int skew_max)
    {
        drop_probability.Reset(drop_precent);
        skew_probability.Reset(skew_precent);

        packet_record = 0;
        use_jitter_max = (packet_total.size()*skew_max)/100;
        if(use_jitter_max<1)
            use_jitter_max = 1;
        return true;
    }

    bool JITBuffer::Ready()
    {
        if(packet_total.size()>0
            && (drop_probability.Ready() || skew_probability.Ready()))
            return true;
        else
            return false;
    }

    bool JITBuffer::CreateBuffer(size_t max_delay_ms,size_t buffer_MTU)
    {
        assert(max_delay_ms>0);
        assert(buffer_MTU>0);

        /*free all*/
        DestroyBuffer();

        /*save*/
        use_MTU         = buffer_MTU;
        use_packet_max  = max_delay_ms/10+10;
        use_jitter_max  = use_packet_max/2;

        /*alloc buffer*/
        for (size_t i=0;i<use_packet_max;i++)
        {
            NetPacket * pkt = new NetPacket;

            assert(pkt!=NULL);

            packet_total.push_back(pkt);
            packet_free.push_back(pkt);
        }
        packet_busy.clear();

        packet_record = 0;

        return true;
    }

    bool JITBuffer::DestroyBuffer()
    {
        /*free all*/
        for(NetPacketList::iterator it=packet_total.begin();it!=packet_total.end();it++)
        {
            NetPacket * pkt = static_cast<NetPacket*>(*it);

            delete pkt;
        }

        packet_total.clear();
        packet_free.clear();
        packet_busy.clear();
        return true;
    }
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};
/************************************************************************/
/*                                                                      */
/************************************************************************/
