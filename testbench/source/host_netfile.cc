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

/************************************************************************/
/*Include                                                              */
/************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "host.h"
#include "pcap.h"

/************************************************************************/
/*Define                                                                */
/************************************************************************/
namespace host
{
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
#define NETFILE_IP_TYPE             0x0008      /*network byte order*/
#define NETFILE_UDP_TYPE            0x11

#define NETFILE_VERSION_MAST        0xF0
#define NETFILE_RFC1889_VERSION     2


    typedef struct
    {
#define ETH_HEAD_LENGTH     14

        unsigned char  src_mac_addr[6];
        unsigned char  dst_mac_addr[6];
        unsigned short protocol_type;
    }ETHHead;

    typedef struct
    {
        unsigned int addr;
    }INAddress;

    /* IPv4 header */
    typedef struct
    {
#define IP_HEAD_LENGTH      20

        unsigned char      ver_ihl;        /* Version (4 bits) + Internet header length (4 bits) */
        unsigned char      tos;            /* Type of service */
        unsigned short     tlen;           /* Total length */
        unsigned short     identification; /* Identification */
        unsigned short     flags_fo;       /* Flags (3 bits) + Fragment offset (13 bits) */
        unsigned char      ttl;            /* Time to live */
        unsigned char      proto;          /* Protocol */
        unsigned short     crc;            /* Header checksum */
        INAddress          saddr;          /* Source address */
        INAddress          daddr;          /* Destination address */
    }IPHead;

    /* UDP header*/
    typedef struct
    {
#define UDP_HEAD_LENGTH     8

        unsigned short     sport;          /* Source port */
        unsigned short     dport;          /* Destination port */
        unsigned short     length;         /* Datagram length */
        unsigned short     crc;            /* Checksum */
    }UDPHead;/* rtp head*/

    typedef struct
    {
#define RTCP_HEAD_LENGTH    4

        unsigned char       version;        /* version */
        unsigned char       packet_type;    /* packet_type */
        unsigned short      packet_wlen;    /* sequence number */
    }RTCPHead;

    typedef struct
    {
#define RTP_HEAD_LENGTH     12

        unsigned char       version;        /* version */
        unsigned char       payload_type;   /* mark+payload_type */
        unsigned short      seq_num;        /* sequence number */
        unsigned int        timestamp;      /* timestamp */
        unsigned int        ssrc;           /* ssrc */
    }RTPHead;

    /************************************************************************/
    /*Declare                                                              */
    /************************************************************************/
    const int     netfile_rtcp_types[] =
    {
        200,        /*Send Report*/
        201,        /*Receive Report*/
        202,        /*SDES*/
        203,        /*BYE*/
        204,        /*APP*/
        207,        /*Extended Reports*/
    };

    const IPHead  netfile_dump_ip_head=
    {
        0x45,        /*     unsigned char      ver_ihl;         Version (4 bits) + Internet header length (4 bits)*/
        0x00,        /*     unsigned char      tos;             Type of service  */
        0x0000,      /*     unsigned short     tlen;            Total length */
        0x0000,      /*     unsigned short     identification;  Identification */
        0x0000,      /*     unsigned short     flags_fo;        Flags (3 bits) + Fragment offset (13 bits)*/
        0xff,        /*     unsigned char      ttl;             Time to live */
        0x11,        /*     unsigned char      proto;           Protocol udp*/
        0x00,        /*     unsigned short     crc;             Header checksum */
        {0x00000000},/*     struct in_addr     saddr;           Source address*/
        {0x00000000},/*     struct in_addr     daddr;           Destination address */
    };

    /************************************************************************/
    /*Debug                                                                */
    /************************************************************************/
    static const char  *helper_IPAddress2String(unsigned int IPaddr)
    {
        static char   IPtmp[64];
        const u_char *IPsec = (const u_char *)&IPaddr;

        sprintf(IPtmp, "%u.%u.%u.%u", IPsec[0], IPsec[1], IPsec[2], IPsec[3]);

        return IPtmp;
    }
    static unsigned int helper_String2IPAddress(const char *str)
    {
        unsigned int  IPaddr  = 0;
        unsigned int  IPsec[4]={0};
        unsigned char*IPstr   = (unsigned char*)&IPaddr;

        if(sscanf(str,"%d.%d.%d.%d",IPsec+0,IPsec+1,IPsec+2,IPsec+3)==4)
        {
            IPstr[0] = static_cast<unsigned char>(IPsec[0]);
            IPstr[1] = static_cast<unsigned char>(IPsec[1]);
            IPstr[2] = static_cast<unsigned char>(IPsec[2]);
            IPstr[3] = static_cast<unsigned char>(IPsec[3]);
        }

        return IPaddr;
    }

    static void print_eth_header(const ETHHead* eth_head)
    {
        printf("eth header length: %d\n", sizeof(ETHHead));

        printf("mac %x:%x:%x:%x:%x:%x -> mac %x:%x:%x:%x:%x:%x\n",
            eth_head->src_mac_addr[0],eth_head->src_mac_addr[1],
            eth_head->src_mac_addr[2],eth_head->src_mac_addr[3],
            eth_head->src_mac_addr[4],eth_head->src_mac_addr[5],

            eth_head->dst_mac_addr[0],eth_head->dst_mac_addr[1],
            eth_head->dst_mac_addr[2],eth_head->dst_mac_addr[3],
            eth_head->dst_mac_addr[4],eth_head->dst_mac_addr[5]);

        printf("protocol: %d\n", eth_head->protocol_type);

        return;
    }

    static void print_ip_header(const IPHead* ip_head)
    {
        printf("ip h-length : %d\n",   sizeof(IPHead));
        printf("Ver+hlen    : 0x%x\n", ip_head->ver_ihl);
        printf("serv type   : 0x%x\n", ip_head->tos);
        printf("pkt len     : %d\n",   OSSock::Ntohs(ip_head->tlen));
        printf("id          : 0x%x\n", ip_head->identification);
        printf("flagment    : 0x%x\n", ip_head->flags_fo);
        printf("protocol    : 0x%x\n", ip_head->proto);
        printf("crc         : 0x%x\n", OSSock::Ntohs(ip_head->crc));
        printf("address     :%s -> %s\n", 
            helper_IPAddress2String(ip_head->saddr.addr),
            helper_IPAddress2String(ip_head->daddr.addr));

        return;
    }

    static void print_udp_header(const UDPHead* udp_head)
    {
        printf("udp len  : %d\n",       OSSock::Ntohs(udp_head->length));
        printf("udp port : %d -> %d\n", OSSock::Ntohs(udp_head->sport), OSSock::Ntohs(udp_head->dport));
        printf("udp crc  : 0x%x\n",     OSSock::Ntohs(udp_head->crc));

        return;
    }

    static void print_packet(const struct pcap_pkthdr *header, const unsigned char *pkt_data)
    {
        const ETHHead   *eth_head;
        const IPHead    *ip_head;
        const UDPHead   *udp_head;

        /* print pkt timestamp and pkt len */
        printf("time %ld:%ld (%d)\n", header->ts.tv_sec, header->ts.tv_usec, header->len);
        printf("caplen: %d\n",        header->caplen);
        printf("pktlen: %d\n",        header->len);

        /* Print the packet */
        eth_head= (const ETHHead*) pkt_data;
        /* retireve the position of the ip header */
        ip_head = (const IPHead*) (pkt_data + sizeof(ETHHead)); //length of ethernet header

        /* retireve the position of the udp header */
        udp_head= (const UDPHead*) ((char*)ip_head + sizeof(IPHead));

        print_eth_header(eth_head);

        print_ip_header(ip_head);

        print_udp_header(udp_head);

        printf("\n\n");
    };

    static void print_rtp(const NetPacket *packet)
    {
        assert(packet->type == NetFile::PT_RTP);

        printf("%08d.%08d:SDP(%02x),ptype=0x%02x,SEQ=%d,TS=%d,SSRC=0x%08x\n",
            (int)packet->sec,
            (int)packet->usec,
            (int)packet->rtp.SDP,
            (int)packet->rtp.ptype,
            (int)OSSock::Ntohs(packet->rtp.sequence),
            (int)OSSock::Ntohl(packet->rtp.timestamp),
            (int)OSSock::Ntohl(packet->rtp.ssrc)
            );

        return;
    }

    static void print_rtcp(const NetPacket *packet)
    {
        int             packet_len;
        const char*     ptr;
        RTCPHead*       rtcp_head;

        assert(packet->type == NetFile::PT_RTCP);

        printf("%08d.%08d:len=%d",
            (int)packet->sec,
            (int)packet->usec,
            packet->length);

        packet_len = packet->length;
        ptr        = packet->rtcp.payload;

        int len = 0;
        int i   = 0;
        for(; len < packet_len; len += (OSSock::Ntohs(rtcp_head->packet_wlen) * 4 + sizeof(RTCPHead)))
        {
            i++;
            rtcp_head = (RTCPHead*) (ptr + len);
            printf(",rtcp(%d)=%d", i, rtcp_head->packet_type);
        }
        printf("\n");

        return;
    }

    static void print_rfc2833(const NetPacket *packet)
    {
        assert(packet->type == NetFile::PT_RFC2833);

        printf("%08d.%08d:SDP(%02x),ptype=0x%02x,SEQ=%d,TS=%d,SSRC=0x%08x,ID=%d,VOL=%d,Dur=%d\n",
            (int)packet->sec,
            (int)packet->usec,
            (int)packet->rfc2833.SDP,
            (int)packet->rfc2833.ptype,
            (int)OSSock::Ntohs(packet->rfc2833.sequence),
            (int)OSSock::Ntohl(packet->rfc2833.timestamp),
            (int)OSSock::Ntohl(packet->rfc2833.ssrc),
            (int)packet->rfc2833.event,
            (int)packet->rfc2833.volume,
            (int)OSSock::Ntohs(packet->rfc2833.duration)
            );
    }

    /************************************************************************/
    /*Public                                                               */
    /************************************************************************/
    NetFile::NetFile(bool loop,bool hostbyteOrder)
    {
        m_debug         = false;
        m_hostbyteOrder = hostbyteOrder;
        m_rfc2833_ptype = 101;

        m_pcap          = NULL;
        m_pcap_dead     = NULL;
        m_pcap_dump     = NULL;
        m_skew_time     = 0;
        m_drift_time    = 0;

        m_save_pf      = 0;
        m_save_path[0] = 0;
        m_save_srcIP[0]= 0;
        m_save_dstIP[0]= 0;

        m_read_round    = 0;
        m_read_originMS = 0;
        m_read_elapseMS = 0;

        assert(sizeof(ETHHead) == ETH_HEAD_LENGTH);
        assert(sizeof(IPHead)  == IP_HEAD_LENGTH);
        assert(sizeof(UDPHead) == UDP_HEAD_LENGTH);
    };

    NetFile::~NetFile()
    {
        Close();
    }

    /*open & create*/
    bool NetFile::Open(const char* path,int pf,const char*srcIP,const char *dstIP)
    {
        char    errbuf[PCAP_ERRBUF_SIZE];
        char    filter[256];
        char    temp[512];
        char   *pstr;

        assert(path != NULL);
        assert(pf != 0);

        strcpy(filter,"ip and udp");
        if(srcIP && strlen(srcIP)>0)
        {
            strcpy(temp,srcIP);
            pstr = strchr(temp,':');
            if(pstr)
            {
                *pstr = '\0';
                strcat(filter," and src host ");
                strcat(filter,temp);
                strcat(filter," and src port ");
                strcat(filter,pstr+1);
            }
            else
            {
                strcat(filter," and src host ");
                strcat(filter,srcIP);
            }
        }
        if(dstIP && strlen(dstIP)>0)
        {
            strcpy(temp,dstIP);
            pstr = strchr(temp,':');
            if(pstr)
            {
                *pstr = '\0';
                strcat(filter," and dst host ");
                strcat(filter,temp);
                strcat(filter," and dst port ");
                strcat(filter,pstr+1);
            }
            else
            {
                strcat(filter," and dst host ");
                strcat(filter,dstIP);
            }
        }

        /*safely close*/
        Close();

        /* Open the capture file */
        m_pcap = (void*)pcap_open_offline(path, errbuf);
        if(m_pcap == NULL)
        {
            return false;
        }
        
        if(!SetFilter(filter))
        {
            Close();
            return false;
        }

        SetDrop(0);
        SetSkew(0);

        /*
        * save info
        */
        m_save_pf = pf;
        if(path && m_save_path!=path)
            strcpy(m_save_path, path);
        if(srcIP && m_save_srcIP!=srcIP)
            strcpy(m_save_srcIP,srcIP);
        if(dstIP && m_save_dstIP!=dstIP)
            strcpy(m_save_dstIP,dstIP);

        return true;
    }

    bool NetFile::Create(const char* path)
    {
        assert(path != NULL);

        /*safely close*/
        Close();

        /* Open the capture file */
        m_pcap_dead = pcap_open_dead(DLT_EN10MB, 65535);
        assert(m_pcap_dead != NULL);

        m_pcap_dump = pcap_dump_open((pcap_t*)m_pcap_dead, path);
        assert(m_pcap_dump != NULL);

        return true;
    }

    bool NetFile::Close()
    {
        if(m_pcap)
        {
            pcap_close((pcap_t*)m_pcap);
            m_pcap = NULL;
        }

        if(m_pcap_dead)
        {
            pcap_close((pcap_t*) m_pcap_dead);
            m_pcap_dead = NULL;
            m_pcap_dump = NULL;
        }
        return true;
    }

    /*read&write*/
    bool  NetFile::Read(void * data,int&type,int&length,long&sec,long&usec,bool&drop)
    {
        struct pcap_pkthdr  *raw_head;
        const unsigned char *raw_data;
        IPHead              *ip_head;
        int                  ip_length;
        UDPHead             *udp_head;
        const unsigned char *udp_data;
        int                  udp_len;

        while(m_pcap)
        {
            if(pcap_next_ex((pcap_t*)m_pcap, &raw_head, &raw_data) <= 0)
            {
                return false;
            }

            if(m_debug)
                print_packet(raw_head,raw_data);

            /* retrieve the position of the ip header */
            ip_head    = (IPHead*) (raw_data + 14);

            /* retrieve the position of the udp header */
            ip_length  = (ip_head->ver_ihl & 0xf) * 4;
            udp_head   = (UDPHead*) ((unsigned char*)ip_head + ip_length);
            udp_data   = (const unsigned char*)udp_head + sizeof(UDPHead);
            udp_len    = OSSock::Ntohs(udp_head->length) - sizeof(UDPHead);

            /*is rtp packet */
            if(!IsVoIPPacket(udp_data, udp_len))
                continue;
            else
            {
                bool isRTP = IsRTPPacket(udp_data, udp_len);
                bool isRTCP= isRTP && IsRTCPPacket(udp_data, udp_len);
                bool is2833= isRTP && IsRFC2833(udp_data, udp_len);

                if(isRTCP || is2833)
                {
                    isRTP = false;
                }

                isRTP  = (isRTP &&(m_save_pf&PF_RTP));
                is2833 = (is2833&&(m_save_pf&PF_RFC2833));
                isRTCP = (isRTCP&&(m_save_pf&PF_RTCP));

                if(isRTP || is2833 || isRTCP) 
                {
                    /*
                    * check drop
                    */
                    drop = m_drop.Judge();

                    if(isRTCP)
                        type = PT_RTCP;
                    else if(is2833)
                        type = PT_RFC2833;
                    else
                        type = PT_RTP;

                    length   = udp_len;
                    sec      = raw_head->ts.tv_sec;
                    usec     = raw_head->ts.tv_usec;
                    memcpy(data, udp_data, udp_len);

                    /*
                    * check skew!
                    */
                    if(!drop 
                        && m_skew_time !=0 
                        && m_skew.Judge())
                    {
                        m_drift_time+=m_skew_time;
                        SkewPacket(sec,usec,m_drift_time);
                    }

                    return true;
                }
            }
        }

        return false;
    }

    bool NetFile::Write(void * data,int type,int length,long sec,long usec,const char*srcIPaddr,const char *dstIPaddr)
    {
        ETHHead            *eth_head;
        IPHead             *ip_head;
        UDPHead            *udp_head;
        unsigned char       pkt_data[2048];
        unsigned char      *ptr;
        const    char      *chr;
        struct pcap_pkthdr  header;
        unsigned char       srcMAC[] = {0x00,0x15,0x65,0x11,0x11,0x11};
        unsigned char       dstMAC[] = {0x00,0x15,0x65,0x22,0x22,0x22};
        unsigned int        srcIP;
        unsigned int        dstIP;
        unsigned short      srcPort;
        unsigned short      dstPort;

        if(!srcIPaddr)
        {
            srcIP   = helper_String2IPAddress("192.168.0.1");
            srcPort = 1000+((unsigned)this)%10;/*using ${this} as port number */
        }
        else
        {
            srcIP   = helper_String2IPAddress(srcIPaddr);
            chr     = strchr(srcIPaddr,':');
            if(chr)
                srcPort = atoi(chr+1);
            else
                srcPort = 1000+((unsigned)this)%10;
        }

        if(!dstIPaddr)
        {
            dstIP   = helper_String2IPAddress("192.168.0.2");
            dstPort = 1000+((unsigned)this)%10;/*using ${this} as port number */
        }
        else
        {
            dstIP   = helper_String2IPAddress(dstIPaddr);
            chr     = strchr(dstIPaddr,':');
            if(chr)
                dstPort = atoi(chr+1);
            else
                dstPort = 1000+((unsigned)this)%10;
        }

        /* retrieve the eth header */
        eth_head    =  (ETHHead*) pkt_data;

        /* retrieve the position of the ip header */
        ip_head     = (IPHead*) (pkt_data + sizeof(ETHHead)); //length of ethernet header

        /* retrieve the position of the udp header */
        udp_head    = (UDPHead*) ((unsigned char*)ip_head + sizeof(IPHead));

        /*fill rtp rtcp data*/
        ptr        = (unsigned char*)udp_head + sizeof(UDPHead);
        memcpy(ptr, data, length);

        /*fill udp head*/
        udp_head->sport = OSSock::Htons(srcPort); 
        udp_head->dport = OSSock::Htons(dstPort); 
        udp_head->length= OSSock::Htons(length + sizeof(UDPHead));
        udp_head->crc   = 0x0000;

        /*fill ip head*/
        memcpy(ip_head, &netfile_dump_ip_head, sizeof(IPHead));
        ip_head->saddr.addr   = srcIP;
        ip_head->daddr.addr   = dstIP;
        ip_head->tlen         = OSSock::Htons(length + sizeof(UDPHead) + sizeof(IPHead));
        ip_head->crc          = OSSock::Htons(CheckSum((const unsigned char*)ip_head, sizeof(IPHead)));

        memcpy(eth_head->src_mac_addr,srcMAC,6);
        memcpy(eth_head->dst_mac_addr,dstMAC,6);
        eth_head->protocol_type = NETFILE_IP_TYPE;

        /*fill pcap_pkthdr*/
        header.ts.tv_sec  = sec;
        header.ts.tv_usec = usec;
        header.caplen     = length + sizeof(UDPHead) + sizeof(IPHead) + sizeof(ETHHead);
        header.len        = header.caplen;

        /*write to pcap file*/
        pcap_dump((unsigned char*) m_pcap_dump, &header, (unsigned char*)pkt_data);

        return true;
    }

    bool NetFile::Read (NetPacket &packet)
    {
        /*read out*/
        if(!Read(&packet.rtp,packet.type,packet.length,packet.sec,packet.usec,packet.drop))
            return false;

        /*handle byte order*/
        if(m_hostbyteOrder)
        {
            if(packet.type==PT_RTP || packet.type==PT_RFC2833)
            {
                packet.rtp.sequence  = OSSock::Ntohs(packet.rtp.sequence);
                packet.rtp.timestamp = OSSock::Ntohs(packet.rtp.timestamp);
                packet.rtp.ssrc      = OSSock::Ntohs(packet.rtp.ssrc);
            }
            if(packet.type==PT_RFC2833)
            {
                packet.rfc2833.duration = OSSock::Ntohs(packet.rfc2833.duration);
            }
        }
        return true;
    }

    bool NetFile::Write(NetPacket &packet,const char*srcIP,const char *dstIP)
    {
        /*just write*/
        return Write(&packet.rtp,packet.type,packet.length,packet.sec,packet.usec,srcIP,dstIP);
    }

    /*seek*/
    bool NetFile::Seek(int pos)
    {
        if(m_pcap!=NULL && pos==0)
        {
            /*we just can seek back to begin by reopen!!*/
            return Open(m_save_path,m_save_pf,m_save_srcIP,m_save_dstIP);
        }
        return false;
    }

    /*debug*/
    bool NetFile::Dump(NetPacket &packet)
    {
        switch(packet.type)
        {
        case PT_RTP:
            print_rtp(&packet);
            return true;
        case PT_RTCP:
            print_rtcp(&packet);
            return true;
        case PT_RFC2833:
            print_rfc2833(&packet);
            return true;
        }
        return false;
    }

    /*config*/
    bool NetFile::SetDrop(int persent)
    {
        m_drop.Reset(persent);
        return true;
    }
    bool NetFile::SetSkew(int persent,int delayUs)
    {
        m_skew.Reset(persent);

        m_skew_time = delayUs;
        m_drift_time= 0;

        return true;
    }

    bool NetFile::DropPacket(NetPacket &packet)
    {
        packet.drop = true;
        return true;
    }

    bool NetFile::SkewPacket(long &packet_sec,long &packet_usec,int skewus)
    {
        if(skewus>0)
        {
            packet_usec += skewus;
            if(packet_usec>=1000*1000)
            {
                packet_sec += packet_usec/(1000*1000);
                packet_usec%= (1000*1000);
            }
        }
        else
        {
            int sec =  std::abs(skewus)/(1000*1000);
            int usec=  std::abs(skewus)%(1000*1000);

            packet_sec -= sec;
            if(packet_usec>=usec)
                packet_usec -= usec;
            else
            {
                packet_sec -= 1;
                packet_usec = (1000*1000-usec);
            }
        }
        return true;
    }

    /************************************************************************/
    /*Private                                                       */
    /************************************************************************/

    bool NetFile::IsVoIPPacket(const unsigned char* packet, size_t packet_len)
    {
        assert(packet != NULL);

        if(packet_len <= sizeof(RTCPHead))
        {
            return false;
        }

        /*check pakcket version*/
        if((packet[0] & NETFILE_VERSION_MAST) == (NETFILE_RFC1889_VERSION << 6))
        {
            return true;
        }

        return false;

    }

    bool NetFile::IsPTCPPacket(int rtcp_type)
    {
        unsigned int i;

        /*check rtcp packet type*/
        for(i = 0; i < sizeof(netfile_rtcp_types); i++)
        {
            if(rtcp_type == netfile_rtcp_types[i])
            {
                return true;
            }
        }

        return false;
    }

    bool NetFile::IsRTCPPacket(const unsigned char* packet, size_t packet_len)
    {
        RTCPHead   *rtcp_head;
        size_t      len;

        assert(packet!=NULL);

        rtcp_head = (RTCPHead*) packet;
        for(len = 0; len < packet_len; len += (OSSock::Ntohs(rtcp_head->packet_wlen) * 4 + sizeof(RTCPHead)))
        {
            rtcp_head = (RTCPHead*) (packet + len);

            if(IsPTCPPacket(rtcp_head->packet_type) == false)
            {
                /*unknow rtcp packet type*/
                return false;
            }
        }

        if(len == packet_len)
        {
            /*error packet length*/
            return true;
        }

        return false;
    }

    bool NetFile::IsRTPPacket(const unsigned char* packet, size_t packet_len)
    {
        assert(packet!=NULL);

        if( (packet_len <= (int)sizeof(RTPHead)) || packet[0]  != (NETFILE_RFC1889_VERSION << 6))
        {
            return false;
        }

        return true;
    }

    bool NetFile::IsRFC2833(const unsigned char* packet, size_t packet_len)
    {
        NetPacketRTP * rtp = (NetPacketRTP*)packet;

        return ((rtp->ptype&0x7F) == (unsigned char)m_rfc2833_ptype);
    }

    bool NetFile::SetFilter(const char *filter)
    {
        struct bpf_program  fcode;

        /*compile the filter*/
        if (pcap_compile((pcap_t*)m_pcap,&fcode, filter, 1, 0xffffff) < 0 )
        {
            return false;
        }

        /*set the filter*/
        if (pcap_setfilter((pcap_t*)m_pcap,&fcode)<0)
        {
            return false;
        }

        pcap_freecode(&fcode);

        return true;
    }

    int  NetFile::CheckSum(const unsigned char* data, size_t data_len)
    {
        size_t cksum = 0;
        size_t index = 0;

        if(data_len % 2 != 0)
        {
            return 0;
        }

        while(index < data_len)
        {
            if(index != 10)
            {
                cksum += *(data + index + 1);
                cksum += *(data + index) << 8;
            }

            index += 2;
        }

        while(cksum > 0xffff)
        {
            cksum = (cksum >> 16) + (cksum & 0xffff);
        }
        return ~cksum;
    }

    bool NetFile::TimeReset()
    {
        m_read_round    = 0;
        m_read_originMS = 0;
        m_read_elapseMS = 0;
        return true;
    }
    bool NetFile::TimeElaps(int sec,int ussec)
    {
        m_read_elapseMS += sec*1000*1000+ussec/1000;

        return true;
    }
    bool NetFile::TimeExpire(int msec)
    {
        if(m_read_elapseMS>msec)
            return true;
        else
            return false;
    }

    int  NetFile::TimeRound()
    {
        return m_read_round;
    }

    int  NetFile::TimeRead(void *&packet,size_t&packet_size,int&packet_type)
    {
        /*if on startup*/
        if(m_read_originMS==0)
        {
            if(!Read(m_packet))
                return -1;
            /*setup origin time from first packet*/
            m_read_originMS = (m_packet.sec*1000) + (m_packet.usec/1000);
        }

        /*calculate time elapsed*/
        long long fake_packet = (m_packet.sec*1000) + (m_packet.usec/1000);
        long long fake_elapse = fake_packet - m_read_originMS;

        if(m_read_elapseMS>=fake_elapse)
        {
            if(m_packet.drop)
                return TimeNext();
            else
            {
                packet      = &m_packet.rtp;
                packet_size = m_packet.length;
                packet_type = m_packet.type;

                return packet_size;
            }
        }
        return 0;
    }

    int  NetFile::TimeWrite(void * packet,size_t packet_size,int packet_type,long sec,long usec)
    {
        switch(packet_type)
        {
        case PT_RTP:
        case PT_CNG:
        case PT_RFC2833:
            {
                if(!Write(packet,packet_type,packet_size,sec,usec,
                    "192.168.0.10:1000","192.168.0.11:1000"))
                    return -1;
                break;
            }
        case PT_RTCP:
            {
                if(!Write(packet,packet_type,packet_size,sec,usec,
                    "192.168.0.10:1001","192.168.0.11:1001"))
                    return -2;
                break;
            }
        case PT_PCM:
        default:
            return -3;
        }

        return packet_size;
    }

    int  NetFile::TimeNext(bool loop)
    {
        /*
        * read next packet
        */
        if(!Read(m_packet))
        {
            m_read_round++;

            if(!loop)
                return 0;

            /*
            * seek back and reset time
            * so TimeRead(...) will read the first packet!
            */
            Seek();

            m_read_elapseMS = 0;
            m_read_originMS = 0;
        }

        return (m_packet.length);
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};
/************************************************************************/
/*                                                                      */
/************************************************************************/

