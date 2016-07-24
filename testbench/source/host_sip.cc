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

/*for host*/
#include "host.h"

/*for sip stack*/
#include "re.h"

namespace host
{
    /************************************************************************/
    /*Define                                                               */
    /************************************************************************/
    typedef struct 
    {
        /*DNS holder*/
        struct sa            dns_nsv[16];
        struct dnsc         *dns_dncs;

        /*sip holder*/
        struct sipsess_sock *ssock;      /* SIP session socket */
        struct sdp_session  *sdp;        /* SDP session        */
        struct sdp_media    *sdp_media;  /* SDP media          */
        struct sipreg       *reg;        /* SIP registration   */
        struct sip          *sip;        /* SIP stack          */
    }SIPHandle;

    /************************************************************************/
    /* C-API to C++ API                                                     */
    /************************************************************************/
    static int SIPStack_codec2media(struct sdp_media *media,const char*codec)
    {
        char temp[4096];
        char name[256];
        char ptype[256];
        char*cut;
        char*pos;
        char*end;
        int  srate;
        int  err;

        /*
        * "PCMU/8000/8;PCMA/8000/0"
        */
        if(codec==NULL
            || strlen(codec)<=0)
            return -1;

        strcpy(temp,codec);
        cut = temp;
        pos = temp;
        end = temp+strlen(temp);
        do
        {
            /*cut name*/
            cut = strchr(pos,'/');
            if(cut==NULL || cut>=end)
                return -1;
            *cut = '\0';
            strcpy(name,pos);

            /*cut srate*/
            pos = cut+1;
            cut = strchr(pos,'/');
            if(cut==NULL || cut>=end)
                return -1;
            *cut = '\0';
            srate = atoi(pos);

            /*cut ptype*/
            pos = cut+1;
            cut = strchr(pos,';');
            if(cut!=NULL)
                *cut = '\0';
            strcpy(ptype,pos);

            err = sdp_format_add(NULL, media, false,
                ptype, name, srate, 1,
                NULL, NULL, NULL, false, NULL);

            if(err)
                return err;

            /*next*/
            if(cut>=end || cut==NULL)
                break;
            else
                pos = cut+1;

            /*check len*/
            if(strlen(pos)<=0)
                break;

        }while(1);

        return 0;
    }

    /* called when all sip transactions are completed */
    static void SIPStack_cb_exit(void *arg)
    {
        SIPStack * sip = static_cast<SIPStack*>(arg);
        if(sip!=NULL)
            sip->OnEventExit();
    }
    /* called upon incoming calls */
    static void SIPStack_cb_connect(const struct sip_msg *msg, void *arg)
    {
        SIPStack * sip = static_cast<SIPStack*>(arg);
        if(sip!=NULL)
            sip->OnEventConnect(msg);
    }
    /* called when challenged for credentials*/
    static int SIPStack_cb_auth(char **user, char **pass, const char *realm, void *arg)
    {
        SIPStack * sip = static_cast<SIPStack*>(arg);
        if(sip!=NULL)
            return sip->OnEventRegAuth(user,pass,realm);
        return 0;
    }
    static void SIPStack_cb_resp(int err, const struct sip_msg *msg, void *arg)
    {
        SIPStack * sip = static_cast<SIPStack*>(arg);
        if(sip!=NULL)
            sip->OnEventRegResp(msg,err);
    }

    /*
    * called when an SDP offer is received (got offer: true) or
    * when an offer is to be sent (got_offer: false)
    */
    static int SIPStack_cb_offer(struct mbuf **mbp, const struct sip_msg *msg,void *arg)
    {
        SIPSess * sess = static_cast<SIPSess*>(arg);
        if(sess!=NULL && sess->stack!=NULL)
            return sess->stack->OnEventOffer(sess,msg,mbp);
        return 0;
    }
    /* called when an SDP answer is received */
    static int SIPStack_cb_answer(const struct sip_msg *msg, void *arg)
    {
        SIPSess * sess = static_cast<SIPSess*>(arg);
        if(sess!=NULL && sess->stack!=NULL)
            return sess->stack->OnEventAnswer(sess,msg);
        return 0;
    }
    /* called when the session is established */
    static void SIPStack_cb_establish(const struct sip_msg *msg, void *arg)
    {
        SIPSess * sess = static_cast<SIPSess*>(arg);
        if(sess!=NULL && sess->stack!=NULL)
            sess->stack->OnEventEstablish(sess,msg);
    }
    /* called when the session fails to connect or is terminated from peer */
    static void SIPStack_cb_close(int err, const struct sip_msg *msg, void *arg)
    {
        SIPSess * sess = static_cast<SIPSess*>(arg);
        if(sess!=NULL && sess->stack!=NULL)
            sess->stack->OnEventClose(sess,msg,err);
    }
    /* called when SIP progress (like 180 Ringing) responses are received */
    static void SIPStack_cb_progress(const struct sip_msg *msg, void *arg)
    {
        SIPSess * sess = static_cast<SIPSess*>(arg);
        if(sess!=NULL && sess->stack!=NULL)
            sess->stack->OnEventProgress(sess,msg);
    }
    static void SIPStack_cb_info(struct sip *_sip, const struct sip_msg *_msg,void *arg)
    {
        SIPSess * sess = static_cast<SIPSess*>(arg);
        if(sess!=NULL && sess->stack!=NULL)
            sess->stack->OnEventInformation(sess,_sip,_msg);
    }
    static void SIPStack_cb_refer(struct sip *_sip, const struct sip_msg *_msg,void *arg)
    {
        SIPSess * sess = static_cast<SIPSess*>(arg);
        if(sess!=NULL && sess->stack!=NULL)
            sess->stack->OnEventReference(sess,_sip,_msg);
    }

    /************************************************************************/
    /* static data                                                          */
    /************************************************************************/
    size_t SIPStack::reference = 0;
    size_t SIPStack::thisport  = 11800;

    /************************************************************************/
    /* class of SIPCall                                                     */
    /************************************************************************/
    SIPCall::SIPCall(int id,int type,int state)
    {
        Asign(id,type,state);
    }
    SIPCall::~SIPCall()
    {

    }
    SIPCall& SIPCall::operator = (SIPCall & call)
    {
        callID    = call.callID;
        callType  = call.callType;
        callState = call.callState;

        this_audio = call.this_audio;
        peer_audio = call.peer_audio;
        this_video = call.this_video;
        peer_video = call.peer_video;

        return *this;
    }
    SIPCall& SIPCall::operator = (SIPSess & sess)
    {
        return operator =(dynamic_cast<SIPCall &>(sess));
    }

    void SIPCall::Asign(int id,int type,int state)
    {
        callID    = id;
        callType  = type;
        callState = state;

        memset(&this_audio,0,sizeof(SIPParty));
        memset(&peer_audio,0,sizeof(SIPParty));
        memset(&this_video,0,sizeof(SIPParty));
        memset(&peer_video,0,sizeof(SIPParty));
    }

    void SIPCall::Clear()
    {
        callType  = SIPCall::TP_NA;
        callState = SIPCall::TP_NA;
        callID    = 0;

        memset(&this_audio,0,sizeof(SIPParty));
        memset(&peer_audio,0,sizeof(SIPParty));
        memset(&this_video,0,sizeof(SIPParty));
        memset(&peer_video,0,sizeof(SIPParty));
    }
    /************************************************************************/
    /* class of SIPSess                                                     */
    /************************************************************************/
    SIPSess::SIPSess(SIPStack *ref,int id,int type,int state)
        :SIPCall(id,type,state)
    {
        stack  = ref;
        hsess  = NULL;

        memset(this_uri,0,sizeof(this_uri));
        memset(peer_uri,0,sizeof(peer_uri));
    }

    SIPSess::~SIPSess()
    {
        /*
        * if(hsess)
        *    mem_deref(hsess);
        */
    }
    bool SIPSess::Wait(int timeout)
    {
        error = true;
        sem.Wait(timeout);

        return error;
    }
    bool SIPSess::Answer(bool sucess)
    {
        error = sucess;
        sem.Post();

        return true;
    }

    bool SIPSess::Closed()
    {
        error = false;
        sem.Post();

        return true;
    }
    bool SIPSess::Terminate()
    {
        sem.Reset();
        sipsess_close((struct sipsess*)hsess);
        callType = SIPCall::TP_NA;

        return true;
    }
    /************************************************************************/
    /* class SIPAcct                                                        */
    /************************************************************************/
    SIPAcct::SIPAcct()
    {
        memset(sip_server ,0,sizeof(sip_server));
        memset(sip_account,0,sizeof(sip_account));
        memset(sip_psword ,0,sizeof(sip_psword));
    }

    SIPAcct::~SIPAcct()
    {

    }

    /************************************************************************/
    /* class SIPMain                                                        */
    /************************************************************************/
    SIPMain::SIPMain()
    {

    }
    SIPMain::~SIPMain()
    {

    }

    bool SIPMain::Main()
    {
        /* main loop */
        re_main(NULL);

        return false;
    }

    /************************************************************************/
    /* class of SIPStack                                                    */
    /************************************************************************/
    SIPStack::SIPStack(bool debug)
    {
        debugging = debug;

        /* initialize libre state */
        if(reference++==0)
        {
            if (libre_init()) 
            {
                Debug("re init failed!\n");
            }
        }

        /*alloc handle*/
        handle_ready= false;
        handle_void =  (void*)calloc(1,sizeof(SIPHandle));
    }
    SIPStack::SIPStack(const char *srv,const char *acct,const char * pwd,const char*codec)
    {
        /*call basic construct*/
        SIPStack();

        /*do register*/

        /*
        * do create
        */
        if(Create())
        {
            /*register*/
            if(Register(srv,acct,pwd,codec,3*1000))
            {
                Debug("failed to Register to %s+%s+%s\n",
                    srv,acct,pwd);
            }
        }
    }

    SIPStack::~SIPStack()
    {
        /*free resource*/
        Destroy();

        /*free handle*/
        free(handle_void);

        /*free librar state*/
        if(--reference==0)
        {
            libre_close();
        }
    }

    bool SIPStack::Create()
    {
        SIPHandle * handle = (SIPHandle*)handle_void;
        struct sa   addr;
        int         nsc;
        int         err;

        /*
        * destroy for safe
        */
        Destroy();

        /*lock critical*/
        OSLockSection critical(handle_lock);

        /*
        * init
        */
        nsc = ARRAY_SIZE(handle->dns_nsv);
        /* fetch list of DNS server IP addresses */
        err = dns_srv_get(NULL, 0, handle->dns_nsv, (uint32_t*)&nsc);
        if (err) 
        {
            Debug("unable to get dns servers: %s\n",
                strerror(err));
            return false;
        }

        /* create DNS client */
        err = dnsc_alloc(&handle->dns_dncs, NULL, handle->dns_nsv, nsc);
        if (err) {
            Debug("unable to create dns client: %s\n",
                strerror(err));
            return false;
        }

        /* create SIP stack instance */
        err = sip_alloc(&handle->sip, handle->dns_dncs, 32, 32, 32,
            "SIPStack in TestBench 1.0.0.0",
            SIPStack_cb_exit, this);

        if (err) 
        {
            Debug("sip error: %s\n", 
                strerror(err));
            return false;
        }

        /* fetch local IP address */
        err = net_default_source_addr_get(AF_INET, &addr);
        if (err) 
        {
            Debug("local address error: %s\n",
                strerror(err));
            return false;
        }

        /* listen on random port */
        sa_set_port(&addr, 0);

        /* add supported SIP transports */
        err |= sip_transp_add(handle->sip, SIP_TRANSP_UDP, &addr);
        err |= sip_transp_add(handle->sip, SIP_TRANSP_TCP, &addr);
        if (err) 
        {
            Debug("transport error: %s\n", 
                strerror(err));
            return false;
        }

        /* create SIP session socket */
        err = sipsess_listen(&handle->ssock, handle->sip, 
            32, SIPStack_cb_connect, this);
        if (err) 
        {
            Debug("session listen error: %s\n",
                strerror(err));
            return false;
        }

        /* create SDP session */
        err = sdp_session_alloc(&handle->sdp, &addr);
        if (err) 
        {
            Debug("sdp session error: %s\n",
                strerror(err));
            return false;
        }

        /*
        * load task
        */
        handle_task.Load();

        /*mark ready*/
        handle_ready = true;
        return true;
    }

    bool SIPStack::Destroy()
    {
        SIPHandle * handle = (SIPHandle*)handle_void;

        /*
        * end all call
        */
        End();

        /*
        * try close all
        */
        sipsess_close_all(handle->ssock);

        /*
        * end all call, and wait a little!!
        */
        OS::SleepMs(100);

        /*
        * cancel poll
        */
        re_cancel();
        OS::SleepMs(100);

        /*
        * term task
        */
        handle_task.Term(false);

        /*mark invalid*/
        handle_ready = false;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        /*free pending session*/
        for (SIPSessList::iterator it=call_list.begin();it!=call_list.end();it++)
        {
            SIPSess * sess = static_cast<SIPSess*>(*it);
            delete sess;
        }
        call_list.empty();
        call_income_list.empty();

            /*free resource*/
        if(handle->sdp)
            mem_deref(handle->sdp);
        if(handle->ssock)
            mem_deref(handle->ssock);
        if(handle->sip)
            mem_deref(handle->sip);
        if(handle->dns_dncs)
            mem_deref(handle->dns_dncs);

        memset(handle,0,sizeof(SIPHandle));

        return true;
    }

    bool SIPStack::Register(const char *srv,const char *acct,const char * pwd,const char*codec,int timeout)
    {
        SIPHandle * handle = (SIPHandle*)handle_void;
        int         err;

        if(!handle_ready)
            return false;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        /*setup info*/
        if(!srv)
            srv = "10.2.1.199";
        strcpy(sip_server, srv);

        if(!acct)
            acct = "0.0.0.0";
        strcpy(sip_account,acct);

        if(!pwd)
            pwd = acct;
        strcpy(sip_psword ,pwd);

        if(!codec)
            codec = "PCMU/8000/8";

        sprintf(sip_server_reg,"sip:%s",    sip_server);
        sprintf(sip_server_uri,"sip:%s@%s", sip_account,sip_server);

        /*
        * delete media
        */
        if(handle->sdp_media)
        {
            mem_deref(handle->sdp_media);
            handle->sdp_media = NULL;
        }

        /*
        *--------------------------------------
        * add audio sdp media
        *--------------------------------------
        */
        err = sdp_media_add(&handle->sdp_media, handle->sdp, 
            "audio", 0, "RTP/AVP");
        if (err)
        {
            Debug("sdp media error: %s\n", 
                strerror(err));
            return false;
        }

        /* add sdp media format */
        err = SIPStack_codec2media(handle->sdp_media,codec);
        if (err) 
        {
            Debug("sdp format error: %s\n",
                strerror(err));
            return false;
        }

        /*
        * modify ports of RTP&RTCP
        */
        sdp_media_set_lport(handle->sdp_media, thisport++);
        sdp_media_set_lport_rtcp(handle->sdp_media,thisport++);

        /*
        *--------------------------------------
        * add video sdp media
        *--------------------------------------
        */


        /*
        * register to server
        */
        reg_error = -1;
        reg_sem.Reset();

        err = sipreg_register(&handle->reg, handle->sip, 
            sip_server_reg, sip_server_uri, sip_server_uri, 3600, sip_account,
            NULL, 0, 0, 
            SIPStack_cb_auth, this, false,
            SIPStack_cb_resp, this,
            NULL, NULL);

        if (err)
        {
            Debug("register error: %s\n",
                strerror(err));
            return false;
        }

        /*
        * wait return
        */
        reg_sem.Wait(timeout);
        if (reg_error)
        {
            Debug("wait register error: %s\n",
                strerror(reg_error));
            return false;
        }

        OnRegister(dynamic_cast<SIPAcct&>(*this));

        return true;
    }

    bool SIPStack::Unregister()
    {
        if(!handle_ready)
            return false;

        OnUnregister(dynamic_cast<SIPAcct&>(*this));

        return true;
    }

    bool SIPStack::Call(SIPCall &call,const char*toName,const char*toSrv)
    {
        SIPHandle   * handle = (SIPHandle*)handle_void;
        struct mbuf * mb;
        SIPSess     * sess;
        int           err;

        if(!handle_ready)
            return false;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        /*update ports*/
        sdp_media_set_lport(handle->sdp_media, thisport++);
        sdp_media_set_lport_rtcp(handle->sdp_media,thisport++);

        /* create SDP offer */
        err = sdp_encode(&mb, handle->sdp, true);
        if (err) 
        {
            Debug("sdp encode error: %s\n",
                strerror(err));
            return false;
        }

        /*
        * create new call
        */
        sess = CreateSess(toName,toSrv,SIPCall::TP_OUTGOING);
        if(!sess)
            return false;

        /* current call is invite status */
        sess->callState = SIPCall::ST_INVATE;

        /*
        * do connect
        */
        assert(handle->sdp_media);

        err = sipsess_connect((struct sipsess**)&sess->hsess, handle->ssock, 
            sess->peer_uri, sip_account,
            sess->this_uri, sip_account,
            NULL, 0, "application/sdp", mb,
            SIPStack_cb_auth, this, false,
            SIPStack_cb_offer, 
            SIPStack_cb_answer,
            SIPStack_cb_progress, 
            SIPStack_cb_establish,
            SIPStack_cb_info,
            SIPStack_cb_refer, 
            SIPStack_cb_close, 
            sess, NULL
            );

        /* free SDP buffer */
        mem_deref(mb);

        if (err) 
        {
            Debug("session connect error: %s\n",
                strerror(err));
            return false;
        }

        Debug("inviting (CallID=0x%08x)<%s>...\n",
            sess->callID,sess->peer_uri);

        /*
        * on calling out
        */
        OnOutgoingCall(dynamic_cast<SIPCall&>(*sess));

        /*
        * copy call info
        */
        call = dynamic_cast<SIPCall&>(*sess);

        /*
        * try to cleanup call trash
        */
        CleanupSessTrash();

        return true;
    }

    bool SIPStack::WaitIncome(SIPCall &call,int timeout)
    {
        SIPHandle   * handle = (SIPHandle*)handle_void;
        SIPSess     * sess;

        (void)handle;

        if(!handle_ready)
            return false;

        if(call_income_list.size()<=0)
        {
            call_income_error = -1;
            call_income_sem.Reset();
            call_income_sem.Wait(timeout);

            if(call_income_error)
                return false;
        }

        /*have income*/
        {
            /*lock critical*/
            OSLockSection critical(handle_lock);

            if(call_income_list.size()<=0)
                return false;

            /*get first call*/
            SIPSessList::iterator it = call_income_list.begin();
            sess = static_cast<SIPSess*>(*it);

            /*delete from income list*/
            call_income_list.erase(it);
        }

        assert(sess!=NULL);

        /*wait answer call*/
        if(sess->Wait(timeout))
        {
            /*copy info*/
            call = dynamic_cast<SIPCall&>(*sess);
            return true;
        }

        return false;
    }

    bool SIPStack::WaitAnswer(SIPCall &call,int timeout)
    {
        SIPHandle   * handle = (SIPHandle*)handle_void;
        SIPSess     * sess;

        (void)handle;

        if(!handle_ready)
            return false;

        /*
        * find call
        */
        sess = SearchSess(call);
        if(!sess)
            return false;

        /*wait answer call*/
        if(sess->Wait(timeout))
        {
            /*copy info*/
            call = dynamic_cast<SIPCall&>(*sess);
            return true;
        }
        return false;
    }

    bool SIPStack::WaitEnding(SIPCall &call,int timeout)
    {
        SIPHandle   * handle = (SIPHandle*)handle_void;
        SIPSess     * sess;
        bool          ok;

        (void)handle;

        if(!handle_ready)
            return false;

        /*
        * find call
        */
        sess = SearchSess(call);
        if(!sess)
            return false;

        /*wait end call*/
        ok = sess->Wait(timeout);

        /*
        * cleaning trash
        */
        CleanupSessTrash();

        return ok;
    }

    bool SIPStack::End(SIPCall &call)
    {
        SIPHandle   * handle = (SIPHandle*)handle_void;
        SIPSess     * sess;

        (void)handle;

        if(!handle_ready)
            return false;

        {
            /*lock critical*/
            OSLockSection critical(handle_lock);

            /*
            * find connect
            */
            sess = SearchSess(call);
            if(!sess)
            {
                Debug("End no such CallID=0x%08x\n",
                    call.callID);
                return false;
            }

            /*
            *  end this call
            */
            sess->Terminate();

            /*
            * remove call
            * do NOT call DeleteSess(sess);
            * leave the SIPStack::OnEventClose()
            * to clean this up!!
            */
        }

        /*
        * try to cleanup call trash
        */
        CleanupSessTrash();

        return true;
    }

    bool SIPStack::End()
    {
        SIPHandle * handle = (SIPHandle*)handle_void;

        (void)handle;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        while(call_list.size()>0)
        {
            SIPSessList::iterator it   = call_list.begin();
            SIPSess             * sess = static_cast<SIPSess*>(*it);

            /*
            *  end this call
            */
            sess->Terminate();

            /*
            * remove call
            * do NOT call DeleteSess(sess);
            * leave the SIPStack::OnEventClose()
            * to clean this up!!
            */
        }

        /*
        * try to cleanup call trash
        */
        CleanupSessTrash();

        return true;
    }


    bool SIPStack::Hold(SIPCall &call)
    {
        SIPHandle   * handle = (SIPHandle*)handle_void;
        SIPSess     * sess;
        int           err;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        /* create new SDP offer */
        /*err = sdp_encode(&my_mbuf, handle->sdp, true);
        if (err) 
        {
            Debug("sdp encode error: %s\n",
                strerror(err));
            return false;
        }*/

        /*
        * find connect
        */
        sess = SearchSess(call);
        if(!sess)
        {
            Debug("End no such CallID=0x%08x\n",
                call.callID);
            return false;
        }

        /* current call is hold status */
        sess->callState = SIPCall::ST_HOLD;

        {
            struct mbuf    * my_mbuf;
            struct sipsess * my_sess = (struct sipsess *)sess->hsess;

            sdp_media_set_ldir(handle->sdp_media, SDP_SENDONLY);

            /* create SDP offer */
            err = sdp_encode(&my_mbuf, handle->sdp, true);
            if (err) 
            {
                Debug("sdp encode error: %s\n",
                    strerror(err));
                return false;
            }

            err = sipsess_modify(my_sess, my_mbuf);
        }
        
        return (err == 0);
    }
    
    bool SIPStack::UnHold(SIPCall &call)
    {
        SIPHandle   * handle = (SIPHandle*)handle_void;
        SIPSess     * sess;
        int           err;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        /*
        * find connect
        */
        sess = SearchSess(call);
        if(!sess)
        {
            Debug("End no such CallID=0x%08x\n",
                call.callID);
            return false;
        }

        {
            struct mbuf *my_mbuf;
            struct sipsess *my_sess = (struct sipsess *)sess->hsess;

            sdp_media_set_ldir(handle->sdp_media, SDP_SENDRECV);

            /* create SDP offer */
            err = sdp_encode(&my_mbuf, handle->sdp, true);
            if (err) 
            {
                Debug("sdp encode error: %s\n",
                    strerror(err));
                return false;
            }

            err = sipsess_modify(my_sess, my_mbuf);
        }

        return (err == 0);
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    int SIPStack::OnEventExit()
    {
        /*lock critical*/
        OSLockSection critical(handle_lock);

        /* stop libre main loop */
        re_cancel();

        return 0;
    }

    int SIPStack::OnEventConnect(const void *msg_void)
    {
        SIPHandle            * handle = (SIPHandle*)handle_void;
        const struct sip_msg * msg    = (const struct sip_msg *)msg_void;
        struct mbuf          * mb;
        SIPSess              * sess;
        bool got_offer;
        int  err;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        got_offer = (mbuf_get_left(msg->mb) > 0);

        Debug("OnEventConnect(...)\n");

        /* Decode SDP offer if incoming INVITE contains SDP */
        if (got_offer)
        {
            err = sdp_decode(handle->sdp, msg->mb, true);
            if (err) 
            {
                Debug("unable to decode SDP offer: %s\n",
                    strerror(err));

                /*reply error*/
                sip_treply(NULL, handle->sip, msg, 500, strerror(err));
                return -1;
            }
        }

        /* Encode SDP */
        err = sdp_encode(&mb, handle->sdp, !got_offer);
        if (err) 
        {
            Debug("unable to encode SDP: %s\n",
                strerror(err));

            /*reply error*/
            sip_treply(NULL, handle->sip, msg, 500, strerror(err));
            return -1;
        }

        /*
        * alloc new call info
        */
        sess = CreateSess("","",SIPCall::TP_INCOMING);
        if(!sess)
        {
            Debug("unable to alloc SIPSess\n");

            /*reply error*/
            sip_treply(NULL, handle->sip, msg, 500, strerror(err));
            return -1;
        }

        /* current call is ring status */
        sess->callState = SIPCall::ST_RING;

        /* Answer incoming call */
        err = sipsess_accept((struct sipsess**)&sess->hsess, handle->ssock, msg, 200, "OK",
            sip_account, "application/sdp", mb,
            SIPStack_cb_auth, this, false,
            SIPStack_cb_offer, 
            SIPStack_cb_answer,
            SIPStack_cb_establish,
            SIPStack_cb_info,
            SIPStack_cb_refer,
            SIPStack_cb_close,
            sess, NULL);

        mem_deref(mb); /* free SDP buffer */

        if (err) 
        {
            Debug("session accept error: %s\n",
                strerror(err));

            /*delete session*/
            DeleteSess(sess);

            /*reply error*/
            sip_treply(NULL, handle->sip, msg, 500, strerror(err));
            return -1;
        }

        /*
        * new call
        */
        Debug("accepting incoming call from <%r>\n",
            &msg->from.auri);

        /*
        * on calling io
        */
        OnIncomingCall(dynamic_cast<SIPCall&>(*sess));

        /*
        * wake up waiter
        */
        call_income_error = 0;
        call_income_sem.Post();

        return 0;
    }
    int SIPStack::OnEventAnswer(SIPSess *sess,const void *msg_void)
    {
        SIPHandle            * handle = (SIPHandle*)handle_void;
        const struct sip_msg * msg    = (const struct sip_msg *)msg_void;
        int                    err;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        Debug("OnEventAnswer(...)\n");

        /*do action*/
        err = sdp_decode(handle->sdp, msg->mb, false);
        if (err) 
        {
            Debug("unable to decode SDP answer: %s\n",
                strerror(err));
            return err;
        }

        if (sess->callState == SIPCall::ST_HOLD)
        {
            if (sdp_media_dir(handle->sdp_media) == SDP_SENDONLY)
            {
                Debug("call is Hold...\n");
            }
            else
            {
                Debug("call is TALKING...\n");
                sess->callState = SIPCall::ST_TALKING;
            }
        }

        return 0;
    }

    int SIPStack::OnEventEstablish(SIPSess *sess,const void *msg_void)
    {
        SIPHandle              * handle = (SIPHandle*)handle_void;
        const struct sip_msg   * msg    = (const struct sip_msg *)msg_void;

        (void)msg;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        Debug("OnEventEstablish(...)\n");

        /*
        * update media
        */
        if(UpdateSessMedia(sess))
        {
            /*on call establish*/
            OnEstablished(dynamic_cast<SIPCall&>(*sess));

            /*current call is talking status*/
            sess->callState = SIPCall::ST_TALKING;

            /*wait up waiter*/
            sess->Answer(true);
        }
        else
        {
            sess->Terminate();

            /*wait up waiter*/
            sess->Answer(false);
        }

        /*
        * modify ports of RTP&RTCP
        */
        sdp_media_set_lport(handle->sdp_media, thisport++);
        sdp_media_set_lport_rtcp(handle->sdp_media,thisport++);

        return 0;
    }

    int  SIPStack::OnEventRegAuth(char **user, char **pass, const char *realm)
    {
        int err = 0;

        Debug("OnEventRegAuth(...)\n");

        err |= str_dup(user, sip_account);
        err |= str_dup(pass, sip_psword);

        return err;
    }

    int SIPStack::OnEventRegResp(const void *msg,int err)
    {
        /*save error*/
        reg_error = err;

        /*wake up*/
        reg_sem.Post();

        return 0;
    }

    int  SIPStack::OnEventClose(SIPSess *sess,const void *msg_void,int err)
    {
        SIPHandle            * handle = (SIPHandle*)handle_void;
        const struct sip_msg * msg    = (const struct sip_msg *)msg_void;

        (void)handle;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        Debug("OnEventClose(...) %s\n",strerror(err));

        /*
        * on ending
        */
        OnFinishedCall(dynamic_cast<SIPCall&>(*sess));

        /* current call is end status */
        sess->callState = SIPCall::ST_END;

        /*
        * wake up
        */
        sess->Closed();

        /*
        * remove this call
        */
        DeleteSess(sess);

        return 0;
    }

    int  SIPStack::OnEventProgress(SIPSess *sess,const void *msg_void)
    {
        SIPHandle           * handle = (SIPHandle*)handle_void;
        const struct sip_msg *msg    = (const struct sip_msg*)msg_void;

        (void)handle;
        (void)msg;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        Debug("OnEventProgress(...)\n");

        return 0;
    }

    int  SIPStack::OnEventInformation(SIPSess *sess,void *sip_void,const void *msg_void)
    {
        SIPHandle            *handle = (SIPHandle*)handle_void;
        struct sip           *sip    = (struct sip*)sip_void;
        const struct sip_msg *msg    = (const struct sip_msg*)msg_void;

        (void)handle;
        (void)sip;
        (void)msg;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        Debug("OnEventInformation(...)\n");

        return 0;
    }

    int  SIPStack::OnEventReference(SIPSess *sess,void *sip_void,const void *msg_void)
    {
        SIPHandle            *handle = (SIPHandle*)handle_void;
        struct sip           *sip    = (struct sip*)sip_void;
        const struct sip_msg *msg    = (const struct sip_msg*)msg_void;

        (void)handle;
        (void)sip;
        (void)msg;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        Debug("OnEventReference(...)\n");

        return 0;
    }

    int  SIPStack::OnEventOffer(SIPSess *sess,const void *msg_void,void* mbp_void)
    {
        SIPHandle           * handle = (SIPHandle*)handle_void;
        struct mbuf         **mbp    = (struct mbuf**)mbp_void;
        const struct sip_msg *msg    = (const struct sip_msg*)msg_void;
        const size_t    got_offer    = mbuf_get_left(msg->mb);
        int err;

        /*lock critical*/
        OSLockSection critical(handle_lock);

        Debug("OnEventOffer(...)\n");

        if (got_offer)
        {
            err = sdp_decode(handle->sdp, msg->mb, true);
            if (err) 
            {
                Debug("unable to decode SDP offer: %s\n",
                    strerror(err));
                return err;
            }

            /*
            * update media
            */
            if(UpdateSessMedia(sess))
            {
                /*
                * call event if been hold/resume!!!!
                */
                OnModifiedCall(dynamic_cast<SIPCall&>(*sess));
            }
        }
        else 
        {
            Debug("sending SDP offer\n");
        }

        err = sdp_encode(mbp, handle->sdp, !got_offer);

        if (sdp_media_dir(handle->sdp_media) != SDP_SENDRECV)
        {
            /* current call is hold status */
            sess->callState = SIPCall::ST_HELD;
            Debug("call is HELD...\n");
        }        
        else
        {
            /* current call is hold status */
            sess->callState = SIPCall::ST_TALKING;
            Debug("call is TALKING...\n");
        }

        return err;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    bool SIPStack::Debug(const char* fmt,...)
    {
        if(!debugging)
            return false;

        size_t  ret;
        va_list va;
        va_start(va,fmt);
        ret = re_vfprintf(stderr,fmt,va);
        va_end(va);

        return true;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    SIPSess * SIPStack::CreateSess(const char*toName,const char*toSrv,int type)
    {
        SIPSess *sess;
        int      callid;

        /*make ID*/
        do 
        {
            callid = rand();
        } while (SearchSess(callid)!=NULL);

        /*alloc new*/
        sess   = new SIPSess(this,callid,type,SIPCall::ST_NA);
        if(!sess)
            return NULL;

        call_list.push_back(sess);

        if(type==SIPCall::TP_INCOMING)
            call_income_list.push_back(sess);

        sprintf(sess->peer_uri, "sip:%s@%s",toName,toSrv?toSrv:sip_server);
        sprintf(sess->this_uri, "sip:%s@%s",sip_account,sip_server);

        return sess;
    }

    SIPSess * SIPStack::SearchSess(int callID)
    {
        for (SIPSessList::iterator it=call_list.begin();it!=call_list.end();it++)
        {
            SIPSess * sess = static_cast<SIPSess*>(*it);
            if(sess->callID==callID)
                return sess;
        }
        return NULL;
    }

    SIPSess * SIPStack::SearchSess(SIPCall &call)
    {
        return SearchSess(call.callID);
    }

    bool SIPStack::DeleteSess(SIPSess *sess)
    {
        for (SIPSessList::iterator it=call_list.begin();it!=call_list.end();it++)
        {
            if(static_cast<SIPSess*>(*it) == sess)
            {
                call_list.erase(it);
                break;
            }
        }

        /*remove from income*/
        DeleteSessIncome(sess);

        /*move to trash*/
        call_trash.push_back(sess);

        /*wait up all */
        sess->sem.Post();

        return true;
    }

    bool SIPStack::DeleteSess(int callID)
    {
        for (SIPSessList::iterator it=call_list.begin();it!=call_list.end();it++)
        {
            SIPSess * sess = static_cast<SIPSess*>(*it);
            if(sess->callID==callID)
            {
                /*remove from income*/
                DeleteSessIncome(sess);

                /*wait up all */
                sess->sem.Post();

                /*wait a mount*/
                OS::SleepMs(50);

                /*just delete*/
                call_list.erase(it);
                /*move to trash*/
                call_trash.push_back(sess);
                return true;
            }
        }
        return false;
    }
    bool SIPStack::DeleteSessIncome(SIPSess *sess)
    {
        for (SIPSessList::iterator it=call_income_list.begin();it!=call_income_list.end();it++)
        {
            if(static_cast<SIPSess*>(*it) == sess)
            {
                call_income_list.erase(it);
                break;
            }
        }
        return true;
    }

    bool SIPStack::CleanupSessTrash(void)
    {
        for (SIPSessList::iterator it=call_trash.begin();it!=call_trash.end();it++)
        {
            SIPSess *sess = static_cast<SIPSess*>(*it);
            if(sess->callType = SIPCall::TP_NA)
            {
                call_trash.erase(it);
                delete sess;
            }
        }
        return true;
    }
    bool SIPStack::UpdateSessMedia(SIPSess *sess)
    {
        SIPHandle              * handle = (SIPHandle*)handle_void;
        const struct sa        * raddr_rtp;
        struct sa                raddr_rtcp;
        const struct sa        * laddr_rtp;
        struct sa                laddr_rtcp;
        const struct sdp_format* rfmt;
        const struct sdp_format* lfmt;

        /*get remote address*/
        raddr_rtp = sdp_media_raddr(handle->sdp_media);
        sdp_media_raddr_rtcp(handle->sdp_media,&raddr_rtcp);
        laddr_rtp = sdp_media_laddr(handle->sdp_media);
        sdp_media_laddr_rtcp(handle->sdp_media,&laddr_rtcp);

        Debug("SDP this RTP : %J RTCP : %J\n",laddr_rtp,&laddr_rtcp);
        Debug("SDP peer RTP : %J RTCP : %J\n",raddr_rtp,&raddr_rtcp);

        /* get remote codec */
        rfmt = sdp_media_rformat(handle->sdp_media, NULL);
        if (!rfmt) 
        {
            Debug("no common remote media format found\n");
            return false;
        }

        /* get local codec */
        lfmt = sdp_media_lformat(handle->sdp_media, rfmt->pt);
        if (!lfmt)
        {
            Debug("no common local media format found\n");
            return false;
        }

        /*
        * build call
        */
        strcpy(sess->this_audio.name, "");
        strcpy(sess->this_audio.codec,lfmt->name);
        sess->this_audio.dir   = sdp_media_ldir(handle->sdp_media);;
        sess->this_audio.srate = lfmt->srate;
        sess->this_audio.ptype = lfmt->pt;
        sess->this_audio.ptime = 20;

        sess->this_audio.addr_rtp  = ntohl(sa_in(laddr_rtp));;
        sess->this_audio.port_rtp  = sa_port(laddr_rtp);
        sess->this_audio.addr_rtcp = sa_in(&laddr_rtcp);
        sess->this_audio.port_rtcp = sa_port(&laddr_rtcp);


        strcpy(sess->peer_audio.name, "");
        strcpy(sess->peer_audio.codec,lfmt->name);
        sess->peer_audio.dir   = sdp_media_rdir(handle->sdp_media);;
        sess->peer_audio.srate = lfmt->srate;
        sess->peer_audio.ptype = lfmt->pt;
        sess->peer_audio.ptime = 20;

        sess->peer_audio.addr_rtp  = ntohl(sa_in(raddr_rtp));
        sess->peer_audio.port_rtp  = sa_port(raddr_rtp);
        sess->peer_audio.addr_rtcp = sa_in(&raddr_rtcp);
        sess->peer_audio.port_rtcp = sa_port(&raddr_rtcp);

        Debug("SDP this : %s:%u, %s/%u/%d\n",
            OSSock::IPv4(sess->this_audio.addr_rtp),sess->this_audio.port_rtp,
            sess->this_audio.codec, sess->this_audio.srate, sess->this_audio.ptype);

        Debug("SDP peer: %s:%u, %s/%u/%d\n",
            OSSock::IPv4(sess->peer_audio.addr_rtp),sess->peer_audio.port_rtp,
            sess->peer_audio.codec, sess->peer_audio.srate, sess->peer_audio.ptype);

        if(sess->this_audio.addr_rtp != 0
            || sess->this_audio.port_rtp==0)
            return false;

        if(sess->peer_audio.addr_rtp == 0
            || sess->peer_audio.port_rtp==0)
            return false;

        return true;
    }
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};

/************************************************************************/
/*                                                                      */
/************************************************************************/

