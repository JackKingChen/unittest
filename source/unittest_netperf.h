#pragma once

typedef struct PerData
{
    uint64_t  tmsend; /*����ʱ���*/
    uint64_t  tmrecv; /*����ʱ���*/
    int       msgtype;/*��������*/
    int       id;
    int       datalen;
    char      data[0];
}PerData;

void perf_send()
{

}

void perf_recv()
{

}

void perf_send_recv()
{

}

UNITTEST(tcpperf)
{

}

UNITTEST(quicperf)
{

}

