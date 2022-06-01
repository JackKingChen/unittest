/* radare - LGPL - Copyright 2009-2012 nibble<.ds@gmail.com> */

#include <stdio.h>

#include <r_types.h>
#include <r_util.h>
#include <r_asm.h>
#include <list.h>
#include "../config.h"

static RAsmPlugin *asm_static_plugins[] = { R_ASM_STATIC_PLUGINS };

static int r_asm_pseudo_align(struct r_asm_op_t *op, char *input) {
	eprintf ("TODO: .align\n"); // Must add padding for labels and others.. but this is from RAsm, not RAsmOp
	return 0;
}

static int r_asm_pseudo_string(struct r_asm_op_t *op, char *input, int zero) {
	int len = strlen(input)-1;
	// TODO: if not starting with '"'.. give up
	if (input[len]=='"')
		input[len] = 0;
	if (*input=='"')
		input++;
	len = r_str_escape (input)+zero;
	r_hex_bin2str ((ut8*)input, len, op->buf_hex);
	strncpy ((char*)op->buf, input, R_ASM_BUFSIZE);
	return len;
}

static inline int r_asm_pseudo_arch(RAsm *a, char *input) {
	if (!r_asm_use (a, input)) {
		eprintf ("Error: Unknown plugin\n");
		return -1;
	}
	return 0;
}

static inline int r_asm_pseudo_bits(RAsm *a, char *input) {
	if (!(r_asm_set_bits (a, r_num_math (NULL, input))))
		eprintf ("Error: Unsupported bits value\n");
	else return 0;
	return -1;
}

static inline int r_asm_pseudo_org(RAsm *a, char *input) {
	r_asm_set_pc (a, r_num_math (NULL, input));
	return 0;
}

static inline int r_asm_pseudo_hex(struct r_asm_op_t *op, char *input) {
	int len = r_hex_str2bin (input, op->buf);
	strncpy (op->buf_hex, r_str_trim (input), R_ASM_BUFSIZE);
	return len;
}

static inline int r_asm_pseudo_intN(RAsm *a, struct r_asm_op_t *op, char *input, int n) {
	const ut8 *p;
	short s;
	int i;
	long int l;
	ut64 s64 = r_num_math (NULL, input);
	if (n!= 8 && s64>>(n*8)) {
		eprintf ("int16 Out is out of range\n");
		return 0;
	}
	if (n == 2) {
		s = (short)s64;
		p = (const ut8*)&s;
	} else if (n == 4) {
		i = (int)s64;
		p = (const ut8*)&i;
	} else if (n == 8) {
		l = (long int)s64;
		p = (const ut8*)&l;
	} else return 0;
	r_mem_copyendian (op->buf, p, n, !a->big_endian);
	r_hex_bin2str (op->buf, n, op->buf_hex);
	return n;
}

static inline int r_asm_pseudo_int16(RAsm *a, struct r_asm_op_t *op, char *input) {
	return r_asm_pseudo_intN (a, op, input, 2);
}

static inline int r_asm_pseudo_int32(RAsm *a, struct r_asm_op_t *op, char *input) {
	return r_asm_pseudo_intN (a, op, input, 4);
}

static inline int r_asm_pseudo_int64(RAsm *a, struct r_asm_op_t *op, char *input) {
	return r_asm_pseudo_intN (a, op, input, 8);
}

static inline int r_asm_pseudo_byte(struct r_asm_op_t *op, char *input) {
	int i, len = 0;
	r_str_subchr (input, ',', ' ');
	len = r_str_word_count (input);
	r_str_word_set0 (input);
	for (i=0; i<len; i++) {
		const char *word = r_str_word_get0 (input, i);
		int num = (int)r_num_math (NULL, word);
		op->buf[i] = num;
	}
	r_hex_bin2str (op->buf, len, op->buf_hex);
	return len;
}

static inline int r_asm_pseudo_fill(struct r_asm_op_t *op, char *input) {
	int i, repeat, size, value;
	sscanf (input, "%d,%d,%d", &repeat, &size, &value); // use r_num?
	size *= repeat;
	if (size>0) {
		for (i=0; i<size; i++)
			op->buf[i] = value;
		r_hex_bin2str (op->buf, size, op->buf_hex);
	} else size = 0;
	return size;
}

R_API RAsm *r_asm_new() {
	int i;
	RAsmPlugin *static_plugin;
	RAsm *a = R_NEW (RAsm);
	if (!a) return NULL;
	a->pair = NULL;
	a->user = NULL;
	a->cur = NULL;
	a->binb.bin = NULL;
	a->bits = 32;
	a->big_endian = 0;
	a->pc = 0;
	a->ifilter = NULL;
	a->ofilter = NULL;
	a->syntax = R_ASM_SYNTAX_INTEL;
	a->plugins = r_list_new ();
	a->plugins->free = free;
	for (i=0; asm_static_plugins[i]; i++) {
		static_plugin = R_NEW (RAsmPlugin);
		memcpy (static_plugin, asm_static_plugins[i], sizeof (RAsmPlugin));
		r_asm_add (a, static_plugin);
	}
	return a;
}

R_API int r_asm_setup(RAsm *a, const char *arch, int bits, int big_endian) {
	int ret = 0;
	ret |= !r_asm_use (a, arch);
	ret |= !r_asm_set_bits (a, bits);
	ret |= !r_asm_set_big_endian (a, big_endian);
	return ret;
}

// TODO: spagueti
R_API int r_asm_filter_input(RAsm *a, const char *f) {
	if (!a->ifilter)
		a->ifilter = r_parse_new ();
	if (!r_parse_use (a->ifilter, f)) {
		r_parse_free (a->ifilter);
		a->ifilter = NULL;
		return R_FALSE;
	}
	return R_TRUE;
}

R_API int r_asm_filter_output(RAsm *a, const char *f) {
	if (!a->ofilter)
		a->ofilter = r_parse_new ();
	if (!r_parse_use (a->ofilter, f)) {
		r_parse_free (a->ofilter);
		a->ofilter = NULL;
		return R_FALSE;
	}
	return R_TRUE;
}

R_API void r_asm_free(RAsm *a) {
	if (!a) return;
	// TODO: any memory leak here?
	r_pair_free (a->pair);
	a->pair = NULL;
	// XXX: segfault, plugins cannot be freed
	a->plugins->free = NULL;
	r_list_free (a->plugins);
	a->plugins = NULL;
	free (a);
}

R_API void r_asm_set_user_ptr(RAsm *a, void *user) {
	a->user = user;
}

R_API int r_asm_add(RAsm *a, RAsmPlugin *foo) {
	RListIter *iter;
	RAsmPlugin *h;
	// TODO: cache foo->name length and use memcmp instead of strcmp
	if (!foo->name)
		return R_FALSE;
	if (foo->init)
		foo->init (a->user);
	r_list_foreach (a->plugins, iter, h)
		if (!strcmp (h->name, foo->name))
			return R_FALSE;
	r_list_append (a->plugins, foo);
	return R_TRUE;
}

R_API int r_asm_del(RAsm *a, const char *name) {
	/* TODO: Implement r_asm_del */
	return R_FALSE;
}

// TODO: this can be optimized using r_str_hash()
R_API int r_asm_use(RAsm *a, const char *name) {
	char file[1024];
	RAsmPlugin *h;
	RListIter *iter;
	r_list_foreach (a->plugins, iter, h)
		if (!strcmp (h->name, name)) {
			if (!a->cur || (a->cur && strcmp (a->cur->arch, h->arch))) {
				//const char *dop = r_config_get (core->config, "dir.opcodes");
				// TODO: allow configurable path for sdb files
				snprintf (file, sizeof (file), R_ASM_OPCODES_PATH"/%s.sdb", h->arch);
				r_pair_free (a->pair);
				a->pair = r_pair_new_from_file (file);
			}
			a->cur = h;
			return R_TRUE;
		}
	r_pair_free (a->pair);
	a->pair = NULL;
	return R_FALSE;
}

R_API int r_asm_set_subarch(RAsm *a, const char *name) {
	int ret = R_FALSE;
	if (a->cur && a->cur->set_subarch)
		ret = a->cur->set_subarch(a, name);
	return ret;
}

static int has_bits(RAsmPlugin *h, int bits) {
	int i;
	if (h && h->bits)
		for (i=0; h->bits[i]; i++)
			if (bits == h->bits[i])
				return R_TRUE;
	return R_FALSE;
}

R_API int r_asm_set_bits(RAsm *a, int bits) {
	if (has_bits (a->cur, bits)) {
		a->bits = bits;
		return R_TRUE;
	}
	return R_FALSE;
}

R_API int r_asm_set_big_endian(RAsm *a, int boolean) {
	a->big_endian = boolean;
	return R_TRUE;
}

R_API int r_asm_set_syntax(RAsm *a, int syntax) {
	switch (syntax) {
	case R_ASM_SYNTAX_INTEL:
	case R_ASM_SYNTAX_ATT:
		a->syntax = syntax;
		return R_TRUE;
	default:
		return R_FALSE;
	}
}

R_API int r_asm_set_pc(RAsm *a, ut64 pc) {
	a->pc = pc;
	return R_TRUE;
}

R_API int r_asm_disassemble(RAsm *a, struct r_asm_op_t *op, const ut8 *buf, ut64 len) {
	int ret = 0;
	if (a->cur && a->cur->disassemble)
		ret = a->cur->disassemble (a, op, buf, len);
	if (ret > 0) {
		if (a->ofilter)
			r_parse_parse (a->ofilter, op->buf_asm, op->buf_asm);
		else memcpy (op->buf, buf, ret);
		r_hex_bin2str (buf, ret, op->buf_hex);
	} else ret = 0;
	return ret;
}

R_API int r_asm_assemble(RAsm *a, struct r_asm_op_t *op, const char *buf) {
	int ret = 0;
	RAsmPlugin *h;
	RListIter *iter;
	char *b = strdup (buf);
	if (a->ifilter)
		r_parse_parse (a->ifilter, buf, b);
	if (a->cur) {
		if (!a->cur->assemble) {
			/* find callback if no assembler support in current plugin */
			r_list_foreach (a->plugins, iter, h) {
				if (h->arch && h->assemble
				&& has_bits (h, a->bits)
				&& !strcmp (a->cur->arch, h->arch)) {
					ret = h->assemble (a, op, b);
					break;
				}
			}
		} else ret = a->cur->assemble (a, op, b);
	}
	if (op && ret > 0) {
		r_hex_bin2str (op->buf, ret, op->buf_hex);
		op->inst_len = ret;
		op->buf_hex[ret*2] = 0;
		strncpy (op->buf_asm, b, R_ASM_BUFSIZE);
	}
	return ret;
}

R_API RAsmCode* r_asm_mdisassemble(RAsm *a, ut8 *buf, ut64 len) {
	struct r_asm_op_t op;
	RAsmCode *acode;
	int ret, slen;
	ut64 idx;

	if (!(acode = r_asm_code_new ()))
		return NULL;
	if (!(acode->buf = malloc (1+len)))
		return r_asm_code_free (acode);
	memcpy (acode->buf, buf, len);
	if (!(acode->buf_hex = malloc (2*len+1)))
		return r_asm_code_free(acode);
	r_hex_bin2str (buf, len, acode->buf_hex);
	if (!(acode->buf_asm = malloc (2)))
		return r_asm_code_free (acode);
	
	for (idx = ret = slen = 0, acode->buf_asm[0] = '\0'; idx < len; idx+=ret) {
		r_asm_set_pc (a, a->pc + ret);
		ret = r_asm_disassemble (a, &op, buf+idx, len-idx);
		if (ret<1) {
			eprintf ("disassemble error at offset %"PFMT64d"\n", idx);
			return acode;
		}
		if (a->ofilter)
			r_parse_parse (a->ofilter, op.buf_asm, op.buf_asm);
		slen += strlen (op.buf_asm) + 2;
		if (!(acode->buf_asm = realloc (acode->buf_asm, slen)))
			return r_asm_code_free (acode);
		strcat (acode->buf_asm, op.buf_asm);
		strcat (acode->buf_asm, "\n");
	}
	acode->len = idx;
	return acode;
}

R_API RAsmCode* r_asm_mdisassemble_hexstr(RAsm *a, const char *hexstr) {
	RAsmCode *ret;
	ut8 *buf;
	int len;

	if (!(buf = malloc (1+strlen (hexstr))))
		return NULL;
	len = r_hex_str2bin (hexstr, buf);
	if (len == -1) {
		free (buf);
		return NULL;
	}
	ret = r_asm_mdisassemble (a, buf, (ut64)len);
	if (a->ofilter)
		r_parse_parse (a->ofilter, ret->buf_asm, ret->buf_asm);
	free (buf);
	return ret;
}

R_API RAsmCode* r_asm_assemble_file(RAsm *a, const char *file) {
	RAsmCode *ac;
	char *f = r_file_slurp (file, NULL);
	if (!f) return NULL;
	ac = r_asm_massemble (a, f);
	free (f);
	return ac;
}

R_API RAsmCode* r_asm_massemble(RAsm *a, const char *buf) {
	char *lbuf = NULL, *ptr2, *ptr = NULL, *ptr_start = NULL,
		 *tokens[R_ASM_BUFSIZE], buf_token[R_ASM_BUFSIZE];
	int labels = 0, stage, ret, idx, ctr, i, j;
	struct r_asm_op_t op;
	ut64 off;
	RAsmCode *acode = NULL;

	if (buf == NULL)
		return NULL;
	if (!(acode = r_asm_code_new ()))
		return NULL;
	if (!(acode->buf_asm = malloc (strlen (buf)+16)))
		return r_asm_code_free (acode);
	strncpy (acode->buf_asm, buf, sizeof (acode->buf_asm)-1);
	if (!(acode->buf_hex = malloc (64)))
		return r_asm_code_free (acode);
	*acode->buf_hex = 0;
	if (!(acode->buf = malloc (64)))
		return r_asm_code_free (acode);
	lbuf = strdup (buf);

	if (strchr (lbuf, ':'))
		labels = 1;

	/* Tokenize */
	for (tokens[0] = lbuf, ctr = 0;
		(ptr = strchr (tokens[ctr], ';')) || 
		(ptr = strchr (tokens[ctr], '\n')) ||
		(ptr = strchr (tokens[ctr], '\r'));
		tokens[++ctr] = ptr+1)
			*ptr = '\0';

	/* Stage 0-1: Parse labels*/
	/* Stage 2: Assemble */
	for (stage = 0; stage < 3; stage++) {
		if (stage < 2 && !labels)
			continue;
		for (idx = ret = i = j = 0, off = a->pc, acode->buf_hex[0] = '\0';
			i <= ctr; i++, idx += ret) {
			strncpy (buf_token, tokens[i], R_ASM_BUFSIZE);
			for (ptr_start = buf_token; *ptr_start &&
				isseparator (*ptr_start); ptr_start++);
			ptr = strchr (ptr_start, '#'); /* Comments */
			if (ptr && !R_BETWEEN ('0', ptr[1], '9'))
				*ptr = '\0';
			if (stage == 2) {
				r_asm_set_pc (a, a->pc + ret);
				off = a->pc;
			} else off +=ret;
			ret = 0;
			if (!*ptr_start)
				continue;
			//eprintf ("LINE %d %s\n", stage, ptr_start);
			if (labels) /* Labels */
			if ((ptr = strchr (ptr_start, ':'))) {
				char food[64];
				if (stage != 2) {
					*ptr = 0;
					snprintf (food, sizeof (food), "0x%"PFMT64x"", off);
// TODO: warning when redefined
					r_asm_code_set_equ (acode, ptr_start, food);
				}
				ptr_start = ptr + 1;
			}
			if (*ptr_start == '\0') {
				ret = 0;
				continue;	
			} else if (*ptr_start == '.') { /* pseudo */
				ptr = ptr_start;
				if (!memcmp (ptr, ".intel_syntax", 13)) 
					a->syntax = R_ASM_SYNTAX_INTEL;
				else if (!memcmp (ptr, ".att_syntax", 10)) 
					a->syntax = R_ASM_SYNTAX_ATT;
				else if (!memcmp (ptr, ".string", 7)) {
					r_str_chop (ptr+7);
					ret = r_asm_pseudo_string (&op, ptr+7, 1);
				} else if (!memcmp (ptr, ".ascii ", 7))
					ret = r_asm_pseudo_string (&op, ptr+7, 0);
				else if (!memcmp (ptr, ".align", 7))
					ret = r_asm_pseudo_align (&op, ptr+7);
				else if (!memcmp (ptr, ".arch ", 6))
					ret = r_asm_pseudo_arch (a, ptr+6);
				else if (!memcmp (ptr, ".bits ", 6))
					ret = r_asm_pseudo_bits (a, ptr+6);
				else if (!memcmp (ptr, ".fill ", 6))
					ret = r_asm_pseudo_fill (&op, ptr+6);
				else if (!memcmp (ptr, ".hex ", 5))
					ret = r_asm_pseudo_hex (&op, ptr+5);
				else if ((!memcmp (ptr, ".int16 ", 7)) || !memcmp (ptr, ".short ", 7))
					ret = r_asm_pseudo_int16 (a, &op, ptr+7);
				else if (!memcmp (ptr, ".int32 ", 7))
					ret = r_asm_pseudo_int32 (a, &op, ptr+7);
				else if (!memcmp (ptr, ".int64 ", 7))
					ret = r_asm_pseudo_int64 (a, &op, ptr+7);
				else if (!memcmp (ptr, ".size", 5))
					ret = R_TRUE; // do nothing, ignored
				else if (!memcmp (ptr, ".section", 8))
					ret = R_TRUE; // do nothing, ignored
				else if ((!memcmp (ptr, ".byte ", 6)) || (!memcmp (ptr, ".int8 ", 6)))
					ret = r_asm_pseudo_byte (&op, ptr+6);
				else if (!memcmp (ptr, ".glob", 5)) { // .global .globl
				//	eprintf (".global directive not yet implemented\n");
					ret = 0;
					continue;
				} else if (!memcmp (ptr, ".equ ", 5)) {
					ptr2 = strchr (ptr+5, ',');
					if (ptr2) {
						*ptr2 = '\0';
						r_asm_code_set_equ (acode, ptr+5, ptr2+1);
					} else eprintf ("TODO: undef equ\n");
				} else if (!memcmp (ptr, ".org ", 5)) {
					ret = r_asm_pseudo_org (a, ptr+5);
					off = a->pc;
				} else if (!memcmp (ptr, ".text", 5)) {
					acode->code_offset = a->pc;
				} else if (!memcmp (ptr, ".data", 5)) {
					acode->data_offset = a->pc;
				} else {
					eprintf ("Unknown directive (%s)\n", ptr);
					return r_asm_code_free (acode);
				}
				if (!ret)
					continue;
				if (ret < 0) {
					eprintf ("!!! Oops\n");
					return r_asm_code_free (acode);
				}
			} else { /* Instruction */
				char *str = ptr_start;
				ptr_start = r_str_chop (str);
				if (a->ifilter)
					r_parse_parse (a->ifilter, ptr_start, ptr_start);
				if (acode->equs) {
					if (!*ptr_start)
						continue;
					str = r_asm_code_equ_replace (acode, strdup (ptr_start));
					ret = r_asm_assemble (a, &op, str);
					free (str);
				} else {
					if (!*ptr_start)
						continue;
					ret = r_asm_assemble (a, &op, ptr_start);
				}
			}
			if (stage == 2) {
				if (ret < 1) {
					printf ("Cannot assemble '%s'\n", ptr_start);
					return r_asm_code_free (acode);
				}
				acode->len = idx + ret;
				if (!(acode->buf = realloc (acode->buf, (idx+ret)*2)))
					return r_asm_code_free (acode);
				if (!(acode->buf_hex = realloc (acode->buf_hex, (acode->len*2)+1)))
					return r_asm_code_free (acode);
				memcpy (acode->buf+idx, op.buf, ret);
				strcat (acode->buf_hex, op.buf_hex);
			}
		}
	}
	return acode;
}

R_API int r_asm_modify(RAsm *a, ut8 *buf, int field, ut64 val) {
	int ret = R_FALSE;
	if (a->cur && a->cur->modify)
		ret = a->cur->modify (a, buf, field, val);
	return ret;
}

R_API char *r_asm_op_get_hex(RAsmOp *op) {
	return strdup (op->buf_hex);
}

R_API char *r_asm_op_get_asm(RAsmOp *op) {
	return strdup (op->buf_asm);
}

R_API int r_asm_get_offset(RAsm *a, int type, int idx) { // link to rbin
	if (a && a->binb.bin && a->binb.get_offset)
		return a->binb.get_offset (a->binb.bin, type, idx);
	return -1;
}

R_API char *r_asm_describe(RAsm *a, const char* str) {
	if (a->pair)
		return r_pair_get (a->pair, str);
	return NULL;
}
