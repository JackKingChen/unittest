
#include "unittest.h"


#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#pragma pack(push)
#pragma pack(4)

typedef struct NDWakupMsg
{
#define  NDWakupMsg_MAGINC   0x135AD46F
    unsigned int  magic;
    unsigned int  data;
}NDWakupMsg_t;

#pragma pack(pop)

UNITTEST(poweron)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr_serv;
    addr_serv.sin_addr.s_addr = inet_addr("192.168.4.1");
    addr_serv.sin_family      = AF_INET;
    addr_serv.sin_port        = htons(11777);

    int buflen = 0;
    setsockopt (sockfd,SOL_SOCKET, SO_SNDBUF, &buflen, sizeof(buflen));

    auto conn_status = connect(sockfd, (sockaddr *)&addr_serv, sizeof(addr_serv));
    if(conn_status == 0)
    {        
        NDWakupMsg msg;
        msg.magic = NDWakupMsg_MAGINC;
        msg.data  = 1;
        ::send(sockfd,&msg,sizeof(NDWakupMsg),0);
    }

    close(sockfd);
}

UNITTEST(poweroff)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr_serv;
    addr_serv.sin_addr.s_addr = inet_addr("192.168.4.1");
    addr_serv.sin_family      = AF_INET;
    addr_serv.sin_port        = htons(11777);

    int buflen = 0;
    setsockopt (sockfd,SOL_SOCKET, SO_SNDBUF, &buflen, sizeof(buflen));

    auto conn_status = connect(sockfd, (sockaddr *)&addr_serv, sizeof(addr_serv));
    if(conn_status == 0)
    {
        printf("connect succsssed\n");
        NDWakupMsg msg;
        msg.magic = NDWakupMsg_MAGINC;
        msg.data  = 0;
        printf("send1\n");
        ::send(sockfd,&msg,sizeof(NDWakupMsg),0);
        printf("send2\n");
    }

    close(sockfd);
}
