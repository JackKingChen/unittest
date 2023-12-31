
#include "unittest.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define SERV_PORT   9070

UNITTEST(udpserver)
{
    /* sock_fd --- socket�ļ������� ����udp�׽���*/
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        return;
    }

    /* ���׽��ֺ�IP���˿ڰ� */
    struct sockaddr_in addr_serv;
    int len;
    memset(&addr_serv, 0, sizeof(struct sockaddr_in));  //ÿ���ֽڶ���0���
    addr_serv.sin_family = AF_INET; //ʹ��IPV4��ַ
    addr_serv.sin_port = htons(SERV_PORT); //�˿�
    /* INADDR_ANY��ʾ�������ĸ��������յ����ݣ�ֻҪĿ�Ķ˿���SERV_PORT���ͻᱻ��Ӧ�ó�����յ� */
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);  //�Զ���ȡIP��ַ
    len = sizeof(addr_serv);

    /* ��socket */
    if (bind(sock_fd, (struct sockaddr*)&addr_serv, sizeof(addr_serv)) < 0)
    {
        perror("bind error:");
        return;
    }


    int recv_num;
    int send_num;
    char send_buf[20] = "i am server!";
    char recv_buf[20];
    struct sockaddr_in addr_client;

    while (1)
    {
        printf("server wait:\n");

        recv_num = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&addr_client, (socklen_t*)&len);

        if (recv_num < 0)
        {
            perror("recvfrom error:");
            return;
        }

        recv_buf[recv_num] = '\0';
        printf("server receive %d bytes: %s\n", recv_num, recv_buf);

        send_num = sendto(sock_fd, send_buf, recv_num, 0, (struct sockaddr*)&addr_client, len);

        if (send_num < 0)
        {
            perror("sendto error:");
            return;
        }
    }

    close(sock_fd);
}