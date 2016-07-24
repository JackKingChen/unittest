/**
 * @file re_sys.h  Interface to system module
 *
 * Copyright (C) 2010 Creytiv.com
 */

struct re_printf;

int  sys_daemon(void);
void sys_usleep(unsigned int us);

uint16_t sys_htols(uint16_t v);
uint32_t sys_htoll(uint32_t v);
uint16_t sys_ltohs(uint16_t v);
uint32_t sys_ltohl(uint32_t v);
uint64_t sys_htonll(uint64_t v);
uint64_t sys_ntohll(uint64_t v);


/* Random */
void     rand_init(void);
uint16_t rand_u16(void);
uint32_t rand_u32(void);
uint64_t rand_u64(void);
char     rand_char(void);
void     rand_str(char *str, size_t size);
void     rand_bytes(uint8_t *p, size_t size);
