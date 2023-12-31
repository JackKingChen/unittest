
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

    bool  send(const unsigned char* buffer, size_t length);
    int   recv(unsigned char* buffer, size_t length, int timeout);

    /*connect*/
    bool  connect(const char* target, unsigned short port, const func_connect_event& callback = nullptr);
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
    func_connect_event       m_connectCallback = nullptr;

    static QUIC_STATUS clientConnectionCallback(HQUIC Connection, void* Context, QUIC_CONNECTION_EVENT* event);


    const QUIC_API_TABLE* m_msQuic = nullptr;
    HQUIC            m_registration = nullptr;
    HQUIC            m_configration = nullptr;
    HQUIC            m_connection = nullptr;
    HQUIC            m_streamConn = nullptr;
};