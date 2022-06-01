/********************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: ChenXF
*
*    HISTORY:
*
*    DATE:
*******************************************************************/
//sock.h sock.cpp: v1.0.0.0: just define overlaped client sock interface
//sock.h sock.cpp: v1.0.0.1: add overlaped server sock interface
//sock.h sock.cpp: v1.0.0.2: add wait function
//sock.h sock.cpp: v1.0.0.3: add udp send and differ remote and local addr

#pragma once

#include <afxsock.h>

class Socket
{
public:
    Socket(void);
    ~Socket(void);

    /*
    * operation:if error happen return error info,other return NULL
    */
    char*   Create(int af, int type, int proto, BOOL bOverLapped = TRUE,const char* ipaddr="", int port=0);
    char*   Select(HWND handle, UINT msg, long events);
    char*   Connect(const char *addr, WORD port);
    char*   Disconnect(void);
    char*   Recv(void* rcvData,int maxLen,int& retLen);
    char*   Send(void* sndData,int length);
    char*   RecvEx(char* rcvData,int maxLen,int& retLen);
    char*   SendEx(void* sndData,int len);
    char*   SendTo(const char* data, int len, int& send_bytes);
    char*   RecvFrom(char* rcvData,int maxLen,int& retLen);
    char*   GetError(void);
    char*   Bind(int port);
    char*   Destroy();

    int     GetRecvBuffLen();
    BOOL    SetRecvBuffLen(int bufLen);

    DWORD   GetStatus();

    void    SetEventMask();
    unsigned int GetEventMask();

    BOOL    BytesWaiting();
    BOOL    BytesWaiting(DWORD dwBytes,int& dwTimeout);

    char*   WaitEvent(unsigned int mask);
    char*   OnCompletion(DWORD dwErrorCode, DWORD dwCount, LPOVERLAPPED lpOverlapped);
    char*   WaitSendCommlete(DWORD sndLen);

protected:
    static void CALLBACK RecvCompletionROUTINE(
        IN DWORD dwError, 
        IN DWORD cbTransferred, 
        IN LPWSAOVERLAPPED lpOverlapped, 
        IN DWORD dwFlags
        );
    static void CALLBACK SendCompletionROUTINE(
        IN DWORD dwError, 
        IN DWORD cbTransferred, 
        IN LPWSAOVERLAPPED lpOverlapped, 
        IN DWORD dwFlags
        );

protected:
    HANDLE     m_hComm;
    BOOL       m_bOverlapped;
    OVERLAPPED m_sendOverlapped;
    OVERLAPPED m_rcvOverlapped;

public:
    sockaddr_in m_cltSockAddr;             /*client address for udp server*/

public:
    SOCKET m_sock;

private:
    char   m_errMsg[256];                  /*restore socket error message*/

    /*sock param*/
    sockaddr_in m_remoteAddr;               /*address to which get connected*/
    sockaddr_in m_localAddr;
    char        m_hostname[NI_MAXHOST];     /*host name*/

    int      m_family;
    int      m_socktype;
    int      m_protocol;
    int      m_port;

};

