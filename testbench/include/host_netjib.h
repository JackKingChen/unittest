/* ******************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2013-07-05
*
*
****************************************************************** */

#ifndef __HOST_NETJIB_H__
#define __HOST_NETJIB_H__

/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace host
{
    /*
    * class of NetPacket
    */
    typedef struct
    {
        unsigned char   SDP;            /* version*/
        unsigned char   ptype;          /* mark+payload_type*/
        unsigned short  sequence;       /* sequence number*/
        unsigned int    timestamp;      /* timestamp*/
        unsigned int    ssrc;           /*ssrc*/
        char            payload[1500];
    }NetPacketRTP;

    typedef struct
    {
        unsigned char   SDP;            /* version*/
        unsigned char   ptype;          /* mark+payload_type*/
        unsigned short  sequence;       /* sequence number*/
        unsigned int    timestamp;      /* timestamp*/
        unsigned int    ssrc;           /*ssrc*/
        unsigned char   event;
        unsigned char   volume;
        unsigned short  duration;
    }NetPacketRFC2833;

    typedef struct
    {
        char            payload[1500];
    }NetPacketRTCP;

    typedef struct
    {
        bool           drop;
        int            type;    /*PT_xxx*/
        int            length;  /*data length*/
        long           sec;     /*real time*/
        long           usec;    /*real time*/

        union
        {
            unsigned char    data[64];
            NetPacketRTP     rtp;
            NetPacketRTCP    rtcp;
            NetPacketRFC2833 rfc2833;
        };
    }NetPacket;

    typedef std::list<NetPacket*> NetPacketList;

    /*
    * class of NetProbability
    */
    class NetProbability
    {
    public:
        NetProbability();
        ~NetProbability();

    public:
        bool Ready();
        bool Reset(int per);
        bool Judge();

    protected:
        int     persent;
        int     pasitive;
        int     counter;
        bool    last;
        double  current_ratio;
        double  expect_ratio;
    };

    /*
    * class of jitter simulator
    */
    class JITBuffer
    {
    public:
        JITBuffer(int max_delay_ms,int MTU=1500);
        ~JITBuffer();

    public:
        bool Input (void *pkt_data,int pkt_size);
        bool Output(void *pkt_data,int&pkt_size);
        bool Output(char**pkt_data,int*pkt_size);

        bool Reset(int max_delay_ms=0,int MTU=1500);
        bool Setup(int drop_precent,int skew_precent,int skew_max=50);
        bool Ready();

    protected:
        bool CreateBuffer(size_t max_delay_ms,size_t buffer_MTU);
        bool DestroyBuffer();

    protected:
        size_t          use_MTU;
        size_t          use_packet_max;
        size_t          use_jitter_max;

        NetProbability  drop_probability;
        NetProbability  skew_probability;

        size_t          packet_record;
        NetPacketList   packet_total;
        NetPacketList   packet_free;
        NetPacketList   packet_busy;
    };

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};
/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif  /*__HOST_NETJIB_H__*/
