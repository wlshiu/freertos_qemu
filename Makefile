
CROSS_COMPILE ?=riscv-none-embed-

AR        = $(CROSS_COMPILE)ar
CC        = $(CROSS_COMPILE)gcc
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf


srctree   := $(shell pwd)

# ARCH      := rv64imc
ARCH      := rv32imc

FREERTOS_SOURCE_DIR := freertos

INCLUDES  = -I$(FREERTOS_SOURCE_DIR)/include \
			-I$(FREERTOS_SOURCE_DIR)/portable/GCC/RISC-V \
			-Iapp/device \
			-Iapp


LIBRARY   :=
KERNEL    := libfreertos.a
PROG      := demo.elf
LIBS      := -lfreertos
CFLAGS    := -O0 -g -Wall $(INCLUDES) -march=$(ARCH) -mcmodel=medany -nostdlib -nostartfiles
LDFLAGS   := -L. -T app/virt/default.lds -march=$(ARCH) -mcmodel=medany -nostdlib -nostartfiles


ifeq ($(ARCH), rv32imc)
QEMU_PLT := riscv32
CFLAGS += -D__riscv_xlen=32
INCLUDES += app/virt/rv32

else ifeq ($(ARCH), rv64imc)
QEMU_PLT := riscv64
CFLAGS += -D__riscv_xlen=64
INCLUDES += app/virt/rv64
endif

FREERTOS_SRC = \
    $(FREERTOS_SOURCE_DIR)/croutine.c \
    $(FREERTOS_SOURCE_DIR)/list.c \
    $(FREERTOS_SOURCE_DIR)/queue.c \
    $(FREERTOS_SOURCE_DIR)/tasks.c \
    $(FREERTOS_SOURCE_DIR)/timers.c \
    $(FREERTOS_SOURCE_DIR)/stream_buffer.c \
    $(FREERTOS_SOURCE_DIR)/event_groups.c \
    $(FREERTOS_SOURCE_DIR)/portable/MemMang/heap_4.c

PORT_SRC = $(FREERTOS_SOURCE_DIR)/portable/GCC/RISC-V/port.c

PORT_ASM = $(FREERTOS_SOURCE_DIR)/portable/GCC/RISC-V/portASM.S

RTOS_OBJ 	 := $(FREERTOS_SRC:.c=.o)
PORT_OBJ 	 := $(PORT_SRC:.c=.o)
PORT_ASM_OBJ := $(PORT_ASM:.S=.o)
KERNEL_OBJS  := $(PORT_ASM_OBJ) $(PORT_OBJ) $(RTOS_OBJ)


APP_SRC = app/main.c app/printf.c app/syscalls.c app/memset.c

DEV_SRC     = app/device/ns16550a.c app/device/sifive_test.c app/device/virt/setup.c
DEV_ASM_SRC = app/device/virt/crtm.s

APP_OBJ 	 := $(APP_SRC:.c=.o)
DEV_OBJ 	 := $(DEV_SRC:.c=.o)
DEV_ASM_OBJ  := $(DEV_ASM_SRC:.s=.o)


all: $(KERNEL) $(PROG)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.S
	$(CC) -c $(CFLAGS) -o $@ $<
	
%.o: %.s
	$(CC) -c $(CFLAGS) -o $@ $<	

$(KERNEL): $(KERNEL_OBJS)
	$(AR) rcs $@ $(KERNEL_OBJS)

%.elf: Makefile
	$(CC) -o $@ $(APP_SRC) $(DEV_SRC) $(CFLAGS) $(LDFLAGS) $(LIBS)
	$(OBJDUMP) -d $@ > $@.dis

qemu: $(PROG)
	qemu-system-$(QEMU_PLT) -M virt -m 128M -nographic -kernel $(PROG)

clean:
	$(RM) -f $(KERNEL_OBJS) $(APP_OBJ) $(DEV_OBJ) $(DEV_ASM_OBJ) $(KERNEL) *.elf *.elf.dis


