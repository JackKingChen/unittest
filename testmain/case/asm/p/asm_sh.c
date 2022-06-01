/* radare - LGPL - Copyright 2010-2011 eloi<limited-entropy.com> */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <r_types.h>
#include <r_lib.h>
#include <r_util.h>
#include <r_asm.h>
#include "dis-asm.h"

static unsigned long Offset = 0;
static char *buf_global = NULL;
static unsigned char bytes[2];

static int sh_buffer_read_memory (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *info) {
	memcpy (myaddr, bytes, length);
	return 0;
}

//int print_insn_shl (bfd_vma memaddr, struct disassemble_info *info);

static int symbol_at_address(bfd_vma addr, struct disassemble_info * info) {
	return 0;
}

static void memory_error_func(int status, bfd_vma memaddr, struct disassemble_info *info) {
	//--
}

static void print_address(bfd_vma address, struct disassemble_info *info) {
	char tmp[32];
	if (buf_global == NULL)
		return;
	sprintf (tmp, "0x%08"PFMT64x"", (ut64)address);
	strcat (buf_global, tmp);
}

static int buf_fprintf(void *stream, const char *format, ...) {
	va_list ap;
	char *tmp;
	int ret;
	if (buf_global == NULL)
		return 0;
	va_start (ap, format);
	tmp = malloc (strlen (format)+strlen (buf_global)+2);
	if (!tmp) return 0;
	sprintf (tmp, "%s%s", buf_global, format);
	ret = vsprintf (buf_global, tmp, ap);
	va_end (ap);
	free (tmp);
	return ret;
}

static int disassemble(RAsm *a, struct r_asm_op_t *op, const ut8 *buf, ut64 len) {
	static struct disassemble_info disasm_obj;
	if (len<2) return -1;
	buf_global = op->buf_asm;
	Offset = a->pc;
	memcpy (bytes, buf, 2); // TODO handle thumb

	/* prepare disassembler */
	memset (&disasm_obj,'\0', sizeof (struct disassemble_info));
	disasm_obj.buffer = bytes;
	disasm_obj.read_memory_func = &sh_buffer_read_memory;
	disasm_obj.symbol_at_address_func = &symbol_at_address;
	disasm_obj.memory_error_func = &memory_error_func;
	disasm_obj.print_address_func = &print_address;
	disasm_obj.endian = !a->big_endian;
	disasm_obj.fprintf_func = &buf_fprintf;
	disasm_obj.stream = stdout;

	op->buf_asm[0]='\0';
	op->inst_len = print_insn_shl ((bfd_vma)Offset, &disasm_obj);

	if (op->inst_len == -1)
		strncpy (op->buf_asm, " (data)", R_ASM_BUFSIZE);
	return op->inst_len;
}

RAsmPlugin r_asm_plugin_sh = {
	.name = "sh",
	.arch = "sh",
	.bits = (int[]){ 32, 0 },
	.desc = "SH-4 disassembly plugin",
	.init = NULL,
	.fini = NULL,
	.disassemble = &disassemble,
	.assemble = NULL
};

#ifndef CORELIB
struct r_lib_struct_t radare_plugin = {
	.type = R_LIB_TYPE_ASM,
	.data = &r_asm_plugin_sh
};
#endif
