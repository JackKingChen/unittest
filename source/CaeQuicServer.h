
#pragma once

#include <string>
#include <future>
#include <map>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "msquic.h"
#include "CaeQuicSocket.h"

class CaeQuicServer
{
public:
    CaeQuicServer(const std::string& name, unsigned short port);
    ~CaeQuicServer(void);

    bool  init(const std::string& strCertFile, const std::string& strKeyFile);
    void  exit(void);

    bool  start(const std::function<void(HQUIC)>& funcConnCb);
    void  stop(void);

private:
    QUIC_STATUS onConnectionCallback(HQUIC connection, QUIC_CONNECTION_EVENT* event);

    bool    serverLoadConfiguration(const char* Cert, const char* KeyFile);
    static  QUIC_STATUS serverConnectionCallback(HQUIC connection, void* contex, QUIC_CONNECTION_EVENT* event);
    static  QUIC_STATUS serverListenCallback(HQUIC listener, void* contex, QUIC_LISTENER_EVENT* event);

    /*config*/
    const std::string              m_svrName;
    const uint16_t                 m_udpPort = 9070;
    QUIC_REGISTRATION_CONFIG       m_regConfig;

    std::function<void(HQUIC)>     m_connectedCallback = nullptr;

    HQUIC   m_listener = nullptr;
    HQUIC   m_registration = nullptr;
    HQUIC   m_configuration = nullptr;
};
