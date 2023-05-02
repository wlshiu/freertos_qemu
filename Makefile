
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
# BOARD=STM32F429I-Discovery
BOARD=STM32F4-Discovery
# BOARD=NUCLEO-F072RB


##########
# select CPU
ifeq ($(BOARD), STM32-P107)
ARCH      := ARM_CM3
MCU    	  := STM32F103RB

else ifeq ($(BOARD), NUCLEO-F072RB)
ARCH      := ARM_CM0
MCU    	  := STM32F051R8
else ifeq ($(BOARD), STM32F4-Discovery)
# ARCH      := ARM_CM4F
ARCH      := ARM_CM4F_SoftFP
MCU    	  := STM32F429ZI
endif

##########
# select APP
ifeq ($(ARCH), ARM_CM0)
APP        := demo_m0

else ifeq ($(ARCH), ARM_CM4F_SoftFP)
APP        := demo_m4

endif

# APP        := trace_demo


##########
# common variables
CORES      := 1
INCLUDES   :=
LIBRARY    :=
TARGET_LDS :=
LIBS       :=
PROG_ELF   := $(APP).elf
PROG_BIN   := $(APP).bin

ARCH_FLAGS := -mthumb -mlittle-endian -mthumb-interwork

ifeq ($(ARCH), ARM_CM0)
ARCH_FLAGS += -march=armv6-m -mcpu=cortex-m0
else
ARCH_FLAGS += -march=armv7e-m -mcpu=cortex-m4
endif


DEVICE_SRC_DIR := device/

INCLUDES       := -I$(DEVICE_SRC_DIR) -I$(DEVICE_SRC_DIR)/CMSIS/Core/Include

###########################################
# device
###########################################
ifeq ($(ARCH), ARM_CM0)
DEVICE_SRC := \
	$(DEVICE_SRC_DIR)/STM32F0xx_HAL_Driver/src/stm32f0xx_hal.c \
	$(DEVICE_SRC_DIR)/STM32F0xx_HAL_Driver/src/stm32f0xx_hal_cortex.c \
	$(DEVICE_SRC_DIR)/STM32F0xx_HAL_Driver/src/stm32f0xx_hal_gpio.c \
	$(DEVICE_SRC_DIR)/STM32F0xx_HAL_Driver/src/stm32f0xx_hal_rcc.c \
	$(DEVICE_SRC_DIR)/STM32F0xx_HAL_Driver/src/stm32f0xx_hal_rcc_ex.c \
	$(DEVICE_SRC_DIR)/STM32F0xx_HAL_Driver/src/stm32f0xx_hal_spi.c \
	$(DEVICE_SRC_DIR)/STM32F0xx_HAL_Driver/src/stm32f0xx_hal_i2c.c \
	$(DEVICE_SRC_DIR)/STM32F0xx_HAL_Driver/src/stm32f0xx_hal_dma.c \
	$(DEVICE_SRC_DIR)/CMSIS/ST/STM32F0xx/Source/system_stm32f0xx.c

DEVICE_ASM_SRC = \
	$(DEVICE_SRC_DIR)/CMSIS/ST/STM32F0xx/Source/startup_stm32f072xb.S

INCLUDES += -I$(DEVICE_SRC_DIR)/STM32F0xx_HAL_Driver/inc \
			-I$(DEVICE_SRC_DIR)/CMSIS/ST/STM32F0xx/Include

# TARGET_LDS = $(DEVICE_SRC_DIR)/STM32F072RBTx_FLASH.ld
TARGET_LDS = $(DEVICE_SRC_DIR)/stm32_flash.ld

else ifeq ($(ARCH), ARM_CM3)

DEVICE_SRC := \
	$(DEVICE_SRC_DIR)/STM32F1xx_HAL_Driver/src/stm32f1xx_hal.c \
	$(DEVICE_SRC_DIR)/STM32F1xx_HAL_Driver/src/stm32f1xx_hal_cortex.c \
	$(DEVICE_SRC_DIR)/STM32F1xx_HAL_Driver/src/stm32f1xx_hal_gpio.c \
	$(DEVICE_SRC_DIR)/STM32F1xx_HAL_Driver/src/stm32f1xx_hal_rcc.c \
	$(DEVICE_SRC_DIR)/STM32F1xx_HAL_Driver/src/stm32f1xx_hal_rcc_ex.c \
	$(DEVICE_SRC_DIR)/STM32F1xx_HAL_Driver/src/stm32f1xx_hal_spi.c \
	$(DEVICE_SRC_DIR)/STM32F1xx_HAL_Driver/src/stm32f1xx_hal_dma.c \
	$(DEVICE_SRC_DIR)/CMSIS/ST/STM32F1xx/Source/system_stm32f1xx.c

DEVICE_ASM_SRC = \
	$(DEVICE_SRC_DIR)/CMSIS/ST/STM32F1xx/Source/startup_stm32f103xe.S

INCLUDES += -I$(DEVICE_SRC_DIR)/STM32F1xx_HAL_Driver/inc \
			-I$(DEVICE_SRC_DIR)/CMSIS/ST/STM32F1xx/Include

# TARGET_LDS = $(DEVICE_SRC_DIR)/STM32F072RBTx_FLASH.ld
TARGET_LDS = $(DEVICE_SRC_DIR)/stm32_flash.ld


else ifeq ($(ARCH), ARM_CM4F_SoftFP)
DEVICE_SRC := \
	$(DEVICE_SRC_DIR)/STM32F4xx_HAL_Driver/src/stm32f4xx_hal.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_HAL_Driver/src/stm32f4xx_hal_cortex.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_HAL_Driver/src/stm32f4xx_hal_gpio.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_HAL_Driver/src/stm32f4xx_hal_rcc.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_HAL_Driver/src/stm32f4xx_hal_rcc_ex.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_HAL_Driver/src/stm32f4xx_hal_spi.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_HAL_Driver/src/stm32f4xx_hal_i2c.c \
	$(DEVICE_SRC_DIR)/STM32F4xx_HAL_Driver/src/stm32f4xx_hal_dma.c \
	$(DEVICE_SRC_DIR)/STM32F4_Discovery/stm32f4_discovery.c \
	$(DEVICE_SRC_DIR)/CMSIS/ST/STM32F4xx/Source/system_stm32f4xx.c

DEVICE_ASM_SRC = \
	$(DEVICE_SRC_DIR)/CMSIS/ST/STM32F4xx/Source/startup_stm32f429_439xx.S

INCLUDES += -I$(DEVICE_SRC_DIR)/STM32F4xx_HAL_Driver/inc \
			-I$(DEVICE_SRC_DIR)/STM32F4_Discovery \
			-I$(DEVICE_SRC_DIR)/CMSIS/ST/STM32F4xx/Include

TARGET_LDS = $(DEVICE_SRC_DIR)/stm32_flash.ld
endif # ARM_CM0

####################
## system call
DEVICE_SRC += \
	$(DEVICE_SRC_DIR)/system/syscall/_write.c \
	$(DEVICE_SRC_DIR)/system/syscall/_exit.c \
	$(DEVICE_SRC_DIR)/system/syscall/_sbrk.c \
	$(DEVICE_SRC_DIR)/system/syscall/_syscalls.c

DEVICE_SRC += \
	$(DEVICE_SRC_DIR)/system/trace.c \
	$(DEVICE_SRC_DIR)/system/trace_impl.c

INCLUDES += -I$(DEVICE_SRC_DIR)/system/inc

DEVICE_OBJS := $(DEVICE_SRC:.c=.o) $(DEVICE_ASM_SRC:.S=.o)
###########################################
# app
###########################################
APP_SRC_DIR := app

APP_SRC = $(APP_SRC_DIR)/$(APP)/main.c
INCLUDES  += -I$(APP_SRC_DIR) -I$(APP_SRC_DIR)/$(APP)

ifeq ("$(APP)","demo_m4")
APP_SRC += \
	$(APP_SRC_DIR)/$(APP)/stm32f4xx_hal_msp.c \
	$(APP_SRC_DIR)/$(APP)/stm32f4xx_it.c

else ifeq ("$(APP)","demo_m0")
APP_SRC += \
	$(APP_SRC_DIR)/$(APP)/stm32f0xx_hal_msp.c \
	$(APP_SRC_DIR)/$(APP)/stm32f0xx_it.c

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
# flags
###########################################
LDFLAGS   := -L. -T $(TARGET_LDS) -Wl,-Map,"$(PROG_ELF).map"

LDFLAGS   += $(ARCH_FLAGS)
CFLAGS    := -Wall -MMD -MP $(ARCH_FLAGS) -ffunction-sections -fdata-sections $(INCLUDES)
CFLAGS    += -O0 -g
CFLAGS    += -DUSE_HAL_DRIVER
CFLAGS    += -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DOS_USE_SEMIHOSTING
# CFLAGS    += --specs=nano.specs


ifeq ($(ARCH), ARM_CM3)
CFLAGS    += -DSTM32F407xx

else ifeq ($(ARCH), ARM_CM0)
CFLAGS    += -DSTM32F072xB

else ifeq ($(ARCH), ARM_CM4F_SoftFP)
CFLAGS    += -DSTM32F407xx

CFLAGS    += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
LFLAGS    += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
endif


OBJS := $(DEVICE_OBJS) $(APP_OBJS)

.PHONY: all clean qemu qemu_gdb_server qemu_gdb gtags help

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
	qemu-system-gnuarmeclipse --verbose --verbose --board $(BOARD) --mcu $(MCU) -d unimp,guest_errors --nographic --image $(PROG_ELF) --semihosting-config enable=on,target=native --semihosting-cmdline $(APP) 1 2 3
	@echo ""

qemu_gdb_server: $(PROG_ELF)
	@echo ""
	@echo "Launching QEMU! Press Ctrl-A, X to exit"
	qemu-system-gnuarmeclipse --verbose --verbose --board $(BOARD) --mcu $(MCU) -d unimp,guest_errors --nographic --image $(PROG_ELF) --semihosting-config enable=on,target=native --gdb tcp::1234 -S
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

help:
	@echo "----------------------------------------------------------------------"
	@echo ""
	@echo "  make qemu                  	- Simulate with Qemu"
	@echo "  make qemu_gdb_server           - Run Qemu and wiat GDB client"
	@echo "  make qemu_gdb              	- Start GDB to link GDB server"

