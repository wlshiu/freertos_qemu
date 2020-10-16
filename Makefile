
CROSS_COMPILE ?=riscv-none-embed-

BUILD_DIR = build

AR        = $(CROSS_COMPILE)-ar
CC        = $(CROSS_COMPILE)-gcc
OBJDUMP   = $(CROSS_COMPILE)-objdump
OBJCOPY   = $(CROSS_COMPILE)-objcopy
READELF   = $(CROSS_COMPILE)-readelf
GDB       = $(CROSS_COMPILE)-gdb


srctree   := $(shell pwd)

###########
# select board
# BOARD=sifive_e
BOARD=sifive_u

##########
# select APP
# APP        := hello
APP        := blinky
# APP        := pmp_blinky

CORES      := 1
INCLUDES   :=
LIBRARY    :=
TARGET_LDS :=
LIBS       :=
PROG_ELF   := $(APP).elf
PROG_BIN   := $(APP).bin


ifeq ("$(BOARD)", "sifive_u")
ARCH       := rv64imafdc
ARCH_FLAGS := -march=$(ARCH) -mabi=lp64d -mcmodel=medany
QEMU_PLT   := riscv64
ARCH_FLAGS += -D__riscv_xlen=64

CORES          := 4
DEVICE_SRC_DIR := device/sifive_u54mc

else ifeq ("$(BOARD)", "sifive_e")
ARCH       := rv32imac
ARCH_FLAGS := -march=$(ARCH) -mabi=ilp32 -mcmodel=medlow
QEMU_PLT   := riscv32
ARCH_FLAGS += -D__riscv_xlen=32

DEVICE_SRC_DIR := device/sifive_e31

else
$(error BOARD is sifive_u or sifive_e ?)

endif




###########################################
# device
###########################################

GLOSS_SRC = \
	$(DEVICE_SRC_DIR)/gloss/nanosleep.c           \
	$(DEVICE_SRC_DIR)/gloss/sys_access.c          \
	$(DEVICE_SRC_DIR)/gloss/sys_chdir.c           \
	$(DEVICE_SRC_DIR)/gloss/sys_chmod.c           \
	$(DEVICE_SRC_DIR)/gloss/sys_chown.c           \
	$(DEVICE_SRC_DIR)/gloss/sys_clock_gettime.c   \
	$(DEVICE_SRC_DIR)/gloss/sys_close.c           \
	$(DEVICE_SRC_DIR)/gloss/sys_execve.c          \
	$(DEVICE_SRC_DIR)/gloss/sys_exit.c            \
	$(DEVICE_SRC_DIR)/gloss/sys_faccessat.c       \
	$(DEVICE_SRC_DIR)/gloss/sys_fork.c            \
	$(DEVICE_SRC_DIR)/gloss/sys_fstatat.c         \
	$(DEVICE_SRC_DIR)/gloss/sys_fstat.c           \
	$(DEVICE_SRC_DIR)/gloss/sys_ftime.c           \
	$(DEVICE_SRC_DIR)/gloss/sys_getcwd.c          \
	$(DEVICE_SRC_DIR)/gloss/sys_getpid.c          \
	$(DEVICE_SRC_DIR)/gloss/sys_gettimeofday.c    \
	$(DEVICE_SRC_DIR)/gloss/sys_isatty.c          \
	$(DEVICE_SRC_DIR)/gloss/sys_kill.c            \
	$(DEVICE_SRC_DIR)/gloss/sys_link.c            \
	$(DEVICE_SRC_DIR)/gloss/sys_lseek.c           \
	$(DEVICE_SRC_DIR)/gloss/sys_lstat.c           \
	$(DEVICE_SRC_DIR)/gloss/sys_openat.c          \
	$(DEVICE_SRC_DIR)/gloss/sys_open.c            \
	$(DEVICE_SRC_DIR)/gloss/sys_read.c            \
	$(DEVICE_SRC_DIR)/gloss/sys_sbrk.c            \
	$(DEVICE_SRC_DIR)/gloss/sys_stat.c            \
	$(DEVICE_SRC_DIR)/gloss/sys_sysconf.c         \
	$(DEVICE_SRC_DIR)/gloss/sys_times.c           \
	$(DEVICE_SRC_DIR)/gloss/sys_unlink.c          \
	$(DEVICE_SRC_DIR)/gloss/sys_utime.c           \
	$(DEVICE_SRC_DIR)/gloss/sys_wait.c            \
	$(DEVICE_SRC_DIR)/gloss/sys_write.c           \

SIFIVE_SRC = \
	$(DEVICE_SRC_DIR)/src/drivers/fixed-clock.c                        \
	$(DEVICE_SRC_DIR)/src/drivers/fixed-factor-clock.c                 \
	$(DEVICE_SRC_DIR)/src/drivers/inline.c                             \
	$(DEVICE_SRC_DIR)/src/drivers/riscv_clint0.c                       \
	$(DEVICE_SRC_DIR)/src/drivers/riscv_cpu.c                          \
	$(DEVICE_SRC_DIR)/src/drivers/riscv_plic0.c                        \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_buserror0.c                   \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_ccache0.c                     \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_clic0.c                       \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_fe310-g000_hfrosc.c           \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_fe310-g000_hfxosc.c           \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_fe310-g000_lfrosc.c           \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_fe310-g000_pll.c              \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_fe310-g000_prci.c             \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_global-external-interrupts0.c \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_gpio0.c                       \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_gpio-buttons.c                \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_gpio-leds.c                   \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_gpio-switches.c               \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_i2c0.c                        \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_l2pf0.c                       \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_local-external-interrupts0.c  \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_pwm0.c                        \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_rtc0.c                        \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_simuart0.c                    \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_spi0.c                        \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_test0.c                       \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_trace.c                       \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_uart0.c                       \
	$(DEVICE_SRC_DIR)/src/drivers/sifive_wdog0.c                       \
	$(DEVICE_SRC_DIR)/src/drivers/ucb_htif0.c


DEVICE_SRC = \
	$(SIFIVE_SRC)							  \
	$(DEVICE_SRC_DIR)/src/atomic.c            \
	$(DEVICE_SRC_DIR)/src/button.c            \
	$(DEVICE_SRC_DIR)/src/cache.c             \
	$(DEVICE_SRC_DIR)/src/clock.c             \
	$(DEVICE_SRC_DIR)/src/cpu.c               \
	$(DEVICE_SRC_DIR)/src/gpio.c              \
	$(DEVICE_SRC_DIR)/src/hpm.c               \
	$(DEVICE_SRC_DIR)/src/i2c.c               \
	$(DEVICE_SRC_DIR)/src/init.c              \
	$(DEVICE_SRC_DIR)/src/interrupt.c         \
	$(DEVICE_SRC_DIR)/src/led.c               \
	$(DEVICE_SRC_DIR)/src/lock.c              \
	$(DEVICE_SRC_DIR)/src/memory.c            \
	$(DEVICE_SRC_DIR)/src/pmp.c               \
	$(DEVICE_SRC_DIR)/src/privilege.c         \
	$(DEVICE_SRC_DIR)/src/pwm.c               \
	$(DEVICE_SRC_DIR)/src/rtc.c               \
	$(DEVICE_SRC_DIR)/src/shutdown.c          \
	$(DEVICE_SRC_DIR)/src/spi.c               \
	$(DEVICE_SRC_DIR)/src/switch.c            \
	$(DEVICE_SRC_DIR)/src/synchronize_harts.c \
	$(DEVICE_SRC_DIR)/src/time.c              \
	$(DEVICE_SRC_DIR)/src/timer.c             \
	$(DEVICE_SRC_DIR)/src/tty.c               \
	$(DEVICE_SRC_DIR)/src/uart.c              \
	$(DEVICE_SRC_DIR)/src/watchdog.c	      \
	$(GLOSS_SRC)

DEVICE_ASM_SRC = \
	$(DEVICE_SRC_DIR)/gloss/crt0.S \
	$(DEVICE_SRC_DIR)/src/entry.S  \
	$(DEVICE_SRC_DIR)/src/trap.S   \
	$(DEVICE_SRC_DIR)/src/scrub.S  \
	$(DEVICE_SRC_DIR)/src/vector.S

DEVICE_OBJS := $(DEVICE_SRC:.c=.o) $(DEVICE_ASM_SRC:.S=.o)

INCLUDES += -I$(DEVICE_SRC_DIR)

TARGET_LDS = $(DEVICE_SRC_DIR)/metal.default.lds
###########################################
# app
###########################################
APP_SRC_DIR := app

ifeq ("$(APP)","hello")
APP_SRC = $(APP_SRC_DIR)/$(APP)/hello.c

else ifeq ("$(APP)","blinky")
APP_SRC = \
	$(APP_SRC_DIR)/$(APP)/Bridge_Freedom-metal_FreeRTOS.c \
	$(APP_SRC_DIR)/$(APP)/blinky.c

TARGET_LDS = $(DEVICE_SRC_DIR)/metal.freertos.lds

else ifeq ("$(APP)","pmp_blinky")
APP_SRC = \
	$(APP_SRC_DIR)/$(APP)/Bridge_Freedom-metal_FreeRTOS.c \
	$(APP_SRC_DIR)/$(APP)/blinky_pmp.c

TARGET_LDS = $(DEVICE_SRC_DIR)/metal.freertos.lds

endif


APP_OBJS := $(APP_SRC:.c=.o)

INCLUDES  += -I$(APP_SRC_DIR) -I$(APP_SRC_DIR)/$(APP)
###########################################
# freertos
###########################################
FREERTOS_SRC_DIR := freertos

FREERTOS_SRC = \
    $(FREERTOS_SRC_DIR)/croutine.c \
    $(FREERTOS_SRC_DIR)/list.c \
    $(FREERTOS_SRC_DIR)/queue.c \
    $(FREERTOS_SRC_DIR)/tasks.c \
    $(FREERTOS_SRC_DIR)/timers.c \
    $(FREERTOS_SRC_DIR)/event_groups.c \
	$(FREERTOS_SRC_DIR)/stream_buffer.c \
    $(FREERTOS_SRC_DIR)/portable/MemMang/heap_4.c

# FREERTOS_SRC += $(FREERTOS_SRC_DIR)/portable/Common/mpu_wrappers.c

PORT_SRC = \
	$(FREERTOS_SRC_DIR)/portable/GCC/RISC-V/pmp.c \
	$(FREERTOS_SRC_DIR)/portable/GCC/RISC-V/port.c

PORT_ASM = $(FREERTOS_SRC_DIR)/portable/GCC/RISC-V/portASM.S

FREERTOS_OBJS := $(PORT_ASM:.S=.o) $(PORT_SRC:.c=.o) $(FREERTOS_SRC:.c=.o)

INCLUDES  += \
	-I$(FREERTOS_SRC_DIR)/include \
	-I$(FREERTOS_SRC_DIR)/portable/GCC/RISC-V \
	-I$(FREERTOS_SRC_DIR)/portable/GCC/RISC-V/chip_specific_extensions/RV32I_CLINT_no_extensions

###########################################
# flags
###########################################
LDFLAGS   := -L. -T $(TARGET_LDS) -Wl,-Map,"$(PROG_ELF).map" -nostartfiles
# LDFLAGS   := -L. -T $(LDS) -march=$(ARCH) -static -nostdlib -nostartfiles

LDFLAGS   += $(ARCH_FLAGS)
LDFLAGS   += -Wl,--start-group -lc -lgcc -lm -Wl,--end-group --specs=nano.specs
LDFLAGS   += -Wl,--defsym,__stack_size=0x200 -Wl,--defsym,__heap_size=0x200

CFLAGS    := -Wall -MMD -MP $(ARCH_FLAGS) -ffunction-sections -fdata-sections --specs=nano.specs $(INCLUDES)
CFLAGS    += -DWAIT_MS=1000
# CFLAGS    += -DMTIME_RATE_HZ_DEF=10000000 -DMETAL_WAIT_CYCLE=0
# CFLAGS    +=  -D__ASSEMBLY__  -DITERATIONS=5000 -DDHRY_ITERS=20000000

CFLAGS    += -O0 -g


OBJS := $(DEVICE_OBJS) $(APP_OBJS)


ifneq ("$(APP)","hello")
OBJS += $(FREERTOS_OBJS)
endif

.PHONY: all clean qemu gdb_server gdb gtags

all: $(PROG_ELF) $(PROG_BIN)

%.o: %.c
	@echo "    CC $<"
	@$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.S
	@echo "    CC $<"
	@$(CC) -c $(CFLAGS) -o $@ $<

-include $(OBJS:.o=.d)

$(PROG_ELF): $(OBJS) Makefile
	@echo Linking....
	@$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)
	@$(OBJDUMP) -d $@ > $@.dis
	@echo Completed $@

$(PROG_BIN): $(PROG_ELF)
	@$(OBJCOPY) -O binary $< $@

qemu: $(PROG_ELF)
	@echo ""
	@echo "Launching QEMU! Press Ctrl-A, X to exit"
ifeq ("$(CORES)", "1")
	qemu-system-$(QEMU_PLT) -M $(BOARD) -nographic -kernel $(PROG_ELF)
else
	qemu-system-$(QEMU_PLT) -M $(BOARD) -nographic -smp $(CORES) -kernel $(PROG_ELF)
endif
	@echo ""

gdb_server: $(PROG_ELF)
	@echo ""
	@echo "Launching QEMU! Press Ctrl-A, X to exit"
ifeq ("$(CORES)", "1")
	qemu-system-$(QEMU_PLT) -M $(BOARD) -nographic -kernel $(PROG_ELF) -S -s
else
	qemu-system-$(QEMU_PLT) -M $(BOARD) -nographic -smp $(CORES) -kernel $(PROG_ELF) -S -s
endif
	@echo ""

gdb: $(PROG_ELF)
	$(GDB) $(PROG_ELF) --command=init.gdb

clean:
	@$(RM) -f $(OBJS) $(OBJS:.o=.d) $(FREERTOS_OBJS) $(PROG_ELF) $(PROG_BIN) *.dis *.map
	@find . -type f -name '*.o' -exec $(RM) -f {} \;
	@find . -type f -name '*.d' -exec $(RM) -f {} \;
	@echo "clean done..."

gtags:
	@find . -type f -name '*.c' -o -name '*.h' -o -iname '*.s' > cscope.files
	@rm -f G* tags
	@gtags -f ./cscope.files
