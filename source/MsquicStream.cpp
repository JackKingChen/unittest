#include "MsquicStream.h"

#include <algorithm>

MsQuicStream::MsQuicStream(const QUIC_API_TABLE *msQuic, const HQUIC &connection)
    : m_msQuic(msQuic),m_connection(connection)
{
}

MsQuicStream::~MsQuicStream(void)
{
}

QUIC_STATUS MsQuicStream::streamEventCallback(HQUIC stream,void* context,QUIC_STREAM_EVENT* event)
{
    MsQuicStream* pThis = (MsQuicStream*)context;

    switch (event->Type)
    {
    case QUIC_STREAM_EVENT_SEND_COMPLETE:
        if (event->SEND_COMPLETE.Canceled)
            printf("[strm][%p] Data cacneled\n", stream);
        return pThis->onSendComplete(event);
    case QUIC_STREAM_EVENT_RECEIVE:
        return pThis->onRecv(event);
        printf("[strm][%p],%p  Data received\n",stream, pThis->m_stream);
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
        printf("[strm][%p] Peer aborted\n", stream);
        pThis->m_msQuic->StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        printf("[strm][%p] Peer shut down\n", stream);
        break;
    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        printf("[strm][%p],%p All done\n", stream,pThis->m_stream);
        pThis->close();
        break;
    case QUIC_STREAM_EVENT_IDEAL_SEND_BUFFER_SIZE:
        break;
    default:
        break;
    }

    if (pThis->m_event)
        return pThis->m_event(event);

    return QUIC_STATUS_SUCCESS;
}

bool MsQuicStream::open(HQUIC stream, const func_stream_event& callback)
{
    QUIC_STATUS status;

    m_event = callback;

    if (stream==nullptr)
    {/*one peer started*/
        if (QUIC_FAILED(status = m_msQuic->StreamOpen(m_connection, QUIC_STREAM_OPEN_FLAG_NONE/*QUIC_STREAM_OPEN_FLAG_UNIDIRECTIONAL*/, streamEventCallback, this, &m_stream)))
        {
            printf("StreamOpen failed, 0x%x!\n", status);
            return false;
        }

        if (QUIC_FAILED(status = m_msQuic->StreamStart(m_stream, QUIC_STREAM_START_FLAG_NONE/*QUIC_STREAM_START_FLAG_IMMEDIATE*/)))
        {
            printf("StreamStart failed, 0x%x!\n", status);
            m_msQuic->StreamClose(m_stream);
            m_stream = nullptr;
            return false;
        }

        m_bClosed = false;
    }
    else
    {/*one peer accept*/
        m_stream = stream;
        m_msQuic->SetCallbackHandler(stream, (void*)streamEventCallback, this);
    }

    return true;
}

void MsQuicStream::close(void)
{
    std::lock_guard<std::mutex> guard(m_recvMutex);
    m_bClosed = true;
    if (m_stream)
    {
        m_msQuic->StreamClose(m_stream);
        m_stream = nullptr;
    }
    m_recvConn.notify_all();
}

QUIC_STATUS MsQuicStream::onSendComplete(QUIC_STREAM_EVENT *event)
{
    free(event->SEND_COMPLETE.ClientContext);
    if (event->SEND_COMPLETE.Canceled)
        printf("stream[%p]:send canceled:buffer=%p\n",m_stream,event);

    return QUIC_STATUS_SUCCESS;
}

bool MsQuicStream::send(const unsigned char* buffer, size_t length)
{
    QUIC_STATUS    status;
    QUIC_BUFFER   *quicHeader;
    unsigned char *quicBuffer;

    /*quicbuffer will free at send commplit event function*/
    quicBuffer = (uint8_t*)malloc(sizeof(QUIC_BUFFER) + length);

    quicHeader = (QUIC_BUFFER*)quicBuffer;
    quicHeader->Buffer = quicBuffer + sizeof(QUIC_BUFFER);
    quicHeader->Length = length;
    memcpy(quicHeader->Buffer,buffer,length);

    if (m_bClosed)
        return false;

    /*FIXME: stream may destory at stream event*/
    if (QUIC_FAILED(status = m_msQuic->StreamSend(m_stream, quicHeader, 1, QUIC_SEND_FLAG_NONE, quicHeader)))
    {
        printf("StreamSend failed, 0x%x!\n", status);
        free(quicBuffer);
        m_msQuic->ConnectionShutdown(m_connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
        return false;
    }

    //printf("### send %d\n",length);

    /*wait send finish or not ??*/

    return true;
}

QUIC_STATUS MsQuicStream::onRecv(QUIC_STREAM_EVENT *event)
{
    std::lock_guard<std::mutex> guard(m_recvMutex);

    /*FIXME: now not used flag to control the stream*/
    (void)event->RECEIVE.Flags;

    m_recvTotal = event->RECEIVE.TotalBufferLength;
    for (uint32_t i = 0; i < event->RECEIVE.BufferCount; ++i) {
        QuicBuffer quicBuffer;
        quicBuffer.buffer = event->RECEIVE.Buffers[i].Buffer;
        quicBuffer.length = event->RECEIVE.Buffers[i].Length;
        quicBuffer.offset = 0;
        m_recvBufferQueue.push_back(quicBuffer);
    }
    //printf(".\n");

    m_recvConn.notify_all();

    return QUIC_STATUS_PENDING;
}

int  MsQuicStream::recv(unsigned char* buffer, size_t length, int timeout)
{
    int     retval   = 0;
    size_t  nRecvMax = length;

    std::cv_status status = std::cv_status::no_timeout;

    std::unique_lock<std::mutex> lock(m_recvMutex);

    while (m_recvBufferQueue.empty() && !m_bClosed) {
        if (timeout<0)
            m_recvConn.wait(lock);
        else
            status = m_recvConn.wait_for(lock,std::chrono::milliseconds(timeout));
    }

    if (m_bClosed)
        return -2;

    if (status == std::cv_status::timeout)
        return -1;

    while (!m_recvBufferQueue.empty() && nRecvMax>0)
    {
        QuicBuffer  quicBuffer = m_recvBufferQueue.front();
        size_t      nRecvBytes = std::min(quicBuffer.length-quicBuffer.offset,nRecvMax);

        memcpy(buffer+retval,quicBuffer.buffer+quicBuffer.offset,nRecvBytes);
        nRecvMax          -= nRecvBytes;
        retval            += nRecvBytes;
        quicBuffer.offset += nRecvBytes;

        m_recvBufferQueue.pop_front();

        if (quicBuffer.offset!=quicBuffer.length)
        {
            m_recvBufferQueue.push_front(quicBuffer);
        }
    }

    if (m_recvBufferQueue.empty()){
        m_msQuic->StreamReceiveComplete(m_stream,m_recvTotal);
    }

    return retval;
}

int MsQuicStream::waitingBytes(int timeout)
{
    int nHave = getOnRecvLen();

    if (nHave==0)
    {
        std::unique_lock<std::mutex> lock(m_recvMutex);

        while (m_recvBufferQueue.empty()) {
            std::cv_status status = m_recvConn.wait_for(lock,std::chrono::milliseconds(timeout));
            if (status==std::cv_status::timeout)
                return false;

            nHave = getOnRecvLen();
        }
    }

    return nHave;
}

int  MsQuicStream::getOnRecvLen(void)
{
    int retval = 0;
    std::lock_guard<std::mutex> guard(m_recvMutex);

    for (auto& it : m_recvBufferQueue)
        retval += it.length-it.offset;

    return retval;
}

////////////////////////////////////////////////////////////////////////////

int  MsQuicSelect(MsQuicStream* stream[], int8_t *readflags, size_t count, int delay)
{
    bool bAvailable = false;

    for (size_t i=0; i<count; i++)
    {
        if (stream[i])
        {
            int len = stream[i]->getOnRecvLen();
            if (len>0) {
                bAvailable = true;
                readflags[i]=1;
            }
        }

    }

    if (!bAvailable)
    {/*no data,waiting;FIXME: should poll*/
        for (size_t i=0; i<count; i++)
        {
            if (stream[i])
                stream[i]->waitingBytes(delay);
        }
    }

    return 0;
}
