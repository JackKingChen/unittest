/* radare - LGPL - Copyright 2009-2011 */

#include <stdio.h>
#include <string.h>

#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>

#include <dalvik/opcode.h>

static int dalvik_disassemble (RAsm *a, RAsmOp *op, const ut8 *buf, ut64 len) {
	int i = (int) buf[0];
	int size = 0;
	int vA, vB, vC;
	char str[1024];
	ut64 offset;

	if (dalvik_opcodes[i].len <= len) {
		strcpy (op->buf_asm, dalvik_opcodes[i].name);
		size = dalvik_opcodes[i].len;
		switch (dalvik_opcodes[i].fmt) {
		case fmtop: break;
		case fmtopvAvB:
			vA = buf[1] & 0x0f;
			vB = (buf[1] & 0xf0)>>4;
			sprintf (str, " v%i, v%i", vA, vB);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAAvBBBB:
			vA = (int) buf[1];
			vB = (buf[3]<<8) | buf[2];
			sprintf (str, " v%i, v%i", vA, vB);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAAAAvBBBB: // buf[1] seems useless :/
			vA = (buf[3]<<8) | buf[2];
			vB = (buf[5]<<8) | buf[4];
			sprintf (str, " v%i, v%i", vA, vB);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAA:
			vA = (int) buf[1];
			sprintf (str, " v%i", vA);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAcB:
			vA = buf[1] & 0x0f;
			vB = (buf[1] & 0xf0)>>4;
			sprintf (str, " v%i, %#x", vA, vB);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAAcBBBB:
			vA = (int) buf[1];
			short sB = (buf[3]<<8) | buf[2];
			sprintf (str, " v%i, %#04hx", vA, sB);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAAcBBBBBBBB:
			vA = (int) buf[1];
			vB = buf[5]|(buf[4]<<8)|(buf[3]<<16)|(buf[2]<<24);
			sprintf (str, " v%i, %#08x", vA, vB);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAAcBBBB0000:
			vA = (int) buf[1];
			vB = 0|(buf[3]<<16)|(buf[2]<<24);
			sprintf (str, " v%i, %#08x", vA, vB);
			if (buf[0] == 19) strcat (str, "00000000"); // const-wide/high16
			strcat (op->buf_asm, str);
			break;
		case fmtopvAAcBBBBBBBBBBBBBBBB:
			vA = (int) buf[1];
			long long int lB = buf[9]|(buf[8]<<8)|(buf[7]<<16)|(buf[6]<<24)|
				((long long int)buf[5]<<32)|((long long int)buf[4]<<40)|
				((long long int)buf[3]<<48)|((long long int)buf[2]<<56);
			sprintf (str, " v%i, 0x%"PFMT64x, vA, lB);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAAvBBvCC:
			vA = (int) buf[1];
			vB = (int) buf[2];
			vC = (int) buf[3];
			sprintf (str, " v%i, v%i, v%i", vA, vB, vC);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAAvBBcCC:
			vA = (int) buf[1];
			vB = (int) buf[2];
			vC = (int) buf[3];
			sprintf (str, " v%i, v%i, %#x", vA, vB, vC);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAvBcCCCC:
			vA = buf[1] & 0x0f;
			vB = (buf[1] & 0xf0)>>4;
			vC = (buf[3]<<8) | buf[2];
			sprintf (str, " v%i, v%i, %#x", vA, vB, vC);
			strcat (op->buf_asm, str);
			break;
		case fmtoppAA:
			vA = (char) buf[1];
			sprintf (str, " %i", vA);
			strcat (op->buf_asm, str);
			break;
		case fmtoppAAAA:
			vA = (short) (buf[3] <<8 | buf[2]);
			sprintf (str, " %i", vA);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAApBBBB:
			vA = (int) buf[1];
			vB = (int) (buf[3] <<8 | buf[2]);
			sprintf (str, " v%i, %i", vA, vB);
			strcat (op->buf_asm, str);
			break;
		case fmtoppAAAAAAAA:
			vA = (int) (buf[2]|(buf[3]<<8)|(buf[4]<<16)|(buf[5]<<24));
			sprintf (str, " %#08x", vA);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAvBpCCCC:
			vA = buf[1] & 0x0f;
			vB = (buf[1] & 0xf0)>>4;
			vC = (int) (buf[3] <<8 | buf[2]);
			sprintf (str, " v%i, v%i, %i", vA, vB, vC);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAApBBBBBBBB:
			vA = (int) buf[1];
			vB = (int) (buf[5]|(buf[4]<<8)|(buf[3]<<16)|(buf[2]<<24));
			sprintf (str, " v%i, %i", vA, vB);
			strcat (op->buf_asm, str);
			break;
		case fmtoptinlineI:
			vA = (int) (buf[1] & 0x0f);
			vB = (buf[3]<<8) | buf[2];
			switch (vA) {
				case 1:
					sprintf (str, " {v%i}", buf[4] & 0x0f);
					break;
				case 2:
					sprintf (str, " {v%i, v%i}", buf[4]&0x0f, (buf[4]&0xf0)>>4);
					break;
				case 3:
					sprintf (str, " {v%i, v%i, v%i}", buf[4]&0x0f,
							(buf[4]&0xf0)>>4, buf[5]&0x0f);
					break;
				case 4:
					sprintf (str, " {v%i, v%i, v%i, v%i}", buf[4]&0x0f,
							(buf[4]&0xf0)>>4, buf[5]&0x0f, (buf[5]&0xf0)>>4);
					break;
				default:
					sprintf (str, " {}");
			}
			strcat (op->buf_asm, str);
			sprintf (str, ", [%04x]", vB);
			strcat (op->buf_asm, str);
			break;
		case fmtoptinlineIR:
		case fmtoptinvokeVSR:
			vA = (int) buf[1];
			vB = (buf[3]<<8) | buf[2];
			vC = (buf[5]<<8) | buf[4];
			sprintf (str, " {v%i..v%i}, [%04x]", vC, vC+vA-1, vB);
			strcat (op->buf_asm, str);
			break;
		case fmtoptinvokeVS:
			vA = (int) (buf[1] & 0xf0)>>4;
			vB = (buf[3]<<8) | buf[2];
			switch (vA) {
				case 1:
					sprintf (str, " {v%i}", buf[4] & 0x0f);
					break;
				case 2:
					sprintf (str, " {v%i, v%i}", buf[4]&0x0f, (buf[4]&0xf0)>>4);
					break;
				case 3:
					sprintf (str, " {v%i, v%i, v%i}", buf[4]&0x0f,
							(buf[4]&0xf0)>>4, buf[5]&0x0f);
					break;
				case 4:
					sprintf (str, " {v%i, v%i, v%i, v%i}", buf[4]&0x0f,
							(buf[4]&0xf0)>>4, buf[5]&0x0f, (buf[5]&0xf0)>>4);
					break;
				default:
					sprintf (str, " {}");
			}
			strcat (op->buf_asm, str);
			sprintf (str, ", [%04x]", vB);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAAtBBBB:
			vA = (int) buf[1];
			vB = (buf[3]<<8) | buf[2];
			if (buf[0] == 0x1a) {
				offset = R_ASM_GET_OFFSET(a, 's', vB);
				if (offset == -1)
					sprintf (str, " v%i, string+%i", vA, vB);
				else
					sprintf (str, " v%i, 0x%"PFMT64x, vA, offset);
			} else if (buf[0] == 0x1c || buf[0] == 0x1f || buf[0] == 0x22) {
				offset = R_ASM_GET_OFFSET(a, 'c', vB);
				if (offset == -1)
					sprintf (str, " v%i, class+%i", vA, vB);
				else
					sprintf (str, " v%i, 0x%"PFMT64x, vA, offset);
			} else {
				offset = R_ASM_GET_OFFSET(a, 'f', vB);
				if (offset == -1)
					sprintf (str, " v%i, field+%i", vA, vB);
				else
					sprintf (str, " v%i, 0x%"PFMT64x, vA, offset);
			}
			strcat (op->buf_asm, str);
			break;
		case fmtoptopvAvBoCCCC:
			vA = (buf[1] & 0x0f);
			vB = (buf[1] & 0xf0)>>4;
			vC = (buf[3]<<8) | buf[2];
			offset = R_ASM_GET_OFFSET(a, 'o', vC);
			if (offset == -1)
				sprintf (str, " v%i, v%i, [obj+%04x]", vA, vB, vC);
			else
				sprintf (str, " v%i, v%i, [0x%"PFMT64x"]", vA, vB, offset);
			strcat (op->buf_asm, str);
			break;
		case fmtopAAtBBBB:
			vA = (int) buf[1];
			vB = (buf[3]<<8) | buf[2];
			offset = R_ASM_GET_OFFSET(a, 't', vB);
			if (offset == -1)
				sprintf (str, " v%i, thing+%i", vA, vB);
			else
				sprintf (str, " v%i, 0x%"PFMT64x, vA, offset);
			strcat (op->buf_asm, str);
			break;
		case fmtopvAvBtCCCC:
			vA = (buf[1] & 0x0f);
			vB = (buf[1] & 0xf0)>>4;
			vC = (buf[3]<<8) | buf[2];
			if (buf[0] == 0x20 || buf[0] == 0x23) { //instance-of & new-array
				offset = R_ASM_GET_OFFSET(a, 'c', vC);
				if (offset == -1)
					sprintf (str, " v%i, v%i, class+%i", vA, vB, vC);
				else
					sprintf (str, " v%i, v%i, 0x%"PFMT64x, vA, vB, offset);
			} else {
				offset = R_ASM_GET_OFFSET(a, 'f', vC);
				if (offset == -1)
					sprintf (str, " v%i, v%i, field+%i", vA, vB, vC);
				else
					sprintf (str, " v%i, v%i, 0x%"PFMT64x, vA, vB, offset);
			}
			strcat (op->buf_asm, str);
			break;
		case fmtopvAAtBBBBBBBB:
			vA = (int) buf[1];
			vB = (int) (buf[5]|(buf[4]<<8)|(buf[3]<<16)|(buf[2]<<24));
			offset = R_ASM_GET_OFFSET(a, 's', vB);
			if (offset == -1)
				sprintf (str, " v%i, string+%i", vA, vB);
			else
				sprintf (str, " v%i, 0x%"PFMT64x, vA, offset);
			strcat (op->buf_asm, str);
			break;
		case fmtopvCCCCmBBBB:
			vA = (int) buf[1];
			vB = (buf[3]<<8) | buf[2];
			vC = (buf[5]<<8) | buf[4];
			if (buf[0] == 0x25) { // filled-new-array/range
				offset = R_ASM_GET_OFFSET(a, 'c', vB);
				if (offset == -1)
					sprintf (str, " {v%i..v%i}, class+%i", vC, vC+vA-1, vB);
				else
					sprintf (str, " {v%i..v%i}, 0x%"PFMT64x, vC, vC+vA-1, offset);
			} else {
				offset = R_ASM_GET_OFFSET(a, 'm', vB);
				if (offset == -1)
					sprintf (str, " {v%i..v%i}, method+%i", vC, vC+vA-1, vB);
				else
					sprintf (str, " {v%i..v%i}, 0x%"PFMT64x, vC, vC+vA-1, offset);
			}
			strcat (op->buf_asm, str);
			break;
		case fmtopvXtBBBB:
			vA = (int) (buf[1] & 0xf0)>>4;
			vB = (buf[3]<<8) | buf[2];
			switch (vA) {
				case 1:
					sprintf (str, " {v%i}", buf[4] & 0x0f);
					break;
				case 2:
					sprintf (str, " {v%i, v%i}", buf[4]&0x0f, (buf[4]&0xf0)>>4);
					break;
				case 3:
					sprintf (str, " {v%i, v%i, v%i}", buf[4]&0x0f,
							(buf[4]&0xf0)>>4, buf[5]&0x0f);
					break;
				case 4:
					sprintf (str, " {v%i, v%i, v%i, v%i}", buf[4]&0x0f,
							(buf[4]&0xf0)>>4, buf[5]&0x0f, (buf[5]&0xf0)>>4);
					break;
				default:
					sprintf (str, " {}");
			}
			strcat (op->buf_asm, str);
			if (buf[0] == 0x24) { // filled-new-array
				offset = R_ASM_GET_OFFSET(a, 'c', vB);
				if (offset == -1)
					sprintf (str, ", class+%i", vB);
				else
					sprintf (str, ", 0x%"PFMT64x, offset);
			} else {
				offset = R_ASM_GET_OFFSET(a, 'm', vB);
				if (offset == -1)
					sprintf (str, ", method+%i", vB);
				else
					sprintf (str, ", 0x%"PFMT64x, offset);

			}
			strcat (op->buf_asm, str);
			break;
		case fmtoptinvokeI: // Any opcode has this formats
		case fmtoptinvokeIR:
		case fmt00:
		default:
			strcpy (op->buf_asm, "invalid ");
			size = 2;
		}
		op->inst_len = size;
	} else {
		strcpy (op->buf_asm, "invalid ");
		op->inst_len = len;
		size = len;
	}
	return size;
}

//TODO
static int dalvik_assemble(RAsm *a, RAsmOp *op, const char *buf) {
	int i;
	char *p = strchr (buf,' ');
	if (p) *p = 0;
	for (i=0; i<256; i++)
		if (!strcmp (dalvik_opcodes[i].name, buf)) {
			r_mem_copyendian (op->buf, (void*)&i, 4, a->big_endian);
			op->inst_len = dalvik_opcodes[i].len;
			return op->inst_len;
		}
	return 0;
}

static int init (void *user) {
	return R_TRUE;
}

RAsmPlugin r_asm_plugin_dalvik = {
	.name = "dalvik",
	.arch = "dalvik",
	.desc = "Dalvik (Android VM) disassembly plugin",
	.bits = (int[]){ 32, 64, 0 },
	.init = &init,
	.fini = NULL,
	.disassemble = &dalvik_disassemble,
	.assemble = &dalvik_assemble
};

#ifndef CORELIB
struct r_lib_struct_t radare_plugin = {
	.type = R_LIB_TYPE_ASM,
	.data = &r_asm_plugin_dalvik
};
#endif
