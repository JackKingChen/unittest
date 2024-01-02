
#pragma once

#include <mutex>
#include <condition_variable>
#include <string>
#include <functional>

#include "msquic.h"

class CaeQuicClient
{
public:
    CaeQuicClient(const std::string& strName);
    ~CaeQuicClient(void);


    /*connect*/
    bool  connect(const char* target, unsigned short port);
    bool  waitForConnected(int timeout = 10000/*ms*/);

private:
    struct QuicBuffer
    {
        uint8_t* buffer;
        size_t   length;
        size_t   offset;
    };


    /*connection*/
    bool                     m_bConnected = false;
    bool                     m_bConnectedFailed = false;
    std::mutex               m_connMutex;
    std::condition_variable  m_condConn;

    static QUIC_STATUS clientConnectionCallback(HQUIC Connection, void* Context, QUIC_CONNECTION_EVENT* event);


    const QUIC_API_TABLE* m_msQuic = nullptr;
    HQUIC            m_registration = nullptr;
    HQUIC            m_configration = nullptr;
    HQUIC            m_connection = nullptr;
    HQUIC            m_streamConn = nullptr;
};