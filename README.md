# nds32_freertos
---

# simulate

+ set simulator path

    ```
    $ vi ./setting.env
        export PATH=$HOME/.local/nds32_sim:$PATH
    ```

+ set toolchain pathin of `z_nds32_simulator.sh`

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
