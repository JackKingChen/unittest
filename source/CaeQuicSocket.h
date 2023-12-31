
#pragma once

#include <string>
#include <future>
#include <queue>
#include <mutex>
#include <map>
#include <functional>

#include "msquic.h"

class CaeQuicSocket
{
public:
    CaeQuicSocket(const HQUIC& stream, const std::function<int(uint8_t* data, int len)>& eventHander);
    ~CaeQuicSocket(void);

    int  Send(void* pkt, size_t size);
    int  Recv(void* pkt, size_t size);
    void CloseSocket(void);

private:
    QUIC_STATUS  onRecv(QUIC_STREAM_EVENT* event);
    QUIC_STATUS  onSendComplete(QUIC_STREAM_EVENT* event);
    QUIC_STATUS  onStreamEventCallback(HQUIC stream, QUIC_STREAM_EVENT* event);

private:
    static std::map<HQUIC, CaeQuicSocket*> s_mapStreams;
    static std::mutex                      s_streamMutex;
    static QUIC_STATUS streamEventCallback(HQUIC stream, void* context, QUIC_STREAM_EVENT* event);

    std::atomic<bool>               m_bClosed{ false };

    uint8_t*   m_buffer = nullptr;
    int        m_buflen = 0;
    int        m_datalen= 0;

    std::function<int(uint8_t* data, int len)>  m_eventHandle;


    HQUIC        m_stream = nullptr;
};