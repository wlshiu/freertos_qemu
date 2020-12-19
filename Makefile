
CROSS_COMPILE :=arm-none-eabi-

BUILD_DIR = build

AR        = $(CROSS_COMPILE)ar
CC        = $(CROSS_COMPILE)gcc
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf
GDB       = $(CROSS_COMPILE)gdb


srctree   := $(shell pwd)

###########
# select board
# BOARD=STM32-P107
# BOARD=STM32F4-Discovery
BOARD=STM32F429I-Discovery

##########
# select APP
# APP        := demo
APP        := trace_demo

##########
# select CPU
ARCH      := ARM_CM3
# ARCH      := ARM_CM4F
# ARCH      := ARM_CM4F_SoftFP


CORES      := 1
INCLUDES   :=
LIBRARY    :=
TARGET_LDS :=
LIBS       :=
PROG_ELF   := $(APP).elf
PROG_BIN   := $(APP).bin


ARCH_FLAGS := -march=armv7e-m -mcpu=cortex-m4 -mthumb -mlittle-endian -mthumb-interwork



DEVICE_SRC_DIR := device/

###########################################
# device
###########################################


DEVICE_SRC = \
	$(DEVICE_SRC_DIR)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_StdPeriph_Driver/src/misc.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
	$(DEVICE_SRC_DIR)/STM32F429I_Discovery/stm32f429i_discovery.c \
	$(DEVICE_SRC_DIR)/STM32F4xx/src/stm32f4xx_it.c \
	$(DEVICE_SRC_DIR)/STM32F4xx/src/system_stm32f4xx.c


DEVICE_ASM_SRC = \
	$(DEVICE_SRC_DIR)/STM32F4xx/src/startup_stm32f429_439xx.S


DEVICE_OBJS := $(DEVICE_SRC:.c=.o) $(DEVICE_ASM_SRC:.S=.o)

INCLUDES += -I$(DEVICE_SRC_DIR) \
			-I$(DEVICE_SRC_DIR)/STM32F4xx/inc \
			-I$(DEVICE_SRC_DIR)/STM32F4xx_StdPeriph_Driver/inc \
			-I$(DEVICE_SRC_DIR)/STM32F429I_Discovery \
			-I$(DEVICE_SRC_DIR)/CMSIS/Include

TARGET_LDS = $(DEVICE_SRC_DIR)/stm32_flash.ld
###########################################
# app
###########################################
APP_SRC_DIR := app

APP_SRC = $(APP_SRC_DIR)/$(APP)/main.c
INCLUDES  += -I$(APP_SRC_DIR) -I$(APP_SRC_DIR)/$(APP)

ifeq ("$(APP)","demo")

else ifeq ("$(APP)","trace_demo")

APP_SRC += \
	$(APP_SRC_DIR)/$(APP)/TraceRecorder/trcKernelPort.c \
	$(APP_SRC_DIR)/$(APP)/TraceRecorder/trcSnapshotRecorder.c \
	$(APP_SRC_DIR)/$(APP)/TraceRecorder/trcStreamingRecorder.c

INCLUDES  += -I$(APP_SRC_DIR)/$(APP)/TraceRecorder/include
INCLUDES  += -I$(APP_SRC_DIR)/$(APP)/TraceRecorder/config

endif

APP_OBJS := $(APP_SRC:.c=.o)
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
    $(FREERTOS_SRC_DIR)/portable/MemMang/heap_2.c

# FREERTOS_SRC += $(FREERTOS_SRC_DIR)/portable/Common/mpu_wrappers.c

PORT_SRC = \
	$(FREERTOS_SRC_DIR)/portable/GCC/$(ARCH)/port.c

PORT_ASM =

FREERTOS_OBJS := $(PORT_ASM:.S=.o) $(PORT_SRC:.c=.o) $(FREERTOS_SRC:.c=.o)

INCLUDES  += \
	-I$(FREERTOS_SRC_DIR)/include \
	-I$(FREERTOS_SRC_DIR)/portable/GCC/$(ARCH)

###########################################
# flags
###########################################
LDFLAGS   := -L. -T $(TARGET_LDS) -Wl,-Map,"$(PROG_ELF).map"

LDFLAGS   += $(ARCH_FLAGS)
# LDFLAGS   += -Wl,--start-group -lc -lgcc -lm -Wl,--end-group --specs=nosys.specs
LDFLAGS   += --specs=nosys.specs -O0 -g


CFLAGS    := -Wall -MMD -MP $(ARCH_FLAGS) -ffunction-sections -fdata-sections --specs=nano.specs $(INCLUDES)
CFLAGS    += -O0 -g
CFLAGS    += -DUSE_STDPERIPH_DRIVER -DSTM32F4XX

ifeq ($(ARCH), ARM_CM3)

else ifeq ($(ARCH), ARM_CM4F_SoftFP)
CFLAGS    += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
LFLAGS    += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
endif


OBJS := $(DEVICE_OBJS) $(APP_OBJS)
OBJS += $(FREERTOS_OBJS)

.PHONY: all clean qemu qemu_gdb_server qemu_gdb gtags

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
	qemu-system-gnuarmeclipse --verbose --verbose --board $(BOARD) --mcu STM32F429ZI -d unimp,guest_errors --nographic --image $(PROG_ELF) --semihosting-config enable=on,target=native --semihosting-cmdline $(APP) 1 2 3
	@echo ""

qemu_gdb_server: $(PROG_ELF)
	@echo ""
	@echo "Launching QEMU! Press Ctrl-A, X to exit"
	qemu-system-gnuarmeclipse --verbose --verbose --board $(BOARD) --mcu STM32F429ZI -d unimp,guest_errors --nographic --image $(PROG_ELF) --semihosting-config enable=on,target=native --gdb tcp::1234 -S
	@echo ""


qemu_gdb: $(PROG_ELF)
	@echo ""
	@echo "Start GDB..."
	$(GDB) -ex "target remote:1234" $(PROG_ELF)
	@echo ""


clean:
	@$(RM) -f $(OBJS) $(OBJS:.o=.d) $(FREERTOS_OBJS) $(PROG_ELF) $(PROG_BIN) *.dis *.map
	@find . -type f -name '*.o' -exec $(RM) -f {} \;
	@find . -type f -name '*.d' -exec $(RM) -f {} \;
	@echo "clean done..."

gtags:
	@find . -type f -name '*.c' -o -name '*.h' -o -iname '*.s' > cscope.files
	@rm -f G* tags
	@gtags -f ./cscope.files
