// See LICENSE for license details.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct console_device 
{
    void (*init)();
    int (*getchar)();
    int (*putchar)(int);
} console_device_t;

typedef struct poweroff_device 
{
    void (*init)();
    void (*poweroff)(int);
} poweroff_device_t;



#ifdef __cplusplus
}
#endif
