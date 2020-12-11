
#include "unittest.h"

#include <signal.h>
#include <memory>
#include <string>
#include <iostream>
#include <future>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "lexical_cast.hpp"
#include "headers.h"
#include "NdHardLink.h"
#include "NdVideoSession.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <signal.h>
#include <execinfo.h>

#define UNITTEST_CMD_DATA_SEPART  0


enum {
	NDSTREAM_PROTOCOL_TCP,
	NDSTREAM_PROTOCOL_UDP,
	NDSTREAM_PROTOCOL_RUDP,
	NDSTREAM_PROTOCOL_QUIC,
	NDSTREAM_PROTOCOL_MULTICAST,
	NDSTREAM_PROTOCOL_KCP,
	NDSTREAM_PROTOCOL_UDPRAW
};

const int DEF_WIDTH             = 1920;
const int DEF_HEIGHT            = 1080;
const float DEF_FRAMERATE       = 30.0f;
const int DEF_MAX_PAYLOAD_SIZE  = 64000;
const int DEF_PROTOCOL          = NDSTREAM_PROTOCOL_TCP;
const std::string DEF_REMOTE_ADDRESS = "192.168.4.203";
const int DEF_REMOTE_PORT       = 23230;

enum
{
    ND_STATUS_PROTOC_DISCONNECTED,
    ND_STATUS_PROTOC_CONNECED,
    ND_STATUS_CASTING_STATED,
    ND_STATUS_CASTINNG_STOPPED
};

class ClientContext
{
public:
    int                                 m_sockProto;
    std::string                         m_serverIp;
    std::string                         m_videoCfg;
    std::string                         m_audioCfg;
    std::string                         m_ctrlCfg;
    int                                 m_videoSock;
    int                                 m_audioSock;
    std::future<void>                   m_videoAsyncHandle;
    bool                                m_flags;
    NdVideoSession                      *m_videoSess;
    std::mutex                          m_lock;
    std::atomic<int>                    m_status;

};

/**
 * @brief 视频通信信道
 * 
 */
bool client_video_channel(ClientContext *cls, const std::string& remote_addr, int remote_port)
{
    cls->m_videoSock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr_serv;
    addr_serv.sin_addr.s_addr = inet_addr(remote_addr.c_str());
    addr_serv.sin_family      = AF_INET;
    addr_serv.sin_port        = htons(remote_port);

    int buflen = 0;
    setsockopt (cls->m_videoSock,SOL_SOCKET, SO_SNDBUF, &buflen, sizeof(buflen));

    auto conn_status = connect(cls->m_videoSock, (sockaddr *)&addr_serv, sizeof(addr_serv));
    if(conn_status == 0)
    {
        std::cout << "[video_channel][ socket " << cls->m_videoSock <<"] connect server ok" << std::endl;
        
        return true;
    }
    else{
        return false;
    }
}

/**
 * @brief 音频通信信道
 * 
 */
bool client_audio_channel(ClientContext *cls,  const std::string& remote_addr, int remote_port)
{
    cls->m_audioSock          = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr_serv;
    addr_serv.sin_addr.s_addr = inet_addr(remote_addr.c_str());
    addr_serv.sin_family      = AF_INET;
    addr_serv.sin_port        = htons(remote_port);

    auto conn_status = connect(cls->m_videoSock, (sockaddr *)&addr_serv, sizeof(addr_serv));
    if(conn_status == 0)
    {
        std::cout << "[video_channel]" << " connect server ok" << std::endl;
        return true;
    }
    else{
        return false;
    }
}


/**
 * @brief 信令通信信道
 * 
 */
bool client_request_connect(ClientContext *cls,int& nVideoPort)
{
    char ucBuffer[4096] = {0};
    int iBytes = 0;
    /*send*/
    std::string strJson = "{\
                      \"action\":\"connect_req\",\
                      \"client_id\":\"asdfasdf\", \
                      \"msg\": \"connect request\",\
                      \"client_type\": \"pc\",\
                      \"client_version\": \"1.0\",\
                      \"video\": [\
                          {\
                              \"width\": 1080,\
                              \"height\": 1920,\
                              \"frame_rate\": 30,\
                              \"baud_rate\": 10080,\
                              \"transport_type\": 1\
                          }\
                      ],\
                      \"audio\": [],\
                      \"control\": []\
                  }";
    
    int length = strJson.length()+1;

    ucBuffer[0] = (length & 0xFF000000) >> 24;
    ucBuffer[1] = (length & 0x00FF0000) >> 16;
    ucBuffer[2] = (length & 0x0000FF00) >> 8;
    ucBuffer[3] = length & 0x000000FF;
    memcpy(ucBuffer+4,strJson.c_str(),strJson.length()+1);

    std::lock_guard<std::mutex> sklock(cls->m_lock);
    if (::send(cls->m_sockProto,ucBuffer,length+4,0)<0)
    {
        printf("send failed\n");
        return false;
    }
    
    return true;
}

bool client_start_casting(ClientContext *cls)
{
    char ucBuffer[4096] = {0};
    int iBytes = 0;

    std::string strJson = "{\
    \"action\":\"startcasting_req\",\
	\"client_id\":\"123\"}";

    int length = strJson.length()+1;
    unsigned long size = 0; // TODO

    ucBuffer[0] = (length & 0xFF000000) >> 24;
    ucBuffer[1] = (length & 0x00FF0000) >> 16;
    ucBuffer[2] = (length & 0x0000FF00) >> 8;
    ucBuffer[3] = length & 0x000000FF;
    memcpy(ucBuffer+4,strJson.c_str(),strJson.length()+1);

    std::lock_guard<std::mutex> sklock(cls->m_lock);
    if (::send(cls->m_sockProto,ucBuffer,length+4,0)<0)
    {
        printf("send failed:%d(%s)\n",errno,strerror(errno));
        return false;
    }

    return true;
}
bool client_stop_casting(ClientContext *cls)
{
    char ucBuffer[4096] = {0};
    int iBytes = 0;

    std::string strJson = "{\
    \"action\":\"stopcasting_req\",\
	\"client_id\":\"123\"}";

    int length = strJson.length()+1;

    ucBuffer[0] = (length & 0xFF000000) >> 24;
    ucBuffer[1] = (length & 0x00FF0000) >> 16;
    ucBuffer[2] = (length & 0x0000FF00) >> 8;
    ucBuffer[3] = length & 0x000000FF;
    memcpy(ucBuffer+4,strJson.c_str(),strJson.length()+1);

    std::lock_guard<std::mutex> sklock(cls->m_lock);
    if (::send(cls->m_sockProto,ucBuffer,length+4,0)<0)
    {
        printf("send failed:%d(%s)\n",errno,strerror(errno));
        return false;
    }
    return true;
}

static ClientContext *s_cls = nullptr;

bool client_protoc_channel(ClientContext *cls, std::string strIpAddr, int nPort)
{
    int 	nRequestSocket = -1;
    int 	nIpAddr        = 0;
    struct  sockaddr_in server_addr;
    struct  sockaddr_in tSin;
    char 	cIp[24];

    if((nRequestSocket=socket(AF_INET,SOCK_STREAM,0)) == -1)
        printf("Ln:%d[%s] Socket Error:%s\a\n", __LINE__, __FILE__, strerror(errno));

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(nPort);
    inet_pton(AF_INET, strIpAddr.c_str(), &nIpAddr);
    server_addr.sin_addr.s_addr = nIpAddr;

    if(::connect(nRequestSocket, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr))<0 && errno!=EINPROGRESS)
    {
        fprintf(stdout,"Ln:%d[%s] Connect err(%d)(%s)\n", __LINE__, __FILE__, errno, strerror(errno));
        close(nRequestSocket);
        return false;
    }

    cls->m_sockProto = nRequestSocket;

    return true;
}

void client_protoc_heartbeart(ClientContext *cls)
{
    char ucBuffer[4096] = {0};
    std::string strJson = "{\"action\":\"heartbeat_req\"}";
    int length = strJson.length()+1;
    ucBuffer[0] = (length & 0xFF000000) >> 24;
    ucBuffer[1] = (length & 0x00FF0000) >> 16;
    ucBuffer[2] = (length & 0x0000FF00) >> 8;
    ucBuffer[3] = length & 0x000000FF;
    memcpy(ucBuffer+4,strJson.c_str(),strJson.length()+1);

    while (true)
    {
        std::unique_lock<std::mutex> sklock(cls->m_lock,std::defer_lock);
        if (sklock.try_lock())
        {
            if (::send(s_cls->m_sockProto,ucBuffer,length+4,0)<0)
                printf("send %s failed:errno=%d(%s)\n",strJson.c_str(),errno,strerror(errno));
            sklock.unlock();
        }

        sleep(1);
    }
}

void client_protoc_event(ClientContext *cls)
{
    while (true)
    {
        struct timeval timeout={3,0};
        struct fd_set fds; 
        char buffer[2048] = {0};
        int  nRecv;

        FD_ZERO(&fds); 
        FD_SET(cls->m_sockProto,&fds); 

        if (select(cls->m_sockProto+1,&fds,NULL,NULL,&timeout) < 0)
        {
            fprintf(stderr,"protoc select failed,%d %d(%s)\n",nRecv,errno,strerror(errno));
            break;
        }

        
        if (FD_ISSET(cls->m_sockProto,&fds))
        {
            if ((nRecv = recv(cls->m_sockProto, buffer, 2048, 0)) <= 0) 
            {
                fprintf(stderr,"protoc recv failed,%d %d(%s)\n",nRecv,errno,strerror(errno));
                break;
            }

            printf("%s\n",buffer+4);

            /*do event,just parse as common string*/
            {
                std::string  strAction;
                std::string  strTmp = strstr(buffer+4,"\"action\":\"")+strlen("\"action\":\"");

                int  nPos = strTmp.find("\"");
                printf("%d\n",nPos);
                if (nPos != std::string::npos)
                {
                    strAction = strTmp.substr(0,nPos);
                    printf("strAction=%s\n",strAction.c_str());

                    if (strAction=="connect_resp")
                    {
                        char* szTmp        = strstr(buffer+4,"\"video_listen_port\":\"");
                        std::string strPort= szTmp+strlen("\"video_listen_port\":\"");

                        int  nVideoPort = atoi(strPort.c_str());

                        client_video_channel(cls,cls->m_serverIp,nVideoPort);

                        s_cls->m_status = ND_STATUS_PROTOC_CONNECED;

                        printf("video_listen_port=%d\n",nVideoPort);
                    }
                    else if (strAction=="startcasting_resp")
                    {
                        std::cout << "startcasting_resp receving" << std::endl;

                        char* szTmp         = strstr(buffer+4,"\"error_code\":\"");
                        std::string strCode = szTmp+strlen("\"error_code\":\"");
                        std::string strWidth;
                        std::string strHeight;

                        szTmp         = strstr(buffer+4,"\"width\":\"");
                        strWidth      = szTmp+strlen("\"width\":\"");
                        szTmp         = strstr(buffer+4,"\"height\":\"");
                        strHeight     = szTmp+strlen("\"height\":\"");

                        int nWidth = atoi(strWidth.c_str());
                        int nHeight= atoi(strHeight.c_str());
                        OSTTY::print(OSTTY::RED,"casting widht=%d,height=%d\n",nWidth,nHeight);

                        int  nCode = atoi(strCode.c_str());

                        if(cls->m_videoSess && nCode==0)
                        {
                            s_cls->m_status = ND_STATUS_CASTING_STATED;
                        #if !UNITTEST_CMD_DATA_SEPART
                            cls->m_videoSess->init(/*DEF_WIDTH, DEF_HEIGHT*/nWidth,nHeight,30,90000.0f,DEF_MAX_PAYLOAD_SIZE,cls->m_videoSock);
                            cls->m_videoSess->start();
                        #endif
                        }
                        else
                        {
                            printf("startcasting_resp: ncode=%d\n",nCode);
                        }
                    }
                    else if (strAction=="stopcasting_resp")
                    {
                        bool result = false;

                        char* szTmp        = strstr(buffer+4,"\"error_code\":\"");
                        std::string strCode= szTmp+strlen("\"error_code\":\"");

                        int  nCode = atoi(strCode.c_str());

                        if(cls->m_videoSess && nCode==0) 
                        {
                        #if !UNITTEST_CMD_DATA_SEPART
                            result = cls->m_videoSess->stop();
                        #endif
                            s_cls->m_status = ND_STATUS_CASTINNG_STOPPED;
                        }
                        else
                        {
                            printf("stopcasting_resp: ncode=%d\n",nCode);
                        }
                        
                    }
                    else
                    {
                        printf("strAction=%s do nothing\n",strAction.c_str());
                    }
                }
            }
        } 
    }
}

UNITTEST(nd_connect)
{
    std::string strIp = DEF_REMOTE_ADDRESS;
    int nPort         = DEF_REMOTE_PORT;
    if (argc==1)
    {
        strIp = argv[0];
    }

    printf("connect server %s:%d\n",strIp.c_str(),nPort);

    /*do connect*/
    if (s_cls==nullptr)
    {
        s_cls              = new ClientContext();
        s_cls->m_videoSess = new NdVideoSession();
    }

    s_cls->m_serverIp = strIp;

    if (client_protoc_channel(s_cls,strIp,nPort))
    {
        printf("protocol connect server %s:%d success, and do send heart beart\n",strIp.c_str(),nPort);

        std::thread heartBeart(client_protoc_heartbeart,s_cls);
        heartBeart.detach();
        std::thread eventProcess(client_protoc_event,s_cls);
        eventProcess.detach();

        s_cls->m_status = ND_STATUS_PROTOC_CONNECED;

        /*connect successed, send connect request*/
        if (!client_request_connect(s_cls,nPort))
        {
            printf("client connect faild\n");
            delete s_cls->m_videoSess;
            delete s_cls;
            s_cls->m_videoSess = nullptr;
            s_cls              = nullptr;

            return;
        }
        //printf("Ln:%d[%s] Enter\r\n", __LINE__, __FUNCTION__);
    }
    else
    {
        printf("protocol connect server %s:%d faild\n",strIp.c_str(),nPort);
        delete s_cls->m_videoSess;
        delete s_cls;
        s_cls->m_videoSess = nullptr;
        s_cls              = nullptr;
    }

}

UNITEST(nd_disconnect)
{
#if 0
    if (s_cls)
    {
        if (s_cls->m_videoSess)
            delete s_cls->m_videoSess;
        delete s_cls;
    }
#endif    
}

#if !UNITTEST_CMD_DATA_SEPART
UNITTEST(nd_start)
{
    if (s_cls==nullptr || s_cls->m_status==ND_STATUS_PROTOC_DISCONNECTED)
    {
        printf("not connectd,run \"nd_connect ipaddress\" first\n");
        return;
    }

    client_start_casting(s_cls);
}

UNITTEST(nd_stop)
{
    if (s_cls==nullptr || s_cls->m_status==ND_STATUS_PROTOC_DISCONNECTED)
    {
        printf("not connectd,run \"nd_connect ipaddress\" first\n");
        return;
    }
    client_stop_casting(s_cls);
}
#else
UNITTEST(nd_start_cmd)
{
    if (s_cls==nullptr || s_cls->m_status==ND_STATUS_PROTOC_DISCONNECTED)
    {
        printf("not connectd,run \"nd_connect ipaddress\" first\n");
        return;
    }

    client_start_casting(s_cls);
}

UNITTEST(nd_stop_cmd)
{
    if (s_cls==nullptr || s_cls->m_status==ND_STATUS_PROTOC_DISCONNECTED)
    {
        printf("not connectd,run \"nd_connect ipaddress\" first\n");
        return;
    }
    client_stop_casting(s_cls);
}

UNITTEST(nd_stop_chain)
{
    if (s_cls==nullptr || s_cls->m_status==ND_STATUS_PROTOC_DISCONNECTED)
    {
        printf("not connectd,run \"nd_connect ipaddress\" first\n");
        return;
    }
    s_cls->m_videoSess->stop();
}

UNITTEST(nd_start_chain)
{
    if (s_cls==nullptr || s_cls->m_status==ND_STATUS_PROTOC_DISCONNECTED)
    {
        printf("not connectd,run \"nd_connect ipaddress\" first\n");
        return;
    }
    s_cls->m_videoSess->init(DEF_WIDTH, DEF_HEIGHT,30,90000.0f,DEF_MAX_PAYLOAD_SIZE,s_cls->m_videoSock);
    s_cls->m_videoSess->start();
}
#endif

UNITTEST(nd_status)
{
    switch (s_cls->m_status.load())
    {
    case ND_STATUS_PROTOC_DISCONNECTED:
        printf("connected\n");
        break;
    case ND_STATUS_PROTOC_CONNECED:
        printf("disconnected\n");
        break;
    case ND_STATUS_CASTING_STATED:
        printf("started\n");
        break;
    case ND_STATUS_CASTINNG_STOPPED:
        printf("stopped");
        break;
    }
}

