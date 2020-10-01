
CROSS_COMPILE ?=riscv-none-embed-

AR        = $(CROSS_COMPILE)ar
CC        = $(CROSS_COMPILE)gcc
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf


srctree   := $(shell pwd)

# ARCH      := rv64imc
ARCH      := rv32imc

FREERTOS_SOURCE_DIR :=

INCLUDES  = -I$(FREERTOS_SOURCE_DIR)/include \
			-I$(FREERTOS_SOURCE_DIR)/portable/GCC/RISC-V


LIBRARY   := libfreertos.a
PROG      := demo.elf
LIBS      := -lfreertos
CFLAGS    := -O0 -g -Wall $(INCLUDES) -march=$(ARCH) -mcmodel=medany -nostdlib -nostartfiles
LDFLAGS   := -L. -T Common/default.ld -march=$(ARCH) -mcmodel=medany -nostdlib -nostartfiles


ifeq ($(ARCH), rv32imc)
QEMU_PLT := riscv32
CFLAGS += -D__riscv_xlen=32
else ifeq ($(ARCH), rv64imc)
QEMU_PLT := riscv32
CFLAGS += -D__riscv_xlen=64
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

APP_SRC =

RTOS_OBJ 	 := $(FREERTOS_SRC:.c=.o)
PORT_OBJ 	 := $(PORT_SRC:.c=.o)
PORT_ASM_OBJ := $(PORT_ASM:.S=.o)
APP_OBJ 	 := $(APP_SRC:.c=.o)
OBJS 	 	 := $(PORT_ASM_OBJ) $(PORT_OBJ) $(RTOS_OBJ)

all: $(LIBRARY) $(PROG)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.S
	$(CC) -c $(CFLAGS) -o $@ $<

$(LIBRARY): $(OBJS)
	$(AR) rcs $@ $(OBJS)

%.elf: Makefile
	$(CC) -o $@ $(@:.elf=.c) $(CFLAGS) $(LDFLAGS) $(LIBS)
	$(OBJDUMP) -d $@ > $@.dis

qemu: $(PROG)
	qemu-system-$(QEMU_PLT) -M virt -m 128M -nographic -kernel $(PROG)

clean:
	$(RM) -f $(OBJS) $(APP_OBJ) $(LIBRARY) examples/*.elf examples/*.elf.dis


