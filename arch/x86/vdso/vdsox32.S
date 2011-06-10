#include <linux/init.h>

__INITDATA

	.globl vdsox32_start, vdsox32_end
vdsox32_start:
	.incbin "arch/x86/vdso/vdsox32.so"
vdsox32_end:

__FINIT
