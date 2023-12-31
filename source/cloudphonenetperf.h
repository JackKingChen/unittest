
#pragma once

#include <future>
#include <functional>

typedef struct NetPerfHeader
{
    uint64_t  tmsend;
    uint64_t  tmrecv;
#define NETPERF_HEADER_MAGIC 0xA3B3E5F6
    uint32_t  magic;
    int       msgtype;//0,1,2
    int       id;
    int       datalen;
}NetPerfHeader;

class CloudPhoneNetPerfServer
{
public:
    CloudPhoneNetPerfServer(const std::function<int(void*, size_t)>& fnsend,
        const std::function<int(void*, size_t)>& fnrecv);

    ~CloudPhoneNetPerfServer(void);

    void start(void);
    void stop(void);

private:
    bool                               m_bKeepRunning = false;
    std::future<void>                  m_asyncHandle;
    std::function<int(void*,size_t)>  m_funSend = nullptr;
    std::function<int(void*, size_t)> m_funRecv = nullptr;
};

class CloudPhoneNetPerfClient
{
public:
    CloudPhoneNetPerfClient(const std::function<int(void*, size_t)>& fnsend,
        const std::function<int(void*, size_t)>& fnrecv);
    ~CloudPhoneNetPerfClient(void);

    void test(int type, int times, int peroid);

    void test1(int times, int peroid);
    void test2(int times, int peroid);
    void test3(int times, int peroid);

private:
    std::function<int(void*, size_t)>  m_funSend = nullptr;
    std::function<int(void*, size_t)>  m_funRecv = nullptr;
};
