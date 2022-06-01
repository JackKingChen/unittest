/* radare - LGPL - Copyright 2009-2011 pancake<nopcode.org> */

#include <r_types.h>
#include <r_util.h>
#include <r_lib.h>
#include <r_asm.h>

static int assemble(RAsm *a, RAsmOp *op, const char *buf) {
	char *ipath, *opath;
	int ifd, ofd;
	char asm_buf[R_ASM_BUFSIZE];
	int len = 0;
	if (a->syntax != R_ASM_SYNTAX_INTEL) {
		eprintf ("asm.x86.nasm does not support non-intel syntax\n");
		return -1;
	}

	ifd = r_file_mkstemp ("r_nasm", &ipath);
	ofd = r_file_mkstemp ("r_nasm", &opath);

	len = snprintf (asm_buf, sizeof (asm_buf),
			"BITS %i\nORG 0x%"PFMT64x"\n%s", a->bits, a->pc, buf);
	write (ifd, asm_buf, len);

	close (ifd);

	if ( !r_sys_cmdf ("nasm %s -o %s", ipath, opath)) {
		len = read (ofd, op->buf, R_ASM_BUFSIZE);
	} else {
		eprintf ("Error running 'nasm'\n");
		len = 0;
	}

	close (ofd);

	unlink (ipath);
	unlink (opath);
	free (ipath);
	free (opath);

	op->inst_len = len;
	return len;
}

RAsmPlugin r_asm_plugin_x86_nasm = {
	.name = "x86.nasm",
	.desc = "X86 nasm assembler plugin",
	.arch = "x86",
	// NOTE: 64bits is not supported on OSX's nasm :(
	.bits = (int[]){ 16, 32, 64, 0 },
	.init = NULL,
	.fini = NULL,
	.disassemble = NULL,
	.assemble = &assemble, 
};

#ifndef CORELIB
struct r_lib_struct_t radare_plugin = {
	.type = R_LIB_TYPE_ASM,
	.data = &r_asm_plugin_x86_nasm
};
#endif
