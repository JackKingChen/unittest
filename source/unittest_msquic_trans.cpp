
#include "unittest.h"


#include "MsquicStream.h"
#include "MsQuicClient.h"
#include "MsQuicServer.h"

#define TEST_PORT        5678

UNITTEST(sendNumber)
{
    MsQuicClient  client("transtest");
    MsQuicStream* pStream  = nullptr;
    char buffer[128] = { 0 };
    int  peroid = 1000;/*us*/

    if (client.connect("127.0.0.1",TEST_PORT))
    {
        OSTTY::print(OSTTY::GREEN,"waiting connect ...\n");
        if (client.waitForConnected(5000))
            OSTTY::print(OSTTY::GREEN,"connected\n");
        else
            OSTTY::print(OSTTY::RED,"connected timeout\n");

        pStream = client.createStream(nullptr);

        for (int i = 0;;i++) {
            int retval = 0;
            sprintf(buffer,"%d: number(%d),%d subtle intervals\n", i,i,peroid);
            retval = pStream->send((unsigned char*)buffer,strlen(buffer)+1);
            printf("%s",buffer);
            usleep(peroid);
        }
    }
}

UNITTEST(recvNumber)
{
    const QUIC_REGISTRATION_CONFIG RegConfig = { "quicsample", QUIC_EXECUTION_PROFILE_LOW_LATENCY };
    MsQuicServer server("transtest",TEST_PORT,RegConfig);

    if (server.init("msqucitest.crt","msqucitest.priv.key"))
    {
        server.start();
        MsQuicStream* stream = server.accept(nullptr);

        OSTTY::print(OSTTY::GREEN,"accept stream=%p\n",stream);

        while (true) {
            char buffer[128] = { 0 };
            int   len = 0;
            len = stream->recv((unsigned char*)buffer,128,1000);

            if (len>0)
                printf("%s",buffer);
        }
    }
    else
    {
        OSTTY::print(OSTTY::RED,"server init failed\n");
    }
}

UNITTEST(sendFile)
{
    std::string strFile;
    if (argc<1)
    {
        OSTTY::print(OSTTY::RED,"usage: sendFile filepath\n");
        return;
    }
    else
        strFile = argv[0];

    MsQuicClient  client("transtest");
    MsQuicStream* pStream  = nullptr;
    char buffer[128] = { 0 };
    int  peroid = 1000;/*us*/

    if (client.connect("127.0.0.1",TEST_PORT))
    {
        OSTTY::print(OSTTY::GREEN,"waiting connect ...\n");
        if (client.waitForConnected(5000))
            OSTTY::print(OSTTY::GREEN,"connected\n");
        else {
            OSTTY::print(OSTTY::RED,"connected timeout\n");
            return;
        }
        pStream = client.createStream(nullptr);

        FILE* fp = fopen(strFile.c_str(),"rb");
        char *buf = (char*)malloc(64000);

        while (fp)
        {
            int len = fread(buf,1,64000,fp);
            if (len>0)
                pStream->send((unsigned char*)buf,len);
            else
                break;

            usleep(peroid);
        }
        sleep(10);
        printf("send end\n");
    }


}

UNITTEST(recvFile)
{
    int   total = 0;
    std::string strFile;
    if (argc<1)
    {
        return;;
    }
    else
        strFile = argv[0];

    const QUIC_REGISTRATION_CONFIG RegConfig = { "quicsample", QUIC_EXECUTION_PROFILE_LOW_LATENCY };
    MsQuicServer server("transtest",TEST_PORT,RegConfig);

    if (server.init("msqucitest.crt","msqucitest.priv.key"))
    {
        server.start();
        MsQuicStream* stream = server.accept(nullptr);

        OSTTY::print(OSTTY::GREEN,"accept stream=%p\n",stream);
        FILE* fp = fopen(strFile.c_str(),"wb");
        int   buflen = 32000;
        char *buf = (char*)malloc(32000);


        while (true) {
            int   len = 0;
            len = stream->recv((unsigned char*)buf,buflen,-1);

            total+=len;

            if (len<0)
                break;

            fwrite(buf,1,len,fp);

#if 0
            else if (len==0)
            {
                buflen *=2;
                buf  = (char*)realloc(buf,buflen);
            }
#endif
        }
        fclose(fp);
        printf("recv end:total=%d\n",total);
    }
    else
    {
        OSTTY::print(OSTTY::RED,"server init failed\n");
    }
}
