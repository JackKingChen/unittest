/********************************************************************
*
*    DESCRIPTION:Copyright(c) 2010-2020 Xiamen Yealink Network Technology Co,.Ltd
*
*    AUTHOR: cxf@yealink.com
*
*    HISTORY:
*
*    DATE:2013-01-14
*
*******************************************************************/
//sock.h sock.cpp: v1.0.0.0: just define overlaped client sock interface

#include "StdAfx.h"
#include <afxsock.h>
#include "socket.h"

Socket::Socket(void) : m_sock(0)
{
    strcpy_s(m_hostname,sizeof(m_hostname),"?\0");
    ZeroMemory(&m_sendOverlapped,sizeof(m_sendOverlapped));
    ZeroMemory(&m_rcvOverlapped,sizeof(m_rcvOverlapped));
}
Socket::~Socket(void)
{
    if (m_sock)
        closesocket(m_sock);

    m_sock = 0;
}

char* Socket::Create(int af, int type, int proto, BOOL bOverLapped, const char* ipaddr, int port)
{
    unsigned char flag;

    flag = bOverLapped ? WSA_FLAG_OVERLAPPED : 0;

    m_bOverlapped = bOverLapped;
    m_family      = af;
    m_socktype    = type;
    m_protocol    = proto;

    /*close first*/
    if (m_sock!=0 && m_sock != INVALID_SOCKET)
        closesocket(m_sock);

    m_sock     = WSASocket(af, type, proto, 0, 0,flag);

    if (m_sock == INVALID_SOCKET)
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Create()]", GetError());
        return m_errMsg;
    }

    memset(&m_remoteAddr, 0x0, sizeof(struct sockaddr_in));
    m_remoteAddr.sin_family      = m_family;
    m_remoteAddr.sin_addr.s_addr = inet_addr(ipaddr);
    m_remoteAddr.sin_port        = htons(port);
#if 1
    ZeroMemory(&m_sendOverlapped,sizeof(m_sendOverlapped));
    ZeroMemory(&m_rcvOverlapped,sizeof(m_rcvOverlapped));

    m_sendOverlapped.hEvent = WSACreateEvent();
    if (m_sendOverlapped.hEvent == NULL)
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s WSACreateEvent ]", GetError());
        return m_errMsg;
    }

    m_rcvOverlapped.hEvent = WSACreateEvent();
    if (m_rcvOverlapped.hEvent == NULL)
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s WSACreateEvent ]", GetError());
        return m_errMsg;
    }
#endif
    return NULL;
}

char* Socket::Destroy()
{
    if (m_sock!=0 && m_sock != INVALID_SOCKET)
        closesocket(m_sock);

    m_sock = 0;

    return NULL;
}

char* Socket::Bind(int port)
{
    this->m_port = port;
    m_localAddr.sin_family = m_family;
    m_localAddr.sin_port = htons(port);
    m_localAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    if (bind(m_sock, (sockaddr *)&m_localAddr, sizeof(m_localAddr)))
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Listen() bind ]", GetError());
        return m_errMsg;
    }

    return NULL;
}

char* Socket::Select(HWND handle, UINT msg, long events)
{
    if (WSAAsyncSelect(m_sock, handle, msg, events))
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Select()]", GetError());
        return m_errMsg;
    }

    return NULL;
}

/************************************************************************/
/* get/set config                                                       */
/************************************************************************/
int  Socket::GetRecvBuffLen()
{
    unsigned int rcvBufLen;
    int      Len;
    int      nErrCode;

    Len = sizeof(rcvBufLen);
    nErrCode= getsockopt(m_sock, SOL_SOCKET, SO_SNDBUF,(char*)&rcvBufLen, &Len);
    if (SOCKET_ERROR == nErrCode)
    {
        return -1;
    }

    return rcvBufLen;
}

BOOL Socket::SetRecvBuffLen(int bufLen)
{
    int      nErrCode;

    nErrCode = setsockopt(m_sock, SOL_SOCKET, SO_SNDBUF,(char*)&bufLen,sizeof(bufLen));
    if (SOCKET_ERROR == nErrCode)
    {
        return FALSE;
    }

    /*make sure*/
    if (GetRecvBuffLen()!=bufLen)
    {
        return FALSE;
    }

    return TRUE;
}

/************************************************************************/
/* TCP                                                                  */
/************************************************************************/
char*Socket::Connect(const char *addr, WORD port)
{
    char* err;

    this->m_port = port;
    m_remoteAddr.sin_family = m_family;
    m_remoteAddr.sin_port = htons(port);

    if ((m_remoteAddr.sin_addr.S_un.S_addr = inet_addr(addr)) == INADDR_NONE)
    {
        hostent *host = gethostbyname(addr);
        if (host)
        {
            m_remoteAddr.sin_addr = *((in_addr *)host->h_addr_list[0]);
            strcpy_s(m_hostname,sizeof(m_hostname), host->h_name);
        }
        else
        {
            sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Connect() gethostbyname ]", GetError());
            return m_errMsg;
        }
    } 
    else
    {
        getnameinfo((sockaddr *)&m_remoteAddr, sizeof(m_remoteAddr), m_hostname, NI_MAXHOST, 0, 0, NI_NAMEREQD);
    }

    if (WSAConnect(m_sock, (sockaddr *)&m_remoteAddr, sizeof(m_remoteAddr), 0, 0, 0, 0))
    {
        err = GetError();
        if (err!=NULL)
        {
            sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::WSAConnect() WSAConnect ]", err);
            return m_errMsg;
        }
    }

    return NULL;
}

char* Socket::Disconnect()
{
    if (m_sock)
    {
        WSACloseEvent(m_sendOverlapped.hEvent);
        WSACloseEvent(m_rcvOverlapped.hEvent);

        if (closesocket(m_sock))
        {
            sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Disconnect() closesocket ]", GetError());
            return m_errMsg;
        }
        else
        {
            m_sock = 0;
            return NULL;
        }
    }

    return "zero socket?";
}

char* Socket::Recv(void* rcvData,int maxLen,int& retLen)
{
    int retval;

    /*get read data length*/
    if (ioctlsocket(m_sock, FIONREAD,(DWORD*)&retLen))
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Read() ioctlsocket ]", GetError());
        return m_errMsg;
    }

    if (retLen > maxLen)
    {
        return "Recv buffer is so smaller";
    }

    retval = recv(m_sock, (char*)rcvData, retLen, 0);
    if (retval == 0)
        return "connection have been closed...";
    if (retval == SOCKET_ERROR)
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Read() recv ]", GetError());
        return m_errMsg;
    }

    return NULL;
}

char* Socket::Send(void* sndData,int length)
{
    int  retval;

    retval = send(m_sock, (const char*)sndData, length, 0);
    if (retval == SOCKET_ERROR)
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Send() send ]", GetError());
        return m_errMsg;
    }

    return NULL;
}

char* Socket::RecvEx(char* rcvData,int maxLen,int& retLen)
{
    WSABUF wsaBuf;
    DWORD  flags;
    int    retval;

    flags = 0;

#if 0
    /*get read data length*/
    if (ioctlsocket(m_sock, FIONREAD,(DWORD*)&retLen))
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Read() ioctlsocket ]", GetError());
        return m_errMsg;
    }

    if (retLen > maxLen)
    {
        return "Recv buffer is so smaller";
    }
#endif

    if (!BytesWaiting())
    {
        return "BytesWaiting failed";
    }

    wsaBuf.buf = rcvData;
    wsaBuf.len = maxLen;

    retval = WSARecv(m_sock,&wsaBuf,1,(LPDWORD)&retLen,&flags,&m_rcvOverlapped,/*RecvCompletionROUTINE*/NULL);

    if (retval == SOCKET_ERROR)
    {
        if (WSAGetLastError()==WSA_IO_PENDING)
        {
            DWORD flags;

            WaitForSingleObject(m_sendOverlapped.hEvent,INFINITE);
            //WSAWaitForMultipleEvents(1,&m_rcvOverlapped.hEvent,TRUE,INFINITE,TRUE);

            /*make sure send finished*/
            retval = WSAGetOverlappedResult(m_sock, &m_rcvOverlapped, (LPDWORD)&retLen, FALSE, &flags);
            if (retval != FALSE)
            {
                sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Send() send ]", GetError());
                return m_errMsg;
            }
            return NULL;
        }
    }

    return NULL;
}

char* Socket::SendEx(void* sndData,int len)
{
    int   sndRealLen;
    int   retval;
    int   sndLen;
    WSABUF wsaBuf;

    wsaBuf.buf = (char*)sndData;
    wsaBuf.len = len;

    retval = WSASend(m_sock,&wsaBuf,1,(LPDWORD)&sndRealLen,0,&m_sendOverlapped,/*SendCompletionROUTINE*/NULL);

    if (retval == SOCKET_ERROR)
    {
        if (WSAGetLastError()==WSA_IO_PENDING)
        {
            DWORD flags;

            WaitForSingleObject(m_sendOverlapped.hEvent,INFINITE);
            //WSAWaitForMultipleEvents(1,&m_sendOverlapped.hEvent,TRUE,INFINITE,TRUE);

            /*make sure send finished*/
            retval = WSAGetOverlappedResult(m_sock, &m_sendOverlapped, (LPDWORD)&sndLen, FALSE, &flags);
            if (retval == FALSE)
            {
                sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Send() send ]", GetError());
                return m_errMsg;
            }

            return NULL;
        }
        
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Send() send ]", GetError());
        return m_errMsg;
    }

    if (sndRealLen != len)
    {
        //AfxMessageBox("sndRealLen != sndLen");
    }

    return NULL;
}

/************************************************************************/
/* UDP                                                                  */
/************************************************************************/
char* Socket::SendTo(const char* data, int len, int& send_bytes)
{
    send_bytes = sendto(m_sock, data, len, 0, 
        (struct sockaddr*)&m_remoteAddr, sizeof(struct sockaddr));

    if (send_bytes < 0)
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Send() send ]", GetError());
        return m_errMsg;
    }

    return NULL;
}

char* Socket::RecvFrom(char* rcvData,int maxLen,int& retLen)
{
    int  retval;
    int  cAddrLen;

    cAddrLen = sizeof(m_cltSockAddr);

    /*get read data length*/
    if (ioctlsocket(m_sock, FIONREAD,(DWORD*)&retLen))
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Read() ioctlsocket ]", GetError());
        return m_errMsg;
    }

    retLen = retLen < maxLen ? retLen : maxLen;
#if 0
    if (retLen > maxLen)
    {
        return "Recv buffer is so smaller";
    }
#endif
    retval = recvfrom (m_sock, rcvData, retLen, 0, (struct sockaddr *)&m_cltSockAddr, &cAddrLen);
    if (retval == SOCKET_ERROR)
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s", GetError());
        return m_errMsg;
    }

    return NULL;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

BOOL Socket::BytesWaiting(void)
{
    fd_set read_fds;
    int    retval;
    int    max_fd;

    while (1)
    {
        max_fd = 0;
        FD_ZERO(&read_fds);
        FD_SET(m_sock, &read_fds);

        max_fd = m_sock+1;

        retval = select(max_fd, &read_fds, NULL, NULL, NULL);
        if (retval<0)
        {
            return FALSE;
        }

        if (FD_ISSET(m_sock,&read_fds))
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL Socket::BytesWaiting(DWORD dwBytes,int& dwTimeout)
{
    DWORD retLen;

    while(1)
    {
        if (ioctlsocket(m_sock, FIONREAD,(DWORD*)&retLen))
        {
            return FALSE;
        }
        if (retLen<dwBytes)
        {
            BytesWaiting();
        }
    }

    return TRUE;
}

char* Socket::WaitSendCommlete(DWORD sndLen)
{
    DWORD flags;
    int   retval;

    WSAWaitForMultipleEvents(1,&m_sendOverlapped.hEvent,TRUE,INFINITE,TRUE);

    /*make sure send finished*/
    retval = WSAGetOverlappedResult(m_sock, &m_sendOverlapped, &sndLen, FALSE, &flags);
    if (retval == FALSE)
    {
        sprintf_s(m_errMsg,sizeof(m_errMsg), "Error - %s - in [Socket::Send() send ]", GetError());
        return m_errMsg;
    }

    WSAResetEvent(m_sendOverlapped.hEvent);

    return NULL;
}

void CALLBACK Socket::RecvCompletionROUTINE(
                                    IN DWORD dwError, 
                                    IN DWORD cbTransferred, 
                                    IN LPWSAOVERLAPPED lpOverlapped, 
                                    IN DWORD dwFlags
                                    )
{
    //WSAResetEvent(m_rcvOverlapped.hEvent);
}
void CALLBACK Socket::SendCompletionROUTINE(
                                    IN DWORD dwError, 
                                    IN DWORD cbTransferred, 
                                    IN LPWSAOVERLAPPED lpOverlapped, 
                                    IN DWORD dwFlags
                                    )
{
   // WSAResetEvent(m_sendOverlapped.hEvent);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
char* Socket::GetError()
{
     switch (WSAGetLastError()) {
        case WSAEINTR:
            return "Interrupted function call.";
        case WSAEACCES:
            return "Permission denied.";
        case WSAEFAULT:
            return "Bad address.";
        case WSAEINVAL:
            return "Invalid argument.";
        case WSAEMFILE:
            return "Too many open files.";
        case WSAEWOULDBLOCK:
            return 0;
        case WSAEINPROGRESS:
            return "Operation now in progress.";
        case WSAEALREADY:
            return "Operation already in progress.";
        case WSAENOTSOCK:
            return "Socket operation on nonsocket.";
        case WSAEDESTADDRREQ:
            return "Destination address required.";
        case WSAEMSGSIZE:
            return "Message too long.";
        case WSAEPROTOTYPE:
            return "Protocol wrong type for socket.";
        case WSAENOPROTOOPT:
            return "Bad protocol option.";
        case WSAEPROTONOSUPPORT:
            return "Protocol not supported.";
        case WSAESOCKTNOSUPPORT:
            return "Socket type not supported.";
        case WSAEOPNOTSUPP:
            return "Operation not supported.";
        case WSAEPFNOSUPPORT:
            return "Protocol family not supported.";
        case WSAEAFNOSUPPORT:
            return "Address family not supported by protocol family.";
        case WSAEADDRINUSE:
            return "Address already in use.";
        case WSAEADDRNOTAVAIL:
            return "Cannot assign requested address.";
        case WSAENETDOWN:
            return "Network is down.";
        case WSAENETUNREACH:
            return "Network is unreachable.";
        case WSAENETRESET:
            return "Network dropped connection on reset.";
        case WSAECONNABORTED:
            return "Software caused connection abort.";
        case WSAECONNRESET:
            return "Connection reset by peer.";
        case WSAENOBUFS:
            return "No buffer space available.";
        case WSAEISCONN:
            return "Socket is already connected.";
        case WSAENOTCONN:
            return "Socket is not connected.";
        case WSAESHUTDOWN:
            return "Cannot send after socket shutdown.";
        case WSAETIMEDOUT:
            return "Connection timed out.";
        case WSAECONNREFUSED:
            return "Connection refused.";
        case WSAEHOSTDOWN:
            return "Host is down.";
        case WSAEHOSTUNREACH:
            return "No route to host.";
        case WSAEPROCLIM:
            return "Too many processes.";
        case WSASYSNOTREADY:
            return "Network subsystem is unavailable.";
        case WSAVERNOTSUPPORTED:
            return "Winsock.dll version out of range.";
        case WSANOTINITIALISED:
            return "Successful WSAStartup not yet performed.";
        case WSAEDISCON:
            return "Graceful shutdown in progress.";
        case WSATYPE_NOT_FOUND:
            return "Class type not found.";
        case WSAHOST_NOT_FOUND:
            return "Host not found.";
        case WSATRY_AGAIN:
            return "Nonauthoritative host not found.";
        case WSANO_RECOVERY:
            return "This is a nonrecoverable error.";
        case WSANO_DATA:
            return "Valid name, no data record of requested type.";
        case WSA_INVALID_HANDLE:
            return "Specified event object handle is invalid.";
        case WSA_INVALID_PARAMETER:
            return "One or more parameters are invalid.";
        case WSA_IO_INCOMPLETE:
            return "Overlapped I/O event object not in signaled state.";
        case WSA_IO_PENDING:
            return "Overlapped operations will complete later.";
        case WSA_NOT_ENOUGH_MEMORY:
            return "Insufficient memory available.";
        case WSA_OPERATION_ABORTED:
            return "Overlapped operation aborted.";
        case WSASYSCALLFAILURE:
            return "System call failure.";
        default:
            return "Unknown code?";
    }
}
