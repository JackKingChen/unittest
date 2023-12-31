
#include "unittest.h"

#include "CaeQuicSocket.h"
#include "CaeQuicServer.h"

#include "cloudphonenetperf.h"

#include <future>


CaeQuicSocket* skt = nullptr;
std::future<void> testrecvHandler;

CaeQuicServer server("transtest", 9070);

UNITTEST(server_start)
{
    if (server.init("/data/CAE/cloudAppEngine/CAE_server.crt", "/data/CAE/cloudAppEngine/CAE_server.key"))
    {
        server.start(
            [&](HQUIC stream) {
                printf("connected,skt=%p\n", skt);
                skt = new CaeQuicSocket(stream);
                CloudPhoneNetPerfServer* perfsvr = new CloudPhoneNetPerfServer(
                    [&](void* data, size_t len) {
                        return skt->Send(data, len);
                    },
                    [&](void* data, size_t len) {
                        return skt->Recv(data, len);
                    }
                );

                perfsvr->start();

#if 0
                testrecvHandler = std::async(std::launch::async, [&]() {
                    while (true) {
                        char msg[1024] = { 0 };
                        if (skt->Recv(msg, 1024) > 0)
                            printf("receive %s\n", msg);
                        else {
                            printf("receive failed");
                            return;
                        }
                    }     
                });
#endif
                
            });
    }
    else
    {
        OSTTY::print(OSTTY::RED, "server init failed\n");
    }
}

UNITTEST(server_send)
{
    if (argc < 0) {
        skt->Send((void*)argv[0], strlen(argv[0]) + 1);
    }
}

UNITTEST(server_close)
{
    skt->CloseSocket();
}

UNITTEST(server_stop) {
    server.exit();
}