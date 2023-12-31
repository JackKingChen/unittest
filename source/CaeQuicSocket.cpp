
#include "CaeQuicSocket.h"

extern const QUIC_API_TABLE* s_msQuicApi;

std::map<HQUIC, CaeQuicSocket*> CaeQuicSocket::s_mapStreams;
std::mutex                      CaeQuicSocket::s_streamMutex;

CaeQuicSocket::CaeQuicSocket(const HQUIC& stream, const std::function<int(uint8_t* data, int len)>& eventHander) : 
    m_stream(stream), m_eventHandle(eventHander)
{
    s_msQuicApi->SetCallbackHandler(stream, (void*)streamEventCallback, this);
    {
        std::lock_guard<std::mutex> guard(s_streamMutex);
        s_mapStreams[stream] = this;
    }
}
CaeQuicSocket::~CaeQuicSocket(void)
{
    if (m_buffer) {
        free(m_buffer);
        m_buffer = nullptr;
    }
}

int  CaeQuicSocket::Send(void* pkt, size_t size)
{
    QUIC_STATUS    status;
    QUIC_BUFFER*   quicHeader;
    unsigned char* quicBuffer;

    /*buffer will free @onSendComplete function*/
    quicBuffer = (uint8_t*)malloc(sizeof(QUIC_BUFFER) + size);

    quicHeader = (QUIC_BUFFER*)quicBuffer;
    quicHeader->Buffer = quicBuffer + sizeof(QUIC_BUFFER);
    quicHeader->Length = size;
    memcpy(quicHeader->Buffer, pkt, size);

    if (m_bClosed)
        return -1;

    if (QUIC_FAILED(status = s_msQuicApi->StreamSend(m_stream, quicHeader, 1, QUIC_SEND_FLAG_NONE/*QUIC_SEND_FLAG_ALLOW_0_RTT*/, quicHeader)))
    {
        printf("StreamSend failed, close, 0x%x!\n", status);
        free(quicBuffer);
        CloseSocket();
        return -1;
    }

    return size;
}

int  CaeQuicSocket::Recv(void* pkt, size_t size) {
    return 0;
}

void CaeQuicSocket::CloseSocket(void)
{
    m_bClosed = true;
    m_recvConn.notify_all();
    s_msQuicApi->StreamShutdown(m_stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
}

QUIC_STATUS  CaeQuicSocket::onSendComplete(QUIC_STREAM_EVENT* event) {
    free(event->SEND_COMPLETE.ClientContext);

    if (event->SEND_COMPLETE.Canceled)
        printf("stream[%p]:send canceled:buffer=%p\n", m_stream, event);

    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS CaeQuicSocket::streamEventCallback(HQUIC stream, void* context, QUIC_STREAM_EVENT* event) {
    CaeQuicSocket* pThis = (CaeQuicSocket*)context;
    return pThis->onStreamEventCallback(stream, event);
}

QUIC_STATUS CaeQuicSocket::onRecv(QUIC_STREAM_EVENT* event)
{
    if (m_buffer == nullptr) {
        m_buflen = 256 * 1024;
        m_buffer = (uint8_t*)malloc(m_buflen);
        
        if (m_buffer == nullptr) {
            printf("malloc failed\n");
            return QUIC_STATUS_SUCCESS;
        }
    }

    if (event->RECEIVE.TotalBufferLength > m_buflen - m_datalen) {
        uint8_t* tmp = realloc(m_buffer, event->RECEIVE.TotalBufferLength*2);
        if (tmp == nullptr) {
            printf("realloc failed\n");
            return QUIC_STATUS_SUCCESS;
        }
        m_buflen = event->RECEIVE.TotalBufferLength * 2;
        m_buffer = tmp;
    }

    for (uint32_t i = 0; i < event->RECEIVE.BufferCount; ++i) {
        memcpy(m_buffer + m_datalen, event->RECEIVE.Buffers[i].Buffer, event->RECEIVE.Buffers[i].Length);
        m_datalen += event->RECEIVE.Buffers[i].Length;
    }

    int handsize = m_eventHandle(m_buffer, m_datalen);

    if (handsize > 0) {
        m_datalen -= handsize;
        memmove(m_buffer, m_buffer + handsize, m_datalen);
    }
    
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS CaeQuicSocket::onStreamEventCallback(HQUIC stream, QUIC_STREAM_EVENT* event)
{
    switch (event->Type) {
    case QUIC_STREAM_EVENT_SEND_COMPLETE:
        printf("[strm][%p] Data sent\n", stream);
        free(event->SEND_COMPLETE.ClientContext);
        break;
    case QUIC_STREAM_EVENT_RECEIVE:
        printf("[strm][%p] Data received\n", stream);
        return onRecv(event);
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        printf("[strm][%p] Peer shut down\n", stream);
        s_msQuicApi->StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
    case QUIC_STREAM_EVENT_PEER_RECEIVE_ABORTED:
        printf("[strm][%p] Peer aborted\n", stream);
        s_msQuicApi->StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
        break;
    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        printf("[strm][%p] All done\n", stream);
        s_msQuicApi->StreamClose(stream);
        {
            std::lock_guard<std::mutex> guard(s_streamMutex);
            auto it = s_mapStreams.find(stream);
            if (it != s_mapStreams.end()) {
                delete it->second;
                s_mapStreams.erase(it);
            }
        }
        break;
    default:
        break;
    }

    return QUIC_STATUS_SUCCESS;
}