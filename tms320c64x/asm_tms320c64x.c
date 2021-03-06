/* radare2 - LGPL - Copyright 2017 - pancake */

#include <r_asm.h>
#include <r_lib.h>
#include <capstone/capstone.h>

static csh cd = 0;
static int disassemble(RAsm *a, RAsmOp *op, const ut8 *buf, int len) {
	cs_insn* insn;
	int mode, n, ret = -1;
	mode = CS_MODE_BIG_ENDIAN;
	if (!op) {
		return 0;
	}
	// mode |= (a->bits == 64)? CS_MODE_64: CS_MODE_32;
	memset (op, 0, sizeof (RAsmOp));
	op->size = 4;
	if (cd != 0) {
		cs_close (&cd);
	}
	ret = cs_open (CS_ARCH_TMS320C64X, mode, &cd);
	if (ret) {
		goto fin;
	}
	if (a->syntax == R_ASM_SYNTAX_REGNUM) {
		cs_option (cd, CS_OPT_SYNTAX, CS_OPT_SYNTAX_NOREGNAME);
	} else {
		cs_option (cd, CS_OPT_SYNTAX, CS_OPT_SYNTAX_DEFAULT);
	}
	cs_option (cd, CS_OPT_DETAIL, CS_OPT_OFF);
	n = cs_disasm (cd, (ut8*)buf, len, a->pc, 1, &insn);
	if (n < 1) {
		strcpy (op->buf_asm, "invalid");
		op->size = 4;
		goto beach;
	} else {
		ret = 4;
	}
	if (insn->size < 1) {
		goto beach;
	}
	op->size = insn->size;
	snprintf (op->buf_asm, R_ASM_BUFSIZE, "%s%s%s",
		insn->mnemonic, insn->op_str[0]? " ": "",
		insn->op_str);
	// r_str_replace_in (op->buf_asm, sizeof (op->buf_asm), "*+", "", 1);
	// nasty, the disasm output comes with tabs and uppercase :(
	r_str_replace_char (op->buf_asm, '\t', 0);
	// r_str_replace_in (op->buf_asm, sizeof (op->buf_asm), "\t", "", 1);
	r_str_case (op->buf_asm, false);
	cs_free (insn, n);
beach:
	// cs_close (&cd);
fin:
	return op->size;
}

RAsmPlugin r_asm_plugin_tms320c64x_cs = {
	.name = "tms320c64x",
	.desc = "Capstone TMS320 C64X disassembler",
	.license = "BSD",
	.arch = "tms320c64x",
	.cpus = "c64x",
	.bits = 32, // can perform 64 bit loads, but address and regs are 32 bit
	.endian = R_SYS_ENDIAN_BIG, // ???
	.disassemble = &disassemble
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_ASM,
	.data = &r_asm_plugin_tms320c64x_cs,
	.version = R2_VERSION
};
#endif
