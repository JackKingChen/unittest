#ifndef MSQUICSTREAM_H
#define MSQUICSTREAM_H

#include "msquic/msquic.h"

#include <functional>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <atomic>

using func_stream_event  = std::function<QUIC_STATUS(QUIC_STREAM_EVENT* event)>;

class MsQuicStream
{
    friend class MsQuicClient;
    friend class MsQuicServer;

public:
    bool  send(const unsigned char* buffer, size_t length);
    int   recv(unsigned char* buffer, size_t length, int timeout);

    int   getOnRecvLen(void);
    int   waitingBytes(int timeout);

private:
    /*event*/
    QUIC_STATUS  onSendComplete(QUIC_STREAM_EVENT* event);
    QUIC_STATUS  onRecv(QUIC_STREAM_EVENT* event);

private:
    MsQuicStream(const QUIC_API_TABLE* msQuic, const HQUIC& connection);
    ~MsQuicStream(void);

    /*one peer open stream,other peer get*/
    bool open(HQUIC stream,const func_stream_event& callback);
    void close(void);

    static QUIC_STATUS streamEventCallback(HQUIC stream,void* context,QUIC_STREAM_EVENT* event);

    struct QuicBuffer
    {
        uint8_t* buffer;
        size_t   length;
        size_t   offset;
    };

    bool                            m_bClosed   = false;
    int                             m_recvTotal = 0;
    std::mutex                      m_recvMutex;
    std::deque<QuicBuffer>          m_recvBufferQueue;
    std::condition_variable         m_recvConn;

    const QUIC_API_TABLE* m_msQuic;
    const HQUIC           m_connection;
    func_stream_event     m_event;

    HQUIC                 m_stream = nullptr;
};

int  MsQuicSelect(MsQuicStream* stream[], int8_t *readflags, size_t count, int delay);

#endif // MSQUICSTREAM_H
