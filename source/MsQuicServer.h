
#pragma once

#include <string>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "msquic.h"
#include "msquicstream.h"

class MsQuicServer
{
public:
    MsQuicServer(const std::string& name, unsigned short port);
    ~MsQuicServer(void);

    bool  init(const std::string& strCertFile, const std::string& strKeyFile);
    bool  init(const std::string& strHash);
    void  exit(void);

    bool  start(void);
    void  stop(void);

    MsQuicStream* accept(const func_stream_event& callback=nullptr);
    void destoryStream(const MsQuicStream* stream);

private:
    /*event*/
    QUIC_STATUS onPeerStreamStarted(HQUIC connection, HQUIC stream);


    bool    serverLoadConfiguration(const char* Cert,const char* KeyFile);

    bool    serverLoadConfiguration(const QUIC_CERTIFICATE_HASH& certHash);

    static  QUIC_STATUS serverStreamCallback(HQUIC stream,void* contex,QUIC_STREAM_EVENT* event);
    static  QUIC_STATUS serverConnectionCallback(HQUIC connection,void* contex,QUIC_CONNECTION_EVENT* event);
    static  QUIC_STATUS serverListenCallback(HQUIC listener,void* contex,QUIC_LISTENER_EVENT* event);

    /*config*/
    const std::string              m_svrName;
    const uint16_t                 m_udpPort = 4567;
    QUIC_REGISTRATION_CONFIG       m_regConfig;
    func_stream_event              m_eventcallback = nullptr;
    static const QUIC_API_TABLE*   s_msQuicApi;

    std::queue<MsQuicStream*>      m_streamEventQueue;
    std::mutex                     m_streamMutex;
    std::condition_variable        m_streamCondi;

    HQUIC   m_listener      = nullptr;
    HQUIC   m_registration  = nullptr;
    HQUIC   m_configuration = nullptr;
};
