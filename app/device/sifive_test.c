// See LICENSE for license details.

#include "device.h"

#define SIFIVE_TEST_CTRL_ADDR       0x100000

enum {
	SIFIVE_TEST_FAIL = 0x3333,
	SIFIVE_TEST_PASS = 0x5555,
};

static volatile unsigned int *test;

static void sifive_test_init()
{
	test = (unsigned int *)(void *)SIFIVE_TEST_CTRL_ADDR;
}

static void sifive_test_poweroff(int status)
{
    *test = SIFIVE_TEST_PASS;
    while (1) {
        asm volatile("");
    }
}

poweroff_device_t   poweroff_sifive_test = {
	sifive_test_init,
	sifive_test_poweroff
};
