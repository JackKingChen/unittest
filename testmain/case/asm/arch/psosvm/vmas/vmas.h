#ifndef _INCLUDE_VMAS_H_
#define _INCLUDE_VMAS_H_ 1

int psosvm_disasm(const ut8 *bytes, char *output);
int psosvmasm_init();
int psosvm_assemble(unsigned char *bytes, const char *string);

#endif
