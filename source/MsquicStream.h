#ifndef MSQUICSTREAM_H
#define MSQUICSTREAM_H

#include "msquic.h"

#include <functional>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <atomic>

using func_stream_event  = std::function<QUIC_STATUS(HQUIC stream, void* context, QUIC_STREAM_EVENT* event)>;

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

//#ifdef WIN32
#if 1
#define LOGE(format,...)  printf("MsQuicClient:Error:" format,##__VA_ARGS__)
#define LOGI(format,...)  printf("MsQuicClient:INFO:" format,##__VA_ARGS__)
#else
#include <android/log.h>
static void LOGI(const char* format, ...)//  __android_log_vprint(ANDROID_LOG_INFO, "MsQuicClient", format, ##__VA_ARGS__);
{
    char strBuffer[1024];
    va_list args;
    va_start(args, format);
    __android_log_vprint(ANDROID_LOG_INFO, "MsQuicClient", format, args);
    va_end(args);
}
static void LOGE(const char* format, ...)//  __android_log_vprint(ANDROID_LOG_INFO, "MsQuicClient", format, ##__VA_ARGS__);
{
    char strBuffer[1024];
    va_list args;
    va_start(args, format);
    __android_log_vprint(ANDROID_LOG_INFO, "MsQuicClient", format, args);
    va_end(args);
}
#endif

#endif // MSQUICSTREAM_H
