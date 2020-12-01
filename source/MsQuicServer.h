
#pragma once

#include <string>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "msquic.h"
#include "MsquicStream.h"

class MsQuicServer
{
public:
    MsQuicServer(const std::string& name, unsigned short port, const QUIC_REGISTRATION_CONFIG& regConfig);
    ~MsQuicServer(void);

    bool  init(const std::string& strCertFile, const std::string& strKeyFile);
    void  exit(void);

    void  start(void);
    void  stop(void);

    MsQuicStream* accept(const func_stream_event& callback=nullptr);
    void destoryStream(const MsQuicStream* stream);

private:
    /*event*/
    QUIC_STATUS onPeerStreamStarted(HQUIC connection, HQUIC stream);


    bool    serverLoadConfiguration(const char* Cert,const char* KeyFile);

    static  QUIC_STATUS serverStreamCallback(HQUIC stream,void* contex,QUIC_STREAM_EVENT* event);
    static  QUIC_STATUS serverConnectionCallback(HQUIC connection,void* contex,QUIC_CONNECTION_EVENT* event);
    static  QUIC_STATUS serverListenCallback(HQUIC listener,void* contex,QUIC_LISTENER_EVENT* event);

    /*config*/
    const std::string              m_svrName;
    const uint16_t                 m_udpPort = 4567;
    const QUIC_REGISTRATION_CONFIG m_regConfig  { "quicsample", QUIC_EXECUTION_PROFILE_LOW_LATENCY };
    func_stream_event              m_eventcallback = nullptr;
    static const QUIC_API_TABLE*   s_msQuicApi;

    std::queue<MsQuicStream*>      m_streamEventQueue;
    std::mutex                     m_streamMutex;
    std::condition_variable        m_streamCondi;

    HQUIC   m_registration;
    HQUIC   m_configuration;

    bool               m_bKeepRunning = true;;
    std::future<bool>  m_asyncFuture;
};
