/* ******************************************************************
*
*    DESCRIPTION:Copyright(c) 2010-2020 Xiamen Yealink Network Technology Co,.Ltd
*
*    AUTHOR:
*
*    HISTORY:
*
*    DATE:2013-03-02
*
* for export APIs header file , message layer
*
****************************************************************** */
#ifndef __MK_MESSAGE_H__
#define __MK_MESSAGE_H__

/************************************************************************/
/*                                                                      */
/************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

    /************************************************************************/
    /* limit define                                                         */
    /************************************************************************/
#define MKIT_MAX_NAME        16
#define MKIT_MIN_PEEK        50             /*50ms as min*/
#define MKIT_DEF_PEEK        1000           /*1s   as default*/

#define MKIT_MSG_MAX_DEPTH   (128)          /*max depth */
#define MKIT_MSG_DEF_DEPTH   (16)           /*default depth */
#define MKIT_MSG_MAX_BUFF    (128*1024)     /*max buffer size of each user*/
#define MKIT_MSG_DEF_BUFF    (8*1024)       /*max buffer size of default*/
#define MKIT_MSG_PER_BUFF    (8*1024)       /*max buffer size of each message*/
#define MKIT_MSG_MAX_TIMER   (64)           /*max timer of each user*/
#define MKIT_MSG_DEF_TIMER   (16)           /*max timer of each message*/


    /************************************************************************/
    /* basic message define                                                 */
    /************************************************************************/
    /*
    * <<<section-1>>>
    * system message 
    */
#define MKIT_MSG_INVALID    0x00000000 /*invalid message id*/

#define MKIT_MSG_SYS_MIN    0x00000000 /*min system message id*/
#define MKIT_MSG_SYS_MAX    0x000000FF /*max system message id*/

#define MKIT_MSG_SYS_LOMIN  0x00000000 /*min lower system message id*/
#define MKIT_MSG_SYS_LOMAX  0x0000007F /*max lower system message id*/
#define MKIT_MSG_SYS_UPMIN  0x00000080 /*min upper system message id*/
#define MKIT_MSG_SYS_UPMAX  0x000000FF /*max upper system message id*/

    /*emit by lower system*/
    /*
    * usage   :idle message cause by system idle.
    *          interval depended on config,1s as default,50ms as min
    * id      :0x00000001
    * target  :none
    * wparam  :none
    * lparam  :none
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_IDLE       0x00000001

    /*
    * usage   :terminate message cause by user,
    *          might be self or anyone in system!
    * id      :0x00000002
    * target  :none
    * wparam  :none
    * lparam  :none
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_QUIT       0x00000002

    /*
    * usage   :timer default message,
    *          might be self or anyone in system!
    * id      :0x00000003
    * target  :none
    * wparam  :timer id that set by user,could be any value
    * lparam  :system tick,kernel jiffies
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_TIMER      0x00000003

    /*
    * usage   :partner sign-in or sign-out event
    * id      :0x00000004
    * target  :none
    * wparam  :1==sign-in,0==sign-out
    * lparam  :the partner id,if it's sign-out message,it's invalid to access!
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_REGISTER   0x00000004

    /*
    * usage   :event loop start with init,cause by current task;
    *          user must return 0 as success to indicate go on!
    * id      :0x00000005
    * target  :none
    * wparam  :none
    * lparam  :none
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_INIT       0x00000005

    /*
    * usage   :event loop end with terminate
    * id      :0x00000006
    * target  :none
    * wparam  :none
    * lparam  :none
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_TERM       0x00000006

    /*
    * <<<section-2>>>
    * emit by upper system
    */

    /*
    * usage   :stander msg to control syslog, would be cause by anyone
    * id      :0x00000080
    * target  :target id of who cause the message
    * wparam  :
    *           HIWORD.HIBYTE:0==skip,1==syslog,2==log to stdout,3==log to file
    *           HIWORD.LOBYTE:module id of syslog
    *           LOWORD.HIBYTE:level of syslog
    *           LOWORD.LOBYTE:none
    * lparam  :none
    * data_ptr:if NOT null then it's would be redirect output file path!
    * data_len:none
    *
    *          for more detail ,please reference to stander file 'log.h'
    *
    */
#define MKIT_MSG_SYSLOG     0x00000080

    /*
    * usage   :stander msg to control running stats dump, would be cause by anyone
    * id      :0x00000081
    * target  :target id of who cause the message
    * wparam  :0==dump all stats,1==dump main stats
    * lparam  :none
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_STATS      0x00000081

    /*
    * usage   :stander msg to control memory check, would be cause by anyone
    * id      :0x00000082
    * target  :target id of who cause the message
    * wparam  :0==dump memory info,1==dump all details
    * lparam  :0==dump without check,1==dump with check or free
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_MEMCHECK   0x00000082

    /*
    * usage   :system reboot request,or reboot notification
    * id      :0x00000083
    * target  :target id of who cause the message
    * wparam  :
    * lparam  :
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_REBOOT    0x00000083

    /*
    * usage   :service register request,launcher is the only valid target!!
    * id      :0x00000084 / 0x00000085
    * target  :target id of who cause the message
    * wparam  :MAKELONG(id,0)
    * lparam  :MKSHMEM_KEY_SERVICE
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_SRVREG     0x00000084
#define MKIT_MSG_SRVUNREG   0x00000085

    /*
    * usage   :service register/unregister event,launcher is the only valid sender!!
    * id      :0x00000084 / 0x00000085,the same as MKIT_MSG_SRVREG/MKIT_MSG_SRVUNREG
    * target  :target id of who cause the message
    * wparam  :service ID
    * lparam  :none
    * data_ptr:none
    * data_len:none
    * notice  :this notification MUST be requested actively!
    *          we can user the under API to request this event by setting notifier to '0'!
    *
    *           int mkit_notify_request(mkit_handle_t caller,mkit_target_t notifier,int event,int count);
    *           int mkit_notify_cancel (mkit_handle_t caller,mkit_target_t notifier,int event);
    */
#define MKIT_MSG_SRVREADY   0x00000084 /*emit when service is ready*/
#define MKIT_MSG_SRVLEAVE   0x00000085 /*emit when service is leaved*/

    /*
    * usage   :service data request,launcher is the only valid target!!
    * id      :0x00000086
    * target  :target id of who cause the message
    * wparam  :MAKELONG(id,index)
    * lparam  :data value
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_SRVDATA    0x00000086

    /*
    * usage   :watch service for application
    * id      :0x00000087 0x00000088
    * target  :target id of who cause the message
    * wparam  :startup delay as seconds
    * lparam  :none
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_REQ_WATCH  0x00000087
#define MKIT_MSG_DEL_WATCH  0x00000088

    /*
    * usage   :notify manage 
    * id      :0x00000089 0x0000008A
    * target  :target id of who want notify from
    * wparam  :notify message
    * lparam  :number of notify wanted,after count down to 0,notify service will be deleted!
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_REQ_NOTIFY  0x00000089
#define MKIT_MSG_DEL_NOTIFY  0x0000008A

    /*
    * usage   :just for test;should NOT care on run-time
    * id      :0x000000E0 to 0x000000EE
    * target  :target id of who cause the message
    * wparam  :
    * lparam  :
    * data_ptr:none
    * data_len:none
    */
#define MKIT_MSG_TEST_LOOP  0x000000E0


    /*
    * <<<section-3>>>
    * service message 
    */
#define MKIT_MSG_SRV_MIN    0x000000FF /*min service-define message id*/
#define MKIT_MSG_SRV_MAX    0x00FFFFFF /*max service-define message id*/

    /*
    * <<<section-4>>>
    * user define message 
    */
#define MKIT_MSG_USR_MIN    0x00FFFFFF /*min user-define message id*/
#define MKIT_MSG_USR_MAX    0xFFFFFFFF /*max user-define message id*/

    /************************************************************************/
    /* common define                                                        */
    /************************************************************************/
    /*
    * type of instance handle
    */
    typedef unsigned long  mkit_handle_t;
    typedef unsigned long  mkit_target_t;
    typedef unsigned long  mkit_notify_t;

    /*
    * type of per mk_create
    * MUST be the same as struct mk_create!!
    */
#define MKIT_CFL_NONE      0x00000000     /*Flags:valid none*/
#define MKIT_CFL_OWN_MSG   0x00000001     /*Flags:using own message cache*/
#define MKIT_CFL_OWN_TMR   0x00000002     /*Flags:using own timer cache*/
#define MKIT_CFL_EXC_BCAST 0x00000004     /*Flags:NOT receive broadcast message*/
#define MKIT_CFL_EXC_REGS  0x00000008     /*Flags:NOT send any register message*/

    /*
    * type of query
    * MUST be the same as struct mk_query!!
    */
    typedef struct 
    {
        char          name[MKIT_MAX_NAME];   /*name of target*/
        mkit_target_t first;                 /*handle of first target*/
        mkit_target_t next;                  /*handle of next  target*/
        unsigned int  pid;                   /*pid of return target*/
    }mkit_query_t;

    /*
    * type of per message
    * MUST be the same as struct mk_message!!
    */
    typedef struct
    {
        unsigned long  target;              /*handle of message target,if 0 then broadcast to all*/
        unsigned short sync;                /*wait sync,count as ms*/
        unsigned short flags;               /*control flags*/
#define MKIT_MFL_NONE      0x00000000       /*Flags:valid none*/
#define MKIT_MFL_UMASK     0x0000FFFF       /*Flags:user option mask*/
#define MKIT_MFL_KMASK     0xFFFF0000       /*Flags:kernel option mask*/
#define MKIT_MFL_SYNC      (1<<0)           /*Flags:send sync,else post(async)*/
#define MKIT_MFL_SYNC_HARD (1<<1)           /*Flags:send sync,and wait until message has been handle by target*/
#define MKIT_MFL_WAIT      (1<<2)           /*Flags:wait for message,else just peek once*/
#define MKIT_MFL_WAIT_HARD (1<<3)           /*Flags:wait for message,NOT defined...*/
#define MKIT_MFL_CALL_RET  (1<<4)           /*Flags:want call with return.*/
#define MKIT_MFL_CALL_DAT  (1<<5)           /*Flags:want call with data return.*/

        unsigned int   message;             /*message ID*/
        unsigned long  wparam;              /*word param*/
        unsigned long  lparam;              /*long param*/
        unsigned char *data_ptr;            /*extra data pointer,or address offset*/
        unsigned short data_len;            /*extra data valid length,max would be 65535 bytes*/
        unsigned short data_max;            /*extra data total length,max would be 65535 bytes*/
    }mkit_message_t;

    /*
    * type of per mk_timer
    * MUST be the same as struct mk_timer!!
    */
#define MKIT_TFL_NONE      0x00000000    /*Flags:valid none*/
#define MKIT_TFL_ONCE      (1<<0)        /*Flags:active for once ,else repeat timer*/
#define MKIT_TFL_DUP       (1<<1)        /*Flags:do not check reduplication*/

    /************************************************************************/
    /* APIs for c                                                           */
    /************************************************************************/
    /*helper*/
    /**
    * @brief  :retrieve handle of current thread
    *
    * @param  :handle use to save the return value;if failed value will NOT be changed
    *
    * @warning:must be run on the same task or thread that create the "mkit_handle_t"
    *          and there should be only one task ,otherwise will properly run into chaos!
    * @return :0 is success,otherwise failed.
    * 
    */
    int  mkit_get_current(mkit_handle_t *handle);

    /**
    * @brief  :retrieve private data "mkit_handle_t"
    *
    * @param  :the instance handle 
    *
    * @warning:
    *
    * @return :0 is success,otherwise failed.
    * 
    */
    void*mkit_get_private(mkit_handle_t handle);

    /**
    * @brief  :retrieve fd of handle "mkit_handle_t"
    *
    * @param  :the instance handle 
    *
    * @warning:
    *
    * @return :>=0 as fd,otherwise failed.
    * 
    */
    int  mkit_get_filedes(mkit_handle_t handle);

    /**
    * @brief  :retrieve target of handle "mkit_handle_t"
    *
    * @param  :handle is the instance handle 
    *          ptarget where to save the target id,value will NOT be change if failed
    * @warning:"ptarget" MUST be valid!
    *
    * @return :0 is success,otherwise failed.
    * 
    */
    int  mkit_get_target(mkit_handle_t handle,mkit_target_t * ptarget);

    /**
    * @brief  :retrieve mmap buffer base of handle "mkit_handle_t"
    *
    * @param  :base to save address 
    *          size to save total size
    * @warning:"base" and "size" could be NULL to skip saving value
    *
    * @return :0 is success,otherwise failed.
    * 
    */
    int  mkit_get_buffer(mkit_handle_t handle,unsigned long *base,unsigned int *size);

    /**
    * @brief  :retrieve task-id/thread-id of handle "mkit_handle_t"
    *
    * @param  :handle of "mkit_handle_t"
    *
    * @warning:
    *
    * @return :>0==task-id or thread-id,otherwise failed
    * 
    */
    int  mkit_get_taskid  (mkit_handle_t handle);
    int  mkit_get_threadid(mkit_handle_t handle);

    /**
    * @brief  :to get state of active of target , by handle "mkit_handle_t"
    *
    * @param  :handle of "mkit_handle_t"
    *          target witch want to query
    * @warning:
    *
    * @return :0 is NOT active,1 is active
    * 
    */
    int  mkit_get_active(mkit_handle_t handle,mkit_target_t target);

    /**
    * @brief  :to activate or deactivate a target , by handle "mkit_handle_t"
    *
    * @param  :handle witch want to be change,"mkit_handle_t"
    *          active,0==deactivate,1==activate
    * @warning:
    *
    * @return :0 is success,otherwise failed
    * 
    */
    int  mkit_set_active(mkit_handle_t handle,int active);

    /**
    * @brief  :to create a instance of "mkit_handle_t";
    *          after this call your named task will be active in system
    *
    * @param  :handle, use to save return handle,"mkit_handle_t"
    *          name,task name,would NOT be reduplicated
    *          depth,message queue max depth,16 as default(MK_MSG_DEF_DEPTH),64 as max(MK_MSG_MAX_DEPTH)
    *          buf_max,max size of message buffer heap
    *          buf_per,max size of each message buffer
    *          tmr_max,max timer that's allowed
    *          private_data,user private data
    *          flags,create flags ,please reference to MKIT_CFL_NONE,etc
    * @warning:
    *
    * @return :0 is success,otherwise failed
    * 
    */
    int  mkit_create (mkit_handle_t *handle,const char *name,int flags);
    int  mkit_create2(mkit_handle_t *handle,const char *name,int depth,int buf_max,int buf_per,int tmr_max,void *private_data,int flags);

    /**
    * @brief  :to destroy a target of handle "mkit_handle_t" and free current instance
    *
    * @param  :handle witch want to be destroyed,"mkit_handle_t"
    *
    * @warning:handle can NOT be use no more after destroy!
    *
    * @return :0 is success,otherwise failed
    * 
    */
    int  mkit_destroy(mkit_handle_t  handle);

    /**
    * @brief  :to query instance in current system by handle "mkit_handle_t"
    *
    * @param  :handle, witch query through
    *          target(mkit_target_t),where ti save the find result
    *          name,name of target to find,if NULL or "" ,then will return the first task of system
    *          for mkit_next() or mkit_next2(),it will return the next target of the list
    *
    * @warning:the "next" call will properly failed ,because anyone would be add or remove at runtime!
    *          the name of mkit_exist(...) can NOT be NULL,there is not safe check inside!
    *
    * @return :0 is success,otherwise failed
    * 
    */
    int  mkit_exist(mkit_handle_t handle,const char* name);
    int  mkit_name (mkit_handle_t handle,mkit_target_t  target,char* name);
    int  mkit_topid(mkit_handle_t handle,unsigned int  *pid,   const char* name);
    int  mkit_find (mkit_handle_t handle,mkit_target_t *target,const char* name);
    int  mkit_next (mkit_handle_t handle,mkit_target_t *target);
    int  mkit_find2(mkit_handle_t handle,mkit_query_t *query);
    int  mkit_next2(mkit_handle_t handle,mkit_query_t *query);

    /**
    * @brief  :to retrieve message of handle "mkit_handle_t"
    *
    * @param  :handle witch want to be peek,"mkit_handle_t"
    *          msg,where to save the message
    *          
    *          for none-block usage:
    *               set msg->flags = MKIT_MFL_NONE,msg->sync = 0
    *               then call will return without block!
    *               it can co-work with select(...)
    *          for timeout block usage:
    *               set msg->flags = MKIT_MFL_WAIT,it will wait until message ready or interrupted by user
    *               set msg->sync>0,it will wait untill "msg->sync" timeout
    *               set msg->flags = MKIT_MFL_WAIT_HARD,will wait untill message ready regardless interrupted!
    * @warning:
    *
    * @return :>0 is success,==0 is queue empty,otherwise failed
    * 
    */
    int  mkit_peek (mkit_handle_t handle,mkit_message_t *msg);

    /**
    * @brief  :send/post/broadcast message to target in msg(msg->target) by "mkit_handle_t"
    *
    * @param  :handle use as source,"mkit_handle_t"
    *
    * @warning:
    *
    * @return :0 is success,otherwise failed
    * 
    */
    int  mkit_send2(mkit_handle_t handle,mkit_message_t *msg);
    int  mkit_post2(mkit_handle_t handle,mkit_message_t *msg);
    int  mkit_bcast(mkit_handle_t handle,mkit_message_t *msg);


    /**
    * @brief  :call message to target in msg(msg->target) by "mkit_handle_t"
    *
    * @param  :handle use as source,"mkit_handle_t"
    *
    * @warning:
    *
    * @return :0 is success,otherwise failed
    * 
    */
    int  mkit_call(mkit_handle_t handle,mkit_message_t *msg);

    /**
    * @brief  :reply a receive "send" message by "mkit_handle_t"
    *
    * @param  :handle use as source,"mkit_handle_t"
    *
    * @warning:you have to call this every time after each message processed
    *
    * @return :0 is success,otherwise failed
    * 
    */
    int  mkit_reply(mkit_handle_t handle,mkit_message_t *msg);
    
    static inline int  mkit_return(mkit_handle_t handle,unsigned long wparam,unsigned long lparam,void *data,int data_siz)
    {
        mkit_message_t  retmsg;
        retmsg.wparam   = wparam;
        retmsg.lparam   = lparam;
        retmsg.data_ptr = data;
        retmsg.data_len = data_siz;
        return mkit_reply(handle,&retmsg);
    }

    /**
    * @brief  :to cleanup all message in queue of "mkit_handle_t"
    *
    * @param  :handle to be flushed,"mkit_handle_t"
    *          flushed,number of message have been flush
    * @warning:
    *
    * @return :0 is success,otherwise failed
    * 
    */
    int  mkit_flush(mkit_handle_t handle,int*flushed);

    /**
    * @brief  :start/kill/reset timer of "mkit_handle_t"
    *
    * @param  :handle to be set on,"mkit_handle_t"
    *          id,timer id
    *          ms_interval,timer interval as ms
    *          message,message of the timer ,default as MKIT_MSG_TIMER
    *          flags,control flags,would be 0
    * @warning:
    *
    * @return :0 is success,otherwise failed
    * 
    */
    int  mkit_set_timer (mkit_handle_t handle, int id,int ms_interval);
    int  mkit_set_timer2(mkit_handle_t handle, int id,int ms_interval,int message,int flags);
    int  mkit_kill_timer(mkit_handle_t handle, int id);
    int  mkit_reset_timer(mkit_handle_t handle);

    /**
    * @brief  :simple inline call
    *
    * @param  :
    *
    * @warning:
    *
    * @return :0 is success,otherwise failed
    * 
    */
    static inline int  mkit_send(mkit_handle_t handle,mkit_target_t target,
        unsigned int msg,unsigned long wparam,unsigned long lparam,void *data,int data_siz,int timeout)
    {
        mkit_message_t mkmsg = {target,timeout,MKIT_MFL_SYNC,msg,wparam,lparam,(unsigned char*)data,data_siz,0};
        return mkit_send2(handle,&mkmsg);
    };

    static inline int  mkit_post(mkit_handle_t handle,mkit_target_t target,
        unsigned int msg,unsigned long wparam,unsigned long lparam,const void *data,int data_siz)
    {
        mkit_message_t mkmsg = {target,0,MKIT_MFL_NONE,msg,wparam,lparam,(unsigned char*)data,data_siz,0};
        return mkit_post2(handle,&mkmsg);
    };

    static inline int  mkit_broadcast(mkit_handle_t handle,
        unsigned int msg,unsigned long wparam,unsigned long lparam,const void *data,int data_siz)
    {
        mkit_message_t mkmsg = {0,0,MKIT_MFL_NONE,msg,wparam,lparam,(unsigned char*)data,data_siz,0};
        return mkit_bcast(handle,&mkmsg);
    };

    /************************************************************************/
    /* APIs for notify manage                                               */
    /************************************************************************/
    /**
    * @brief  :notify manage create/destroy
    *
    * @param  :owner,notify owner,who provide the notification
    *          notify_msg,notify message,only one support
    *          notify_max,max service target support
    *
    * @warning:
    *
    * @return :0 is success,otherwise failed
    * 
    */
    int mkit_notify_create (mkit_notify_t *phandle,mkit_handle_t owner,int notify_max);
    int mkit_notify_destroy(mkit_notify_t  handle);

    /**
    * @brief  :notify manage add/delete
    *
    * @param  :caller,max service target support
    *
    * @warning:
    *
    * @return :0 is success,otherwise failed
    * 
    */
    int mkit_notify_add(mkit_notify_t  handle,mkit_target_t caller,int count);
    int mkit_notify_del(mkit_notify_t  handle,mkit_target_t caller);

    /**
    * @brief  :notify action
    *
    * @param  :handle,notify handle
    *          message,notify message
    *          wparam,lparam,data,data_len,message params
    *
    * @warning:
    *
    * @return :number of listener has been notified successfully
    * 
    */
    int mkit_notify_post(mkit_notify_t  handle,int message,unsigned long  wparam,unsigned long  lparam,const void*data,int data_len);
    int mkit_notify_send(mkit_notify_t  handle,int message,unsigned long  wparam,unsigned long  lparam,const void*data,int data_len);

    /**
    * @brief  :notify user request/cancel
    *
    * @param  :caller,who want to request this notification
    *          notifier,who provide the notification
    *          event,notify event
    *          count,notify counter,-1 as forever
    *
    * @warning:
    *          if notifier is zero,notifier will be set as launcher!
    *          if notifier is zero,notifier will be set as launcher!
    * @return :0 is success,otherwise failed
    * 
    */
    int mkit_notify_request(mkit_handle_t caller,mkit_target_t notifier,int event,int count);
    int mkit_notify_cancel (mkit_handle_t caller,mkit_target_t notifier,int event);

    /************************************************************************/
    /* APIs for anonymous access                                            */
    /************************************************************************/
    int  mkit_anonymous_create (int*fd_handle);
    int  mkit_anonymous_destroy(int fd_handle);
    int  mkit_anonymous_send2  (int fd_handle,mkit_message_t *msg);
    int  mkit_anonymous_post2  (int fd_handle,mkit_message_t *msg);
    int  mkit_anonymous_bcast  (int fd_handle,mkit_message_t *msg);
    int  mkit_anonymous_call2  (int fd_handle,mkit_message_t *msg);
    int  mkit_anonymous_find   (int fd_handle,mkit_query_t *query);
    int  mkit_anonymous_next   (int fd_handle,mkit_query_t *query);

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* __MK_MESSAGE_H__ */
