
#include "disasm.h"

bool Asm2Machinecode(char* pAsmcode, unsigned long dwAddrress, __out t_asmmodel* pMachinecode);
bool AsmInject(int pid, const std::string& asmcode);

