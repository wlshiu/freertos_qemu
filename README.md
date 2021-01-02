# nds32_freertos
---

# simulate

+ set simulator path

    ```
    $ vi ./setting.env
        export PATH=$HOME/.local/nds32_sim:$PATH
    ```

+ set toolchain pathin of `z_nds32_simulator.sh`
    > simulator not supports FPU (toolchain MUST not be nds32le-elf-mculib-v3s)

    ```
    $ vi ./z_nds32_simulator.sh
        toolchain_path=...
    $ chmod +x ./z_nds32_simulator.sh
    ```

+ run simulator

    ```
    $ ./z_nds32_simulator.sh
        et-config.conf
        socketiobase: using fd 4
        socketiobase: server at 0.0.0.0:9898
        GDB init ...
        VEPsocketio: init ...
        socketiobase: using fd 5
        socketiobase: server at 0.0.0.0:9899
    ```

    - The port number of the simulator is `9898`

+ run gdb
    > the simulator is crazy...

    ```
    $ nds32le-elf-gdb demo.bin
    GNU gdb (2017-08-22_nds32le-elf) 7.7.0.20140207-cvs
    Copyright (C) 2014 Free Software Foundation, Inc.
    License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
    and "show warranty" for details.
    This GDB was configured as "--host=i686-pc-linux-gnu --target=nds32le-elf".
    Type "show configuration" for configuration details.
    For bug reporting instructions, please see:
    <http://www.gnu.org/software/gdb/bugs/>.
    Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.
    For help, type "help".
    Type "apropos word" to search for commands related to "word"...
    [info] Loading .Andesgdbinit.
    [info] .Andesgdbinit loaded.
    "/home/wl/working/freertos_qemu/demo.bin": not in executable format: File format not recognized

    (gdb) target remote:9898
    Remote debugging using :9898
    warning: while parsing target description (at line 14): Explicitly sized type can not contain non-bitfield ""
    warning: Could not load XML target description; ignoring
    0x00000000 in ?? ()

    (gdb) load
    Cannot check ELF without executable.
    Use the "file" or "exec-file" command.

    (gdb) file demo.elf
    A program is being debugged already.
    Are you sure you want to change the file? (y or n) y
    Reading symbols from demo.elf...done.

    (gdb) l
    82      within this file. */
    83      void vApplicationMallocFailedHook( void );
    84      void vApplicationIdleHook( void );
    85      void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
    86      void vApplicationTickHook( void );
    87
    88      /*-----------------------------------------------------------*/
    89
    90      int main( void )
    91      {

    (gdb) b main
    Breakpoint 1 at 0x105e08: file app/demo/main.c, line 91.
    (gdb) c
    Continuing.

    Breakpoint 1, main () at app/demo/main.c:91
    91      {
    (gdb)
    ```

    ```
    $ nds32le-elf-gdb demo.elf
    (gdb) target remote:9898
    Remote debugging using:9898
    0x00000000 in ?? ()
    (gdb) load   # load symbols
        sending: "qPart:nds32:request:InvalidateCache"
        received: ""
        sending: "qPart:nds32:request:MemAccBus"
        received: ""
        Loading section .nds32_init, size 0x290 lma 0x100000
        Loading section .text, size 0x7bc4 lma 0x100290
        Loading section .ex9.itable, size 0x394 lma 0x107e54
        Loading section .rodata, size 0x61c lma 0x1081e8
        Loading section .data, size 0x98 lma 0x108820
        Start address 0x100290, load size 34972
        Transfer rate: 214 KB/sec, 230 bytes/write.
        sending: "qPart:nds32:request:MemAccCPU"
        received: ""
    (gdb)
    ```
