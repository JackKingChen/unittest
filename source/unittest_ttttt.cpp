
#include "unittest.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>

struct VBoxHgsmiParm
{
    int id; // it is a pointer.
    int op; // operation type
    int datalen;
    unsigned long long data; // send
    int buflen;
    unsigned long long buf; // recv
} __attribute__ ((packed));

UNITTEST(ioctrl)
{
    VBoxHgsmiParm parms;

    int iofile = open("/dev/tttt",O_RDWR);

    if (iofile<0) {
        //printf("TTTT: Unable to open /dev/tttt errno=%d\n",errno);
        return;
    }

    parms.id  = iofile;
    parms.op  = 12345678;
    char* data = "hello,world";
    parms.datalen = strlen(data)+1;
    parms.data = (unsigned long long)data;
    char* buf  = "good bye";
    parms.buflen = strlen(buf)+1;
    parms.buf  = (unsigned long long)buf;
    
     int rc = ioctl(iofile,1, &parms);

    if (rc != 0 ) {
        //printf("connectHgsmi Error in write errno=%d\n", rc);
        return;
    }
}
