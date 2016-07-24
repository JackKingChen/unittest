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


#ifndef __HOST_SIP_H__
#define __HOST_SIP_H__

/************************************************************************/
/*                                                                      */
/************************************************************************/

namespace host
{
    class SIPStack;
    class SIPCall;
    class SIPSess;
    class SIPAcct;

    /*
    * type of SIPCall
    */
    typedef struct
    {
        char           name[256];
        unsigned int   dir;
        unsigned int   addr_rtp;
        unsigned short port_rtp;
        unsigned int   addr_rtcp;
        unsigned short port_rtcp;
        char           codec[32];
        unsigned int   srate;
        unsigned int   ptype;
        unsigned int   ptime;
    } SIPParty;

    /*
    * class of SIPCall
    */
    class SIPCall
    {
    public:
        SIPCall(int id=0,int type=TP_NA,int state=ST_NA);
        ~SIPCall();
    public:
        enum
        {
            TP_NA,
            TP_OUTGOING,
            TP_INCOMING,
        };
        enum
        {
            ST_NA,
            ST_INVATE,
            ST_RING,
            ST_PICKUP,
            ST_TALKING,
            ST_HOLD,
            ST_HELD,
            ST_END,
        };
        enum
        {
            DIR_INACTIVE = 0,
            DIR_RECVONLY = 1,
            DIR_SENDONLY = 2,
            DIR_SENDRECV = 3,
        };
    public:
        SIPCall& operator = (SIPCall & call);
        SIPCall& operator = (SIPSess & conn);

        void Asign(int id,int type,int state);
        void Clear();

    public:

        static const char* Type2Name(int type)
        {
            switch(type)
            {
            case TP_NA:      return "TP_NA";
            case TP_OUTGOING:return "TP_OUTGOING";
            case TP_INCOMING:return "TP_INCOMING";
            }
            return "error";
        };

        static const char* State2Name(int state)
        {
            switch(state)
            {
            case ST_NA:     return "ST_NA";
            case ST_INVATE: return "ST_INVATE";
            case ST_RING:   return "ST_RING";
            case ST_PICKUP: return "ST_PICKUP";
            case ST_TALKING:return "ST_TALKING";
            case ST_HOLD:   return "ST_HOLD";
            case ST_HELD:   return "ST_HELD";
            case ST_END:    return "ST_END";
            }
            return "error";
        };

        static const char* Dir2Name(int dir)
        {
            switch(dir)
            {
            case DIR_INACTIVE:return "DIR_INACTIVE";
            case DIR_RECVONLY:return "DIR_RECVONLY";
            case DIR_SENDONLY:return "DIR_SENDONLY";
            case DIR_SENDRECV:return "DIR_SENDRECV";
            }
            return "error";
        };

    public:

        /*TODO:add needed info*/
        int        callID;
        int        callType;
        int        callState;

        /*
        * info audio
        */
        SIPParty   this_audio;
        SIPParty   peer_audio;

        /*
        * info video
        */
        SIPParty   this_video;
        SIPParty   peer_video;
    };
    typedef std::list<SIPCall*> SIPCallList;

    /*
    * class of SIPSess
    */
    class SIPSess:
        public SIPCall
    {
    public:
        SIPSess(SIPStack *ref,int id,int type,int state);
        ~SIPSess();

    public:
        bool Wait(int timeout=0);
        bool Answer(bool sucess=true);
        bool Closed();
        bool Terminate();

    public:
        /*lib handle*/
        SIPStack      *stack;
        void          *hsess;
        bool           error;
        OSSemaphore    sem;

        /*TODO:add needed info*/
        char           this_uri[256];
        char           peer_uri[256];

    };
    typedef std::list<SIPSess*> SIPSessList;

    /*
    * class of SIPAcct
    */
    class SIPAcct
    {
    public:
        SIPAcct();
        ~SIPAcct();

    public:
        int   accountID;

        /*TODO:add needed info*/
        char  sip_server[256];
        char  sip_account[256];
        char  sip_psword[256];

        char  sip_server_reg[256];
        char  sip_server_uri[256];
    };
    typedef std::list<SIPAcct*> SIPAcctList;

    /*
    * class of SIPMain
    */
    class SIPMain:
        public OSThread
    {
    public:
        SIPMain();
        ~SIPMain();

    public:
        virtual bool Main();
    };

    /*
    * class of SIPStack
    */
    class SIPStack:
        public SIPAcct
    {
    public:
        SIPStack(bool debug=false);
        SIPStack(const char *srv,const char *acct,const char * pwd,const char*codec);
        ~SIPStack();

    public:
        /*
        * instance create/destroy
        * must be call to alloc and initial before any action
        */
        bool Create();
        bool Destroy();

        /*
        * register/unregister
        * call to login an SIP server
        * or log off from server.
        *
        * ${codec} = "PCMU/8000/8;PCMA/8000/0"
        */
        bool Register(const char *srv,const char *acct,const char * pwd,const char*codec,int timeout=3000);
        bool Unregister();

        /*
        * call control
        * to make a call in timeout
        */
        bool Call(SIPCall &call,const char*toName,const char*toSrv);

        /*
        * call control
        * to wait for a call in timeout
        */
        bool WaitIncome(SIPCall &call,int timeout=0);
        bool WaitAnswer(SIPCall &call,int timeout=0);
        bool WaitEnding(SIPCall &call,int timeout=0);

        /*
        * call control
        * to end current call
        */
        bool End(SIPCall &call);
        bool End();

        bool Hold(SIPCall &call);
        bool UnHold(SIPCall &call);

        /*status*/


        /*event*/
        virtual bool OnRegister  (SIPAcct &call){return true;};
        virtual bool OnUnregister(SIPAcct &call){return true;};

        virtual bool OnOutgoingCall(SIPCall &call){return true;};
        virtual bool OnIncomingCall(SIPCall &call){return true;};
        virtual bool OnEstablished (SIPCall &call){return true;};
        virtual bool OnFinishedCall(SIPCall &call){return true;};
        virtual bool OnModifiedCall(SIPCall &call){return true;};

    public:
        /*
        * interval func
        */

        /*global callback*/
        virtual int  OnEventExit();
        virtual int  OnEventConnect(const void *msg);
        virtual int  OnEventRegAuth(char **user, char **pass, const char *realm);
        virtual int  OnEventRegResp(const void *msg,int err);

        /*session callback*/
        virtual int  OnEventAnswer     (SIPSess *sess,const void *msg);
        virtual int  OnEventEstablish  (SIPSess *sess,const void *msg);
        virtual int  OnEventOffer      (SIPSess *sess,const void *msg,void* mbp);
        virtual int  OnEventClose      (SIPSess *sess,const void *msg,int err);
        virtual int  OnEventProgress   (SIPSess *sess,const void *msg);
        virtual int  OnEventInformation(SIPSess *sess,void *sip,const void *msg);
        virtual int  OnEventReference  (SIPSess *sess,void *sip,const void *msg);

        /*debug*/
        virtual bool Debug(const char* fmt,...);

    protected:
        SIPSess * CreateSess(const char*toName,const char*toSrv,int type);
        SIPSess * SearchSess(int callID);
        SIPSess * SearchSess(SIPCall &call);
        bool      DeleteSess(SIPSess *sess);
        bool      DeleteSess(int callID);
        bool      DeleteSessTrash(SIPSess *sess);
        bool      DeleteSessIncome(SIPSess *sess);
        bool      CleanupSessTrash(void);
        bool      UpdateSessMedia(SIPSess *sess);

    protected:
        /*
        * static data
        */
        static size_t reference;
        static size_t thisport;

        /*
        * interval data
        */
        bool          debugging;
        SIPMain       handle_task;
        OSLock        handle_lock;
        bool          handle_ready;
        void         *handle_void;

        /*call info*/
        int           reg_error;
        OSSemaphore   reg_sem;

        /*call trash*/
        SIPSessList   call_trash;

        /*call info:income*/
        int           call_income_error;
        OSSemaphore   call_income_sem;
        SIPSessList   call_income_list;

        /*call info*/
        int           call_error;
        OSSemaphore   call_sem;
        SIPSessList   call_list;
    };

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};

#endif
