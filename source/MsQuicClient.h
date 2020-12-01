
#pragma once

#include <mutex>
#include <condition_variable>
#include <string>
#include <functional>

#include "msquic.h"

#include "MsquicStream.h"

using func_connect_event = std::function<QUIC_STATUS(QUIC_CONNECTION_EVENT* event)>;

class MsQuicClient
{
public:
    MsQuicClient(const std::string& name);
    ~MsQuicClient(void);

    /*connect*/
    bool  connect(const char* target, unsigned short port, const func_connect_event& callback = nullptr);
    bool  waitForConnected(int timeout=10000/*ms*/ );

    /*after connected,create stream for communication*/
    MsQuicStream* createStream(const func_stream_event& callback=nullptr);
    void          destoryStream(const MsQuicStream* stream);

private:
    /*connection*/
    bool                     m_bConnected       = false;
    bool                     m_bConnectedFailed = false;
    std::mutex               m_connMutex;
    std::condition_variable  m_condConn;
    func_connect_event       m_connectCallback = nullptr;

    static QUIC_STATUS clientConnectionCallback(HQUIC Connection,void* Context ,QUIC_CONNECTION_EVENT* event);

    std::string               m_name;
    QUIC_REGISTRATION_CONFIG  m_regConfig;

    const QUIC_API_TABLE  *m_msQuic = nullptr;
    HQUIC            m_registration = nullptr;
    HQUIC            m_configration = nullptr;
    HQUIC            m_connection   = nullptr;
    HQUIC            m_streamConn   = nullptr;
    
};
