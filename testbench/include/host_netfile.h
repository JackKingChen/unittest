/* ******************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2013-09-16
*
*
****************************************************************** */


#ifndef __HOST_NETFILE_H__
#define __HOST_NETFILE_H__

/************************************************************************/
/*                                                                      */
/************************************************************************/

namespace host
{
    /*
    * class of NetFile
    */
    class NetFile
    {
    public:
        NetFile(bool loop=true,bool hostbyteOrder=false);
        ~NetFile();

    public:

        enum
        {
            /*
            * ${packet.type} MUST be the same as below!
            * 
            * #define  DSP_DTYPE_INVALID      -1
            * #define  DSP_DTYPE_NONE         0
            * #define  DSP_DTYPE_PCM          1
            * #define  DSP_DTYPE_RTP          2
            * #define  DSP_DTYPE_RTCP         3
            * #define  DSP_DTYPE_RFC2833      4
            * #define  DSP_DTYPE_CNG          5
            * #define  DSP_DTYPE_AVS          6
            * #define  DSP_DTYPE_RED          7
            */
            PT_UNKNOWN = -1,
            PT_NONE    = 0,
            PT_PCM     = 1,
            PT_RTP     = 2,
            PT_RTCP    = 3,
            PT_RFC2833 = 4,
            PT_CNG     = 5,
            PT_AVS     = 6,
            PT_RED     = 7,
        };
        enum
        {
            PF_PCM     = (1<<0),
            PF_RTP     = (1<<1),
            PF_RTCP    = (1<<2),
            PF_RFC2833 = (1<<3),
            PF_CNG     = (1<<4),
        };

    public:
        /*open & create*/
        bool Open  (const char* path,int pf=0xFF,const char*srcIP=NULL,const char *dstIP=NULL);
        bool Create(const char* path);
        bool Close();

        
        /*read&write*/
        bool Read (void * data,int&type,int&length,long&sec,long&usec,bool&drop);
        bool Write(void * data,int type,int length,long sec,long usec,const char*srcIP=NULL,const char *dstIP=NULL);

        bool Read (NetPacket &packet);
        bool Write(NetPacket &packet,const char*srcIP=NULL,const char *dstIP=NULL);
        
        /*seek*/
        bool Seek(int pos=0);

        /*dump*/
        bool Dump(NetPacket &packet);

        /*config*/
        bool SetDrop(int persent);
        bool SetSkew(int persent,int delayUs=1000);

        /*helper*/
        bool IsOpen()
        {return (m_pcap||m_pcap_dead);};

        /*time control*/
        bool TimeReset ();
        bool TimeElaps (int sec,int ussec);
        bool TimeExpire(int msec);
        int  TimeRound ();
        int  TimeRead  (void *&packet,size_t&packet_size,int&packet_type);
        int  TimeWrite (void * packet,size_t packet_size,int packet_type,long sec,long usec);
        int  TimeNext  (bool loop=false);

    private:
        /*decide is rtp or rtcp packet*/
        bool IsVoIPPacket(const unsigned char* packet, size_t packet_len);
        bool IsPTCPPacket(int rtcp_type);
        bool IsRTCPPacket(const unsigned char* packet, size_t packet_len);
        bool IsRTPPacket (const unsigned char* packet, size_t packet_len);
        bool IsRFC2833   (const unsigned char* packet, size_t packet_len);

        /*ip head checksum*/
        bool SetFilter(const char *filter);
        int  CheckSum(const unsigned char* data, size_t data_len);
        bool DropPacket(NetPacket &packet);
        bool SkewPacket(long &packet_sec,long &packet_usec,int usec);

    private:
        bool           m_debug;
        bool           m_hostbyteOrder;

        int            m_rfc2833_ptype;
        void          *m_pcap;
        void          *m_pcap_dead;        /*dump pcap file handle*/
        void          *m_pcap_dump;        /*dump pcap file handle*/

        int            m_save_pf;
        char           m_save_path[256];
        char           m_save_srcIP[256];
        char           m_save_dstIP[256];

        NetProbability m_drop;
        NetProbability m_skew;
        int            m_skew_time;     /*us*/
        int            m_drift_time;    /*us*/

        NetPacket      m_packet;
        size_t         m_read_round;
        long long      m_read_originMS; /*time started*/
        long long      m_read_elapseMS; /*time elapsed since m_read_originMS*/
    };

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};

#endif
