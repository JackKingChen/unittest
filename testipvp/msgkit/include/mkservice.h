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
* for export APIs header file , task/service layer
*
****************************************************************** */
#ifndef __MK_SERVICE_H__
#define __MK_SERVICE_H__


#include "mkmessage.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
    /************************************************************************/
    /* micro define                                                         */
    /************************************************************************/
    /**
    * @brief  :service ID define
    * 
    */
#define MKSRV_ID_INVALID      -1    /*invalid id*/
#define MKSRV_ID_MIN          0     /*min possible ID*/
#define MKSRV_ID_MAX          24    /*max possible ID*/
#define MKSRV_PD_MAX          24    /*max private data index*/

    /*basic service from 0-16*/
#define MKSRV_ID_LAUNCHER     0     /*launcher service*/
#define MKSRV_ID_CONFIG       1     /*config service*/
#define MKSRV_ID_SYSTEM       2     /*system/network service,old name "rtService"*/
#define MKSRV_ID_AUTOP        3     /*autop/update service*/
#define MKSRV_ID_SIPUA        4     /*SIP service*/
#define MKSRV_ID_IPVP         5     /*audio/video service*/
#define MKSRV_ID_DSK          6     /*LCD-UI/desktop service*/
#define MKSRV_ID_WUI          7     /*WEB-UI/CGI service*/

    /*extension service from 16-24*/
#define MKSRV_ID_TR069        16     /*TR069/TR111 service*/
#define MKSRV_ID_SNMP         17     /*SNMP service*/
#define MKSRV_ID_PCAP         18     /*pcap service*/

    /*for making service msg*/
#define MKSRV_MSG(id,msg)     ((((id)&0xFF)<<16)|((msg)&0xFFFF)<<0)

    /*for service request timeout*/
#define MKSRV_REQ_TIMEOUT     1000


    /************************************************************************/
    /* type define                                                          */
    /************************************************************************/
    /*log callback type*/
    typedef void (*mkit_debuglog)(int module,int log_level,const char* fmt,...);

    /*event callback type*/
    typedef int  (*mkit_callback)(mkit_handle_t handle,void *private_data,const mkit_message_t *msg);

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    /*for version*/
    const char* mkit_version(void);
    const char* mkit_build(void);

    /*for debug*/
    /**
    * @brief  :setup global syslog hook,if NULL or default,will NOT printf any info!
    *
    * @param  :module,user defined syslog module id
    *          cb,syslog callback
    *
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    void mkit_debug(int module,mkit_debuglog cb);


    /************************************************************************/
    /* APIs for multi-tasking                                               */
    /************************************************************************/
    
    /*for create/config/destroy*/
    /**
    * @brief  :use to create msgkit based task instance handle,but without register
    *
    * @param  :handle,use to save return result,will NOT be change if failed
    *          name,task name,set to NULL to use system generated default name!
    *
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_create(mkit_handle_t *handle,const char *name);

    /**
    * @brief  :use to destroy msgkit based task instance ,and destroy os based thread!
    *
    * @param  :handle,witch instance want to be destroy
    *          
    * @warning:if task is still running,we will try to stop and then destroy registration
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_destroy(mkit_handle_t  handle);
    
    /**
    * @brief  :do config of the instance before starting
    *
    * @param  :handle,witch instance want to config
    *          depth,message depth,use -1 as default 16
    *          buf_max,max receive data buffer as bytes,-1 as default 8*1024
    *          buf_per,max receive data for one message,-1 as default 2*1024
    *          tmr_max,max timer can be set in instance
    *          priority,priority of current task [-20,+20,-1 as default]
    *          peekms,wait time when each peek,min setting is 50ms,-1 as default 1s;if invalid then will be adjust to default
    *          flags,creation flags,pleaser reference to mkmessage.h
    *          
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_config(mkit_handle_t  handle,int depth,int buf_max,int buf_per,int tmr_max,int priority,int peekms,int flags);

    /**
    * @brief  :poll instance on current thread
    *
    * @param  :handle,witch instance to run
    *          event,event callback,can NOT be NULL
    *          private_data,void pointer for user
    *
    * @warning:you'd better call this function on the thread witch create the instance!
    *          call will block until task terminated!
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_run(mkit_handle_t  handle,mkit_callback event,void *private_data);

    /**
    * @brief  :create a new thread and put the instance running on it
    *
    * @param  :handle,witch instance to start
    *          event,event callback,can NOT be NULL
    *          private_data,void pointer for user
    *
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_start(mkit_handle_t  handle,mkit_callback event,void *private_data);

    /**
    * @brief  :stop the thread softly,task will receive MKIT_MSG_EXIT message
    *
    * @param  :handle,witch instance to stop
    *          sync,1==wait until the thread is exit safely,0==just tell the thread exit without wait
    *
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_stop(mkit_handle_t  handle,int sync);

    /**
    * @brief  :terminate the thread rudely
    *
    * @param  :handle,witch instance to terminate
    *
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_term (mkit_handle_t  handle);

    /**
    * @brief  :use for task to quit by himself
    *
    * @param  :handle,witch instance to quit
    *
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_quit(mkit_handle_t  handle);

    /**
    * @brief  :use for user to define self behavior;
    *          also,we can user this series of API to combine
    *          the event poll with another group of fd(file describetors)
    *
    * @param  :handle,witch instance to control
    *
    * @warning:!!!event callback,without valid check,caller must guarantee param valid!!!
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_on_init(mkit_handle_t  handle);
    int mktask_on_exit(mkit_handle_t  handle);
    int mktask_on_idle(mkit_handle_t  handle);

    /**
    * @brief  :default message to handle,user can capture this group of message!!
    *
    * @param  :handle,witch instance to control
    *
    * @warning:!!!event callback,without valid check,caller must guarantee param valid!!!
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_on_default(mkit_handle_t  handle,mkit_message_t *message);

    /**
    * @brief  :default API to register to service
    *
    * @param  :handle,witch instance to register
    *
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_do_register(mkit_handle_t  handle);

    /**
    * @brief  :default message poll
    *
    * @param  :handle,witch instance to control
    *
    * @warning:!!!event callback,without valid check,caller must guarantee param valid!!!
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mktask_do_poll(mkit_handle_t  handle,mkit_message_t *message,int timeout);

    /************************************************************************/
    /* APIs for service                                                     */
    /************************************************************************/
    /*
    * types
    */

    /*
    * manager APIs
    */
    /**
    * @brief  :use for user to init basic data of service framework
    *
    * @param  :master_key,user name ,would be anything!!
    *          reset,try to do reset of the framework
    *
    * @warning:for normal user ,do not care the params!
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int   mkservice_init(const char *master_key,int reset);

    /**
    * @brief  :use for user cleanup service data of framework
    *
    * @param  :none
    *
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int   mkservice_exit(void);

    /**
    * @brief  :use for retrieve service private data
    *
    * @param  :none
    *
    * @warning:!!!do NOT use the except 'launcher'!!!
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    void* mkservice_data(void);
    

    /*
    * client APIs
    */
    /**
    * @brief  :use to register server handle to system
    *
    * @param  :server,server handle
    *          id,service ID,must be 0 to 32
    *
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mkservice_register(mkit_handle_t server,int id);

    /**
    * @brief  :use to unregister server handle off system
    *
    * @param  :server,server handle
    *          id,service ID,must be 0 to 32
    *
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mkservice_unregister(mkit_handle_t server,int id);

    /**
    * @brief  :use to set data of service
    *
    * @param  :server,server handle
    *          id,service ID, must be 0 to 32
    *          index,data index,0 is default for handle saving
    *          value,data value
    *
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mkservice_set_data(mkit_handle_t server,int id,int index,unsigned long value);

    /**
    * @brief  :use to get data of service
    *
    * @param  :server,server handle
    *          id,service ID, must be 0 to 32
    *          index,data index,0 is default for handle saving
    *
    * @warning:!!APIs will NOT check params valid,user must guarantee validity!!
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    unsigned long mkservice_get_data(mkit_handle_t server,int id,int index);
    

    /**
    * @brief  :use to set syslog of a task
    *
    * @param  :caller   :caller handle
    *          task     :task name
    *          log_id   :log module id
    *          log_level:log level
    *          type     :log type, can be 0,1,2,3,set to '0' if you do NOT want to change
    *          file     :log file,just valid for type==DBG_LOG_TYPE_FILE
    *           
    *           for more detail ,please reference to stander file 'log.h'
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mkservice_req_syslog  (mkit_handle_t caller,const char *task,int log_id,int log_level,int type,const char*file);
    int mkservice_req_stats   (mkit_handle_t caller,const char *task,int breif_only);
    int mkservice_req_memcheck(mkit_handle_t caller,const char *task,int details,int do_free);
    int mkservice_req_reboot  (mkit_handle_t caller);
    
    /**
    * @brief  :watch service
    *
    * @param  :caller       ,who want to be watch by launcher
    *          startup_delay,count as seconds,this time need to startup for this task!
    * @warning:
    *
    * @return :0 as success,otherwise failed.
    * 
    */
    int mkservice_req_watch(mkit_handle_t caller,int startup_delay);
    int mkservice_del_watch(mkit_handle_t caller);


    /**
    * @brief  :notify service
    *
    * @param  :caller,max service target support
    *          notifier,whom provide the notification
    *          event,notify event
    *          count,notify counter,-1 as forever
    *
    * @warning: if notifier set to be '0',then notifier will be reset as service master 'launcher'!
    *           
    * @return :0 as success,otherwise failed.
    * 
    */
    static inline int mkservice_req_notify(mkit_handle_t caller,mkit_target_t notifier,int event,int count)
    {
        return mkit_notify_request(caller,notifier,event,count);
    }
    static inline int mkservice_del_notify(mkit_handle_t caller,mkit_target_t notifier,int event)
    {
        return mkit_notify_cancel(caller,notifier,event);
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* __MK_SERVICE_H__ */
