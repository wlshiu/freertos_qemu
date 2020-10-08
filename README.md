# riscv_freertos

Run freertos (v10.2.1) with RISCV of Qemu

# toolchain

[sifive-software](https://www.sifive.com/software) -> `GNU Embedded Toolchain — v2020.04.1`

[xPack GNU RISC-V Embedded GCC](https://github.com/xpack-dev-tools/riscv-none-embed-gcc-xpack/releases/tag/v8.3.0-1.2/)

# qemu

[sifive-software](https://www.sifive.com/software) -> `QEMU — v2020.04.0`


# run

```
$ make
$ make qemu
```

## GDB

```
$ make gdb_server
$ make gdb
(gdb) b _enter
(gdb) c
```

# reference

+ [SiFive Freedom E SDK](https://github.com/sifive/freedom-e-sdk)
+ [FreeRTOS_QEMU](https://github.com/padmaraob/FreeRTOS_QEMU)
+ [FreeRTOS-RISCV-Minres](https://github.com/Minres/FreeRTOS-RISCV)
+ [FreeRTOS-RISCV](https://github.com/kuopinghsu/FreeRTOS-RISCV)
+ [riscv-probe](https://github.com/michaeljclark/riscv-probe)
