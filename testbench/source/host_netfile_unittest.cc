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

/*for stander*/
#include <stdlib.h>
#include <string.h>

/*for unittest*/
#include "unittest.h"

/*for local*/
#include "host.h"

using namespace host;

/************************************************************************/
/*                                                                      */
/************************************************************************/
UNITTEST(NetFile)
{
    NetFile     pcap;
    NetFile     dump;
    NetPacket   packet;
    int         count;

    /*
    * test all filter
    */
    EXPECT_TRUE(pcap.Open("./runtest.pcap",
        NetFile::PF_RTP|NetFile::PF_RTCP|NetFile::PF_RFC2833,"10.2.2.1"));
    EXPECT_TRUE(dump.Create("./runtest_dump.pcap"));

    count = 0;
    while(pcap.Read(packet))
    {
        if(count<10)
            pcap.Dump(packet);
        dump.Write(packet);
        count++;
        QUIET_EXPECT_TRUE(packet.type == NetFile::PT_RTP || packet.type == NetFile::PT_RTCP || packet.type == NetFile::PT_RFC2833);
    }
    EXPECT_LT(0,count);

    /*
    * test RTP filter
    */
    EXPECT_TRUE(pcap.Open("./runtest.pcap",
        NetFile::PF_RTP,"10.2.2.1:5062"));
    EXPECT_TRUE(dump.Create("./runtest_dump_rtp.pcap"));

    count = 0;
    while(pcap.Read(packet))
    {
        if(count<10)
            pcap.Dump(packet);
        dump.Write(packet);
        count++;
        QUIET_EXPECT_EQ(packet.type , (int)NetFile::PT_RTP);
    }
    EXPECT_LT(0,count);


    /*
    * test RTCP filter
    */
    EXPECT_TRUE(pcap.Open("./runtest.pcap",
        NetFile::PF_RTCP,"10.2.10.4:11821"));
    EXPECT_TRUE(dump.Create("./runtest_dump_rtcp.pcap"));

    count = 0;
    while(pcap.Read(packet))
    {
        if(count<10)
            pcap.Dump(packet);
        dump.Write(packet);
        count++;
        QUIET_EXPECT_EQ(packet.type , (int)NetFile::PT_RTCP);
    }
    EXPECT_LT(0,count);

    /*
    * test DTMF filter
    */
    EXPECT_TRUE(pcap.Open("./runtest.pcap",
        NetFile::PF_RFC2833,"10.2.10.4:11820"));
    EXPECT_TRUE(dump.Create("./result/runtest_dump_dtmf.pcap"));

    count = 0;
    while(pcap.Read(packet))
    {
        if(count<10)
            pcap.Dump(packet);
        dump.Write(packet);
        count++;
        QUIET_EXPECT_EQ(packet.type , (int)NetFile::PT_RFC2833);
    }
    EXPECT_LT(0,count);

    /*
    * test rand drop
    */
    EXPECT_TRUE(pcap.Open("./runtest.pcap",
        NetFile::PF_RTP|NetFile::PF_RFC2833,"10.2.10.4:11820"));
    EXPECT_TRUE(pcap.SetDrop(50));

    int total  = 0;
    int missed = 0;
    int droped = 0;
    int lastseq= 0;
    int currseq= 0;

    while(pcap.Read(packet))
    {
        total++;

        if(packet.drop)
        {
            droped++;
            continue;
        }

        currseq = OSSock::Ntohs(packet.rtp.sequence);
        if(total==1)
            lastseq = currseq;
        else
        {
            if(lastseq+1 != currseq)
                missed++;
        }

        lastseq = currseq;
    }

    EXPECT_LT(0,droped);
    EXPECT_LT(0,missed);
    EXPECT_LT(0,total);

    OSTTY::getkey();
}


UNITTEST(NetFile_drop)
{
    /*
    * test drop
    */
    for (int i=0;i<=5;i++)
    {
        NetFile     pcap;
        NetFile     dump;
        NetPacket   packet;
        char        path[256];

        EXPECT_TRUE(pcap.Open("./runtest.pcap",
            NetFile::PF_RTP|NetFile::PF_RFC2833,"10.2.10.4:11820"));

        sprintf(path,"./runtest_dump_00_drop-%dp100.pcap",i*10);
        EXPECT_TRUE(dump.Create(path));
        pcap.SetDrop(i*10);

        while(pcap.Read(packet))
        {
            if(!packet.drop)
                dump.Write(packet);
        }
    }
}

UNITTEST(NetFile_skew)
{
    /*
    * test skew
    */
    for (int i=0;i<=5;i++)
    {
        NetFile     pcap;
        NetFile     dump;
        NetPacket   packet;
        char        path[256];

        EXPECT_TRUE(pcap.Open("./runtest.pcap",
            NetFile::PF_RTP|NetFile::PF_RFC2833,"10.2.10.4:11820"));

        sprintf(path,"./result/runtest_dump_00_skew+2ms-%dp100.pcap",i*10);
        EXPECT_TRUE(dump.Create(path));
        pcap.SetSkew(i*10,+2000);

        while(pcap.Read(packet))
        {
            if(!packet.drop)
                dump.Write(packet);
        }

        EXPECT_TRUE(pcap.Open("./runtest.pcap",
            NetFile::PF_RTP|NetFile::PF_RFC2833,"10.2.10.4:11820"));

        sprintf(path,"./result/runtest_dump_00_skew-2ms-%dp100.pcap",i*10);
        EXPECT_TRUE(dump.Create(path));
        pcap.SetSkew(i*10,-2000);

        while(pcap.Read(packet))
        {
            if(!packet.drop)
                dump.Write(packet);
        }
    }
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
