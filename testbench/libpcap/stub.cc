/*/*******************************************************************
*
*    DESCRIPTION:Copyright(c) 2010-2020 Xiamen Yealink Network Technology Co,.Ltd
*
*    AUTHOR: The Venus project authors. All Rights Reserved.
*
*    HISTORY:
*
*    DATE:2013-04-07
*
*******************************************************************/

#include "host.h"
#include "pcap.h"

#if defined(OS_LINUX) && !defined(HAVE_PCAP)

/************************************************************************/
/*                                                                      */
/************************************************************************/

__attribute__((weak)) 
pcap_t *pcap_open_offline(const char *, char *)
{
    return NULL;
}

__attribute__((weak)) 
pcap_t *pcap_open_dead(int, int)
{
    return NULL;
}


__attribute__((weak))
void pcap_close(pcap_t *)
{

}

__attribute__((weak)) 
int pcap_compile(pcap_t *, struct bpf_program *, const char *, int,bpf_u_int32)
{

    return -1;
}

__attribute__((weak)) 
int pcap_setfilter(pcap_t *, struct bpf_program *)
{
    return -1;
}

__attribute__((weak)) 
int pcap_next_ex(pcap_t *, struct pcap_pkthdr **, const u_char **)
{
    return -1;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
__attribute__((weak)) 
pcap_dumper_t *pcap_dump_open(pcap_t *, const char *)
{

    return NULL;
}

__attribute__((weak)) 
void pcap_dump(u_char *, const struct pcap_pkthdr *, const u_char *)
{

}

__attribute__((weak)) 
void pcap_freecode(struct bpf_program *)
{

}

/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*OS_LINUX*/
