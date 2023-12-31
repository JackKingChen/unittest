
#include "cloudphonenetperf.h"

#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>

#define TEST_BUF_BIGSIZE  (4096)

CloudPhoneNetPerfServer::CloudPhoneNetPerfServer(const std::function<int(void*, size_t)>& fnsend,
    const std::function<int(void*, size_t)>& fnrecv)
{
    m_funSend = fnsend;
    m_funRecv = fnrecv;
}

CloudPhoneNetPerfServer::~CloudPhoneNetPerfServer(void) 
{

}

void CloudPhoneNetPerfServer::start(void)
{
    m_bKeepRunning = true;
    m_asyncHandle = std::async(std::launch::async, [&]() {
        int      maxlen = 128 * 1024;
        uint8_t* maxbuf = (uint8_t*)malloc(maxlen);
        if (maxbuf == nullptr)
            return;

        while (m_bKeepRunning) {
            NetPerfHeader* header = (NetPerfHeader*)maxbuf;
            uint8_t* data = maxbuf + sizeof(NetPerfHeader);
            printf("##recv:%p,%d\n", header, sizeof(NetPerfHeader));
            int recvlen = m_funRecv(header, sizeof(NetPerfHeader));
            if (recvlen != sizeof(NetPerfHeader)) {
                printf("recv heaer %d != %zd\n", recvlen, sizeof(NetPerfHeader));
                return;
            }
            if (header->magic != NETPERF_HEADER_MAGIC) {
                printf("magic wrong\n");
                return;
            }

            int      datalen = header->datalen-sizeof(NetPerfHeader);
            if (datalen > maxlen) {
                printf("datalen too long\n");
                return;
            }
            int      total = 0;
            while (total < datalen) {
                printf("##recv:%p,%d\n", data + total, datalen - total);
                recvlen = m_funRecv(data + total, datalen - total);
                if (recvlen < 0) {
                    printf("recv failed, break;");
                    return;
                }
                total += recvlen;
            }
#if 0
            if (header->msgtype == 0) {
                printf("##send:%p,%d\n", maxbuf, header->datalen);
                m_funSend(maxbuf, header->datalen);
            }
            else {
                header->datalen = TEST_BUF_BIGSIZE;
                m_funSend(maxbuf, TEST_BUF_BIGSIZE);
            }
#endif
        }
        });
}
void CloudPhoneNetPerfServer::stop(void)
{
    m_bKeepRunning = false;
    m_asyncHandle.get();
}

CloudPhoneNetPerfClient::CloudPhoneNetPerfClient(const std::function<int(void*, size_t)>& fnsend,
    const std::function<int(void*, size_t)>& fnrecv)
{
    m_funSend = fnsend;
    m_funRecv = fnrecv;
}
CloudPhoneNetPerfClient::~CloudPhoneNetPerfClient(void)
{

}

void CloudPhoneNetPerfClient::test(int type, int times, int peroid)
{
    std::vector<uint64_t>  statistic(times, 0);
    uint8_t* buffer = (uint8_t*)malloc(TEST_BUF_BIGSIZE);
    int      maxlen = TEST_BUF_BIGSIZE;
    int      dlen   = sizeof(NetPerfHeader);

    if (buffer == nullptr)
        return;

    NetPerfHeader* header = (NetPerfHeader*)buffer;
    if (type == 0) {
        header->msgtype = 0;
        dlen = sizeof(NetPerfHeader);
    }
    else if (type == 2) {
        header->msgtype = 0;
        dlen            = TEST_BUF_BIGSIZE;
    } 
    else {
        header->msgtype = 1;
        dlen            = TEST_BUF_BIGSIZE;
    }
        
    

    for (int i = 0; i < times; i++) {
        header->id      = i;
        header->tmsend  = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        header->tmrecv  = 0;
        header->datalen = dlen;
        header->magic = NETPERF_HEADER_MAGIC;
        /*send*/
        printf("##send:%p,%d\n", buffer, dlen);
        m_funSend(buffer, dlen);
#if 0
        /*recv*/
        printf("##recv:%p,%d\n", header, sizeof(NetPerfHeader));
        int recvlen = m_funRecv(header, sizeof(NetPerfHeader));
        if (recvlen != sizeof(NetPerfHeader)) {
            printf("recv heaer %d != %zd\n", recvlen, sizeof(NetPerfHeader));
            return;
        }
        if (header->magic != NETPERF_HEADER_MAGIC) {
            printf("magic wrong\n");
            return;
        }
        
        uint8_t* data = buffer + sizeof(NetPerfHeader);
        int   datalen = header->datalen - sizeof(NetPerfHeader);
        if (datalen > maxlen - sizeof(NetPerfHeader)) {
            printf("datalen too long\n");
            return;
        }
        int   total = 0;
        while (total < datalen) {
            printf("##recv:%p,%d\n", data + total, datalen - total);
            recvlen = m_funRecv(data + total, datalen - total);
            if (recvlen < 0) {
                printf("recv failed, break;");
                return;
            }
            total += recvlen;
        }
#endif
        header->tmrecv = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        statistic[i] = (header->tmrecv - header->tmsend) / 2;
    }

    uint64_t sum = std::accumulate(std::begin(statistic), std::end(statistic), 0);
    uint64_t average = sum / statistic.size();
    uint64_t maxtm = *max_element(statistic.begin(), statistic.end());
    uint64_t mintm = *min_element(statistic.begin(), statistic.end());

    printf("result:max=%lld,min=%lld,avg=%lld\n", maxtm, mintm, average);
}

void CloudPhoneNetPerfClient::test1(int times, int peroid)
{
    return test(0, times, peroid);
    
}
void CloudPhoneNetPerfClient::test2(int times, int peroid)
{
    return test(1, times, peroid);
}
void CloudPhoneNetPerfClient::test3(int times, int peroid)
{
    return test(2, times, peroid);
}
