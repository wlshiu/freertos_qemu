// See LICENSE for license details.

#include <string.h>
#include "device.h"

extern console_device_t     console_ns16550a;
extern poweroff_device_t    poweroff_sifive_test;


void arch_setup()
{
    console_ns16550a.init();
    poweroff_sifive_test.init();
    return;
}

extern char _bss_start;
extern char _bss_end;
extern char _memory_end;

extern int main(void);

__attribute__((noreturn)) void _pre_main()
{
    memset(&_bss_start, 0, &_bss_end - &_bss_start);

    arch_setup();

    // _malloc_addblock(&_bss_end, &_memory_end - &_bss_end);
    main();

    __builtin_unreachable();
}
