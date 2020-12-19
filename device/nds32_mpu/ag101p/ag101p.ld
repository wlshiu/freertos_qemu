OUTPUT_FORMAT("elf32-nds32", "elf32-nds32",
	"elf32-nds32")
OUTPUT_ARCH(nds32)
ENTRY(_start)


_mpu_region_align = 0x1000;

_VA_tasks_common = 0x20000000;	/* Region 1 */
_VA_tasks  = 0x40000000;	/* Region 2 */
_VA_stacks = 0x60000000;	/* Region 3 */

SECTIONS
{
	.nds32_init	:	{ *(.nds32_init) }
	.privileged_functions :
	{
		__privileged_functions_start__ = .;
		*start.o (.text*) *startup-nds32.o (.text*) *cache.o (.text*)
		*ag101p.o (.text*) *irq.o (.text*) *FreeRTOS_tick_config.o (.text*)
		*(privileged_functions)
		__privileged_functions_end__ = ALIGN(4);
	}
	
	.privileged_data : 
	{
		__privileged_data_start__ = .;
		*start.o (.data*) *startup-nds32.o (.data*) *cache.o (.data*)
		*ag101p.o (.data*) *irq.o (.data*) *FreeRTOS_tick_config.o (.data*)
		*(privileged_data)
		__privileged_data_end__ = ALIGN(4);
	}
	. = ALIGN(4);
	.nds32.ovly.tbl	:
	{
		_ovly_table = .;
		/* vma */
		/* size of overlay in byte */
		/* lma */
		/* mapped or not ? */
		LONG(ABSOLUTE(ADDR(.task1)));
		LONG(SIZEOF(.task1));
		LONG(ABSOLUTE(LOADADDR(.task1)));
		LONG(0);
		LONG(ABSOLUTE(ADDR(.task2)));
		LONG(SIZEOF(.task2));
		LONG(ABSOLUTE(LOADADDR(.task2)));
		LONG(0);
		LONG(ABSOLUTE(ADDR(.task3)));
		LONG(SIZEOF(.task3));
		LONG(ABSOLUTE(LOADADDR(.task3)));
		LONG(0);
		_novlys = .;
		LONG((_novlys - _ovly_table) / 16);
		. = ALIGN(8);
	}
	__bss_start = ALIGN(0x1000);
	.bss __bss_start	: { *(.bss*)  }
	. = ALIGN(4);
	_end = . ;		/* force bss_end word-align to match our bss clean code*/
	_stack_end = ALIGN(_end,0x1000) + 0x1000 ;
	PROVIDE(_stack = _stack_end );

	/* ovly section start */	
	_tasks_start = ALIGN( _stack_end, _mpu_region_align );
		_ovly_lma_start = _tasks_start;
		__load_start_task1 = _tasks_start;
		.task1 _VA_tasks : AT( _tasks_start ) 
		{ 
			*(.task1_text) 
			*(.task1_rodata) 
		}
		__load_stop_task1 = _tasks_start + SIZEOF(.task1) ;

		__load_start_task2 = ALIGN(__load_stop_task1,0x1000);
		.task2  _VA_tasks : AT (__load_start_task2)
		{ 
			*(.task2_text) 
			*(.task2_rodata) 
		}
		__load_stop_task2 = __load_start_task2 + SIZEOF(.task1) ;

		__load_start_task3 = ALIGN(__load_stop_task2, 0x1000);
		.task3  _VA_tasks : AT (__load_start_task3)
		{ 
			*(.task3_text) 
			*(.task3_rodata) 
		}
		__load_stop_task3 = __load_start_task3 + SIZEOF(.task3);

		_ovly_lma_end = __load_stop_task3;	
	_task1_size = SIZEOF(.task1) ;
	_task2_size = SIZEOF(.task2) ;
	_task3_size = SIZEOF(.task3) ;
	/* ovly section end */
	
	.user_common _VA_tasks_common : 
	AT( 0x05000000 ) 
	{ 
		*(.text*) 
		*(.rodata*) 
		*(.data*) 
		*(.sdata_d)
		*(.sdata_w)
		*(.sdata_h)
		*(.sdata_b)
		*(.sdata_f) . = ALIGN(4);
		/* .tbss */
		*(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon)
		/* .sbss_f */
		*(.sbss_f .sbss_f.*) *(.scommon_f .scommon_f.*)
		/* .sbss_b */
		*(.sbss_b .sbss_b.*) *(.scommon_b .scommon_b.*) . = ALIGN(2);
		/* .sbss_h */
		*(.sbss_h .sbss_h.*) *(.scommon_h .scommon_h.*) . = ALIGN(4);
		/* .sbss_w */
		*(.sbss_w .sbss_w.*) *(.scommon_w .scommon_w.*) *(.dynsbss) *(.scommon) . = ALIGN(8);
		/* .sbss_d */
		*(.sbss_d .sbss_d.*) *(.scommon_d .scommon_d.*)

	}
	_user_common_end = . + 16;

	.stab	0 : { *(.stab) }
	.stabstr	0 : { *(.stabstr) }
	.stab.excl	0 : { *(.stab.excl) }
	.stab.exclstr	0 : { *(.stab.exclstr) }
	.stab.index	0 : { *(.stab.index) }
	.stab.indexstr	0 : { *(.stab.indexstr) }
	.note.nds32	0 : { *(.note.nds32) *(.note.nds32.*) }
	.comment	0 : { *(.comment) }
	.debug	0 : { *(.debug) }
	.line	0 : { *(.line) }
	.debug_srcinfo	0 : { *(.debug_srcinfo) }
	.debug_sfnames	0 : { *(.debug_sfnames) }
	.debug_aranges	0 : { *(.debug_aranges) }
	.debug_pubnames	0 : { *(.debug_pubnames) }
	.debug_info	0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev	0 : { *(.debug_abbrev) }
	.debug_line	0 : { *(.debug_line) }
	.debug_frame	0 : { *(.debug_frame) }
	.debug_str	0 : { *(.debug_str) }
	.debug_loc	0 : { *(.debug_loc) }
	.debug_macinfo	0 : { *(.debug_macinfo) }
	.debug_weaknames	0 : { *(.debug_weaknames) }
	.debug_funcnames	0 : { *(.debug_funcnames) }
	.debug_typenames	0 : { *(.debug_typenames) }
	.debug_varnames	0 : { *(.debug_varnames) }	
}
