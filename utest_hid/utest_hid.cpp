
#include "unittest.h"
#include "hidapi.h"

#define HWCASTING_VID  0x0527
#define HWCASTING_PID  0xA5A6

hid_device* usb_dev = nullptr;

UNITTEST(hid_init)
{
    if (hid_init()<0)
    {
        printf("hid_init failed\n");
        return;
    }
    usb_dev = hid_open(HWCASTING_VID, HWCASTING_PID, 1, nullptr);

    if (usb_dev==nullptr)
    {
        printf("hid_open failed\n");
    }
}

UNITTEST(hid_exit)
{
    if (usb_dev)
        hid_close(usb_dev);

    hid_exit();
}

UNITTEST(hid_test)
{
    if (argc < 1)
    {
        printf("usage:hid_test length");
        return;
    }

    long long  size = atol(argv[0]);

    int ret = 0;

    char * buffer = (char*)malloc(size);
    int    inc    = 0;

    if (usb_dev != NULL)
    {
        // 增加包头
        uint8_t header[4] = { 0xFE, 0xFE, 0xF7, 0x77 };
        unsigned int length = size + 16;
        uint8_t byteLength[4];
        byteLength[0] = (length & 0xFF000000) >> 24;
        byteLength[1] = (length & 0x00FF0000) >> 16;
        byteLength[2] = (length & 0x0000FF00) >> 8;
        byteLength[3] = length & 0x000000FF;
        uint8_t byteParams[8] = {0};
        byteParams[0] = 0; // 0: video 1: configure
        byteParams[1] = 0;
        byteParams[2] = 1;
        byteParams[3] = (inc & 0xFF000000) >> 24;
        byteParams[4] = (inc & 0x00FF0000) >> 16;
        byteParams[5] = (inc & 0x0000FF00) >> 8;
        byteParams[6] = inc & 0x000000FF;

        memcpy(buffer, header, 4);
        memcpy(buffer + 4, byteLength, 4);
        memcpy(buffer + 8, byteParams, 8);

        static long long s_startTransfer = 0;
        timespec time1;
        timespec time2;
        clock_gettime(CLOCK_REALTIME, &time1);
        static int  ms = 0;
        if (!s_startTransfer)
        {
            ms = time1.tv_sec*1000+time1.tv_nsec/1000/1000;
        }

        int nCount = (length+1023-1)/1023;
        int nRes   = length % 1023;
        int nPoint = 0;

        char cWrite[1024];
        for (int i=0; i<nCount; i++)
        {
            if (i==nCount-1)
            {
                if (nRes==0)
                    memcpy(cWrite+1,buffer+nPoint,1023);
                else
                    memcpy(cWrite+1,buffer+nPoint,nRes);
            }
            else
                memcpy(cWrite+1,buffer+nPoint,1023);

            cWrite[0] = 2;

            if (hid_write(usb_dev, (unsigned char*)cWrite, 1024)<0)
            {
                printf("failed\n");
                return;
            }

            nPoint += 1023;
        }
        free(buffer);

        clock_gettime(CLOCK_REALTIME, &time2);

        int  ns1 = time1.tv_sec*1000*1000*1000+time1.tv_nsec;
        int  ns2 = time2.tv_sec*1000*1000*1000+time2.tv_nsec;

        s_startTransfer += length;

        printf("trans %lld bytes coss %lld us,speed=%lld B/s\n",size,(ns2-ns1)/1000,size*1000*1000/((ns2-ns1)/1000));


        if (ret < 0) {
            // TODO: re init the usb | verify length
            // Do nothing
        }

    }


}
