
#include "unittest.h"

#include "msquicstream.h"
#include "msquicclient.h"
#include "msquicserver.h"

UNITTEST(quicserver)
{
    MsQuicServer server("transtest", 9070);

    if (server.init("/data/CAE/cloudAppEngine/CAE_server.crt", "/data/CAE/cloudAppEngine/CAE_server.key"))
    {
        server.start();
        MsQuicStream* stream = server.accept(nullptr);

        OSTTY::print(OSTTY::GREEN, "accept stream=%p\n", stream);

        while (true) {
            char buffer[128] = { 0 };
            int   len = 0;
            len = stream->recv((unsigned char*)buffer, 128, 1000);

            if (len > 0)
                printf("%s", buffer);
        }
    }
    else
    {
        OSTTY::print(OSTTY::RED, "server init failed\n");
    }
}