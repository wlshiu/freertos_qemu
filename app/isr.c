// See LICENSE for license details.

#include <stddef.h>

/* Trap handeler */
unsigned long _syscall_trap(long cause, long epc, long regs[32])
{
	return epc + 4;
}

void _context_switch_timer(void)
{
	// portSAVE_CONTEXT
	// portSAVE_EPC
	// jal	vPortSysTickHandler
	// portRESTORE_CONTEXT
    return;
}