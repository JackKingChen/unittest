
#include "unittest.h"

#include "msquicstream.h"
#include "msquicclient.h"
#include "msquicserver.h"

#include "cloudphonenetperf.h"

#include <thread>

UNITTEST(quicclient)
{
    MsQuicClient  client("transtest");
    MsQuicStream* pStream = nullptr;
    char buffer[128] = { 0 };
    int  peroid = 1000;/*us*/

    if (client.connect("116.63.189.108", 20242))
    {
        OSTTY::print(OSTTY::GREEN, "waiting connect ...\n");
        if (client.waitForConnected(5000))
            OSTTY::print(OSTTY::GREEN, "connected\n");
        else {
            OSTTY::print(OSTTY::RED, "connected timeout\n");
            //return;
        }
            

        pStream = client.createStream(nullptr);

        CloudPhoneNetPerfClient* pclient = new CloudPhoneNetPerfClient([&](void* data, size_t len) {
            return pStream->send((const unsigned char*)data, len);
            },
            [&](void* data, size_t len) {
                return pStream->recv((unsigned char*)data, len, -1);
            }
            );
        //pclient->test1(100,10);
        //pclient->test2(100, 10);
        pclient->test3(100, 100);

        std::this_thread::sleep_for(std::chrono::milliseconds(10000000));
#if 0
        for (int i = 0;; i++) {
            int retval = 0;
            sprintf(buffer, "%d: number(%d),%d subtle intervals\n", i, i, peroid);
            retval = pStream->send((unsigned char*)buffer, strlen(buffer) + 1);
            printf("%s", buffer);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
#endif
    }
}