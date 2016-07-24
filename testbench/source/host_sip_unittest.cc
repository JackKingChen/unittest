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
class SIPUser:public SIPStack
{
public:
    SIPUser(bool debug):SIPStack(debug)
    {

    };
    ~SIPUser()
    {

    };
public:

    virtual bool OnRegister  (SIPAcct &call)
    {
        printf("SIPUser:OnRegister()...\n");
        return true;
    };
    virtual bool OnUnregister(SIPAcct &call)
    {
        printf("SIPUser:OnUnregister()...\n");
        return true;
    };

    virtual bool OnOutgoingCall(SIPCall &call)
    {
        printf("SIPUser:OnOutgoingCall()...\n");
        return true;
    };
    virtual bool OnIncomingCall(SIPCall &call)
    {
        printf("SIPUser:OnIncomingCall()...\n");
        return true;
    };
    virtual bool OnEstablished (SIPCall &call)
    {
        printf("SIPUser:OnEstablished()...\n");

        printf("This : (%s)%s:%u, %s/%u/%d\n",
            SIPCall::Dir2Name(call.this_audio.dir),
            OSSock::IPv4(call.this_audio.addr_rtp),call.this_audio.port_rtp,
            call.this_audio.codec, call.this_audio.srate, call.this_audio.ptype);

        printf("Peer: (%s)%s:%u, %s/%u/%d\n",
            SIPCall::Dir2Name(call.peer_audio.dir),
            OSSock::IPv4(call.peer_audio.addr_rtp),call.peer_audio.port_rtp,
            call.peer_audio.codec, call.peer_audio.srate, call.peer_audio.ptype);

        return true;
    };
    virtual bool OnFinishedCall(SIPCall &call)
    {
        printf("SIPUser:OnFinishedCall()...\n");
        return true;
    };
    virtual bool OnModifiedCall(SIPCall &call)
    {
        printf("SIPUser:OnModifiedCall()...\n");

        printf("This : (%s)%s:%u, %s/%u/%d\n",
            SIPCall::Dir2Name(call.this_audio.dir),
            OSSock::IPv4(call.this_audio.addr_rtp),call.this_audio.port_rtp,
            call.this_audio.codec, call.this_audio.srate, call.this_audio.ptype);

        printf("Peer: (%s)%s:%u, %s/%u/%d\n",
            SIPCall::Dir2Name(call.peer_audio.dir),
            OSSock::IPv4(call.peer_audio.addr_rtp),call.peer_audio.port_rtp,
            call.peer_audio.codec, call.peer_audio.srate, call.peer_audio.ptype);

        return true;
    };
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
UNITTEST(SIP_outgoing_call)
{
    SIPUser   sip(true);
    SIPCall   call;

    /*create*/
    EXPECT_TRUE(sip.Create());

    /*register*/
    EXPECT_TRUE(sip.Register("10.2.1.199","383889",NULL,
        "PCMU/8000/0;PCMA/8000/8;G722/16000/9"));

    do
    {

        /*
        *--------------------------------------
        * make a call in 20s
        *--------------------------------------
        */
        EXPECT_TRUE(sip.Call(call,"52722",NULL));

        /*after ${SIPStack::WaitAnswer(...)} return with true*/
        if(sip.WaitAnswer(call,20*1000))
        {
            printf("wait answer:Peer:%s+%d,RTP=%s+%d,RTCP=%s+%d\n",
                call.peer_audio.codec,call.peer_audio.ptype,
                OSSock::IPv4(call.peer_audio.addr_rtp), call.peer_audio.port_rtp,
                OSSock::IPv4(call.peer_audio.addr_rtcp),call.peer_audio.port_rtcp);

            printf("wait answer:This:%s+%d,RTP=%s+%d,RTCP=%s+%d\n",
                call.this_audio.codec,call.this_audio.ptype,
                OSSock::IPv4(call.this_audio.addr_rtp), call.this_audio.port_rtp,
                OSSock::IPv4(call.this_audio.addr_rtcp),call.this_audio.port_rtcp);
        }
        else
        {
            printf("wait no answer...\n");
        }

        /*
        * TODO:
        * we can start a RTP streaming....
        */


        /*we wait for user input*/
        printf("press any key to HOLD...\n");
        OSTTY::getkey();

        EXPECT_TRUE(sip.Hold(call));

        printf("press any key to UnHOLD...\n");
        OSTTY::getkey();

        EXPECT_TRUE(sip.UnHold(call));

        printf("press any key to End...\n");
        OSTTY::getkey();

        /*
        * TODO:
        * we can stop current RTP streaming
        */

        /*
        * after RTP stop,
        * we end this call
        */
        EXPECT_TRUE(sip.End(call));

        /*after all,let every thing go...*/
    }while(OSTTY::getkey()!=OSTTY::TTY_VK_ESC);

}

UNITTEST(SIP_incoming_call)
{
    SIPUser   sip(true);
    SIPCall   call;

    /*create*/
    EXPECT_TRUE(sip.Create());

    /*register*/
    EXPECT_TRUE(sip.Register("10.2.1.199","383889",NULL,
        "PCMU/8000/0;"
        "PCMA/8000/8;"
        "G722/16000/9;"
        "L16PCM128/8000/107;"
        "L16PCM256/16000/107;"
        "L16PCM512/32000/107;"
        ));

    do
    {
        /*
        *--------------------------------------
        * wait a call in 20s
        *--------------------------------------
        */
        printf("waiting incoming call...\n");

        /*after ${SIPStack::WaitAnswer(...)} return with true*/
        if(sip.WaitIncome(call,20*1000))
        {
            printf("wait answer:Peer:%s+%d,RTP=%s+%d,RTCP=%s+%d\n",
                call.peer_audio.codec,call.peer_audio.ptype,
                OSSock::IPv4(call.peer_audio.addr_rtp), call.peer_audio.port_rtp,
                OSSock::IPv4(call.peer_audio.addr_rtcp),call.peer_audio.port_rtcp);

            printf("wait answer:This:%s+%d,RTP=%s+%d,RTCP=%s+%d\n",
                call.this_audio.codec,call.this_audio.ptype,
                OSSock::IPv4(call.this_audio.addr_rtp), call.this_audio.port_rtp,
                OSSock::IPv4(call.this_audio.addr_rtcp),call.this_audio.port_rtcp);

            /*we wait for user input*/
            printf("press any key to HOLD...\n");
            OSTTY::getkey();

            EXPECT_TRUE(sip.Hold(call));

            printf("press any key to UnHOLD...\n");
            OSTTY::getkey();

            EXPECT_TRUE(sip.UnHold(call));

            /*
            * TODO:
            * we can start a RTP streaming....
            */
            if(!sip.WaitEnding(call,10*1000))
            {
                printf("wait timerout, end the call...\n");

                EXPECT_TRUE(sip.End(call));

                printf("press any key to continue...\n");
                if(OSTTY::getkey()==OSTTY::TTY_VK_ESC)
                    break;
            }

            /*
            * TODO:
            * we can stop current RTP streaming
            */
        }
        else
        {
            printf("wait no answer...\n");
        }

    }while(1);
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
