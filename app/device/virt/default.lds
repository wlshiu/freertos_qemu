OUTPUT_ARCH( "riscv" )

ENTRY( _start )

MEMORY
{
    ram   (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 128M
}

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x1000;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x1000;


PHDRS
{
  text PT_LOAD;
  data PT_LOAD;
  bss PT_LOAD;
  heap PT_LOAD;
  stack PT_LOAD;
}


SECTIONS
{
    .text : {
        PROVIDE(__text_start = .);
        *boot.o(.text);
        *(.text.init)
        *(.text .text.*)
        PROVIDE(__text_end = .);
    } >ram AT>ram :text

    .rodata : {
        PROVIDE(__rodata_start = .);
        *(.rodata .rodata.*)
        PROVIDE(__rodata_end = .);
    } >ram AT>ram :text

    .data : {
        . = ALIGN(4);
        __gp = . + 0x800;
        PROVIDE(__data_start = .);
        *(.sdata .sdata.*)
        *(.data .data.*)
        PROVIDE(__data_end = .);
    } >ram AT>ram :data

    .bss :{
        . = ALIGN(4);
        PROVIDE(__bss_start = .);
        *(.sbss .sbss.*)
        *(.bss .bss.*)
        . = ALIGN(4);
        PROVIDE(__bss_end = .);
    } >ram AT>ram :bss

    .heap : {
        . = ALIGN(0x10);
        __heap_start = .;
        . += _HEAP_SIZE;
        __heap_end = .;
        . = ALIGN(0x10);
        _heap_end = __heap_end;
    } > ram AT>ram :heap

    .stack : {
        . = ALIGN(0x10);
        __stack_end = .;
        . += _STACK_SIZE;
        . = ALIGN(0x10);
        __stack_top = .;
        __stack = .;
        __freertos_irq_stack_top = .;
    } > ram AT>ram :stack

    _end = .;

    PROVIDE(_memory_start = ORIGIN(ram));
    PROVIDE(_memory_end = ORIGIN(ram) + LENGTH(ram));
}
