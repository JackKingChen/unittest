#pragma once

typedef struct PerData
{
    uint64_t  tmsend; /*发送时间戳*/
    uint64_t  tmrecv; /*接收时间戳*/
    int       msgtype;/*数据类型*/
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

