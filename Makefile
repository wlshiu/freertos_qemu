
CROSS_COMPILE := nds32le-elf-

CC		:= $(CROSS_COMPILE)gcc
OBJDUMP	:= $(CROSS_COMPILE)objdump
OBJCOPY := $(CROSS_COMPILE)objcopy
AR		:= $(CROSS_COMPILE)ar
AS		:= $(CROSS_COMPILE)as

####################
# Select platform configure
# PLAT: AG101P, AE210P, AE3XX

PLAT := AG101P
# PLAT := AE210P
# PLAT := AE3XX

DEBUG=1
#####################
# Select ADDR: 4GB, 16MB
ADDR ?= 4GB

CONFIG_ENABLE_MPU := 0

ifeq ($(DEBUG),1)
	OPTIM   := -O0 -g3
else
	OPTIM   := -Os -g3
endif



# Define the directories
FREERTOS_SOURCE_DIR	:= freertos
APP_SOURCE_DIR		:= app/common/minimal
BSP_SOURCE_DIR		:= device
DEMO_SOURCE_DIR		:= app

ifneq ($(filter $(PLAT), AG101P AE210P AE100 AE3XX),$(PLAT))
$(error Unknown PLAT "$(PLAT)" is not supported!)
endif

ifneq ($(filter $(ADDR), 4GB 16MB),$(ADDR))
$(error Unknown ADDR "$(ADDR)" is not supported!)
endif

PLATFORM = $(shell echo $(PLAT) | tr A-Z a-z)


ifeq ($(CONFIG_ENABLE_MPU),0)
PROG	:= demo

# The linker script
LDSCRIPT = $(BSP_SOURCE_DIR)/nds32/$(PLATFORM)/$(PLATFORM).ld

# BSP source files
STARTUP_SRCS = $(BSP_SOURCE_DIR)/nds32/start.S

BSP_SRCS = \
	$(BSP_SOURCE_DIR)/nds32/$(PLATFORM)/$(PLATFORM).c \
	$(BSP_SOURCE_DIR)/nds32/$(PLATFORM)/irq.S \
	$(BSP_SOURCE_DIR)/nds32/$(PLATFORM)/uart.c \
	$(BSP_SOURCE_DIR)/nds32/$(PLATFORM)/FreeRTOS_tick_config.c \

DEMO_SRCS = \
	$(DEMO_SOURCE_DIR)/$(PROG)/main.c \
	$(DEMO_SOURCE_DIR)/$(PROG)/blinky_demo/main_blinky.c \
	$(DEMO_SOURCE_DIR)/$(PROG)/full_demo/main_full.c \
	$(DEMO_SOURCE_DIR)/$(PROG)/full_demo/reg_test.S

PORT_SRCS = \
	$(FREERTOS_SOURCE_DIR)/portable/GCC/NDS32/port.c \
	$(FREERTOS_SOURCE_DIR)/portable/GCC/NDS32/portasm.S

INCLUDES := \
	-I$(BSP_SOURCE_DIR)/nds32/$(PLATFORM) \
	-I$(FREERTOS_SOURCE_DIR)/portable/GCC/NDS32

else
##### mpu case

ifneq ($(filter $(PLAT), AG101P AE3XX),$(PLAT))
$(error MPU not support AE210P!)
endif

PROG	:= mpu_demo

# The linker script
LDSCRIPT = $(BSP_SOURCE_DIR)/nds32_mpu/$(PLATFORM)/$(PLATFORM).ld

# BSP source files
STARTUP_SRCS = $(BSP_SOURCE_DIR)/nds32_mpu/start.S

BSP_SRCS = \
	$(BSP_SOURCE_DIR)/nds32_mpu/$(PLATFORM)/$(PLATFORM).c \
	$(BSP_SOURCE_DIR)/nds32_mpu/$(PLATFORM)/irq.S \
	$(BSP_SOURCE_DIR)/nds32_mpu/$(PLATFORM)/uart.c \
	$(BSP_SOURCE_DIR)/nds32_mpu/$(PLATFORM)/FreeRTOS_tick_config.c \

DEMO_SRCS = \
	$(DEMO_SOURCE_DIR)/$(PROG)/main.c

PORT_SRCS = \
	$(FREERTOS_SOURCE_DIR)/portable/GCC/NDS32_MPU/port.c \
	$(FREERTOS_SOURCE_DIR)/portable/GCC/NDS32_MPU/portasm.S

INCLUDES := \
	-I$(BSP_SOURCE_DIR)/nds32_mpu/$(PLATFORM) \
	-I$(FREERTOS_SOURCE_DIR)/portable/GCC/NDS32_MPU

endif



LIBC_SRCS = \
	$(BSP_SOURCE_DIR)/common/startup-nds32.c \
	$(BSP_SOURCE_DIR)/common/cache.c \
	$(BSP_SOURCE_DIR)/common/libc/stdio/do_printf.c \
	$(BSP_SOURCE_DIR)/common/libc/stdio/printf.c

# Kernel source files
FREERTOS_SRCS = \
	$(FREERTOS_SOURCE_DIR)/croutine.c \
	$(FREERTOS_SOURCE_DIR)/list.c \
	$(FREERTOS_SOURCE_DIR)/queue.c \
	$(FREERTOS_SOURCE_DIR)/tasks.c \
	$(FREERTOS_SOURCE_DIR)/timers.c \
	$(FREERTOS_SOURCE_DIR)/event_groups.c \
	$(FREERTOS_SOURCE_DIR)/stream_buffer.c \
	$(FREERTOS_SOURCE_DIR)/portable/MemMang/heap_2.c

ifeq ($(CONFIG_ENABLE_MPU),1)
	FREERTOS_SRCS += $(FREERTOS_SOURCE_DIR)/portable/Common/mpu_wrappers.c
endif

RTOS_SRCS = $(FREERTOS_SRCS) $(PORT_SRCS)

# Demo source files
APP_SRCS = \
	$(APP_SOURCE_DIR)/AbortDelay.c \
	$(APP_SOURCE_DIR)/BlockQ.c \
	$(APP_SOURCE_DIR)/blocktim.c \
	$(APP_SOURCE_DIR)/countsem.c \
	$(APP_SOURCE_DIR)/death.c \
	$(APP_SOURCE_DIR)/dynamic.c \
	$(APP_SOURCE_DIR)/EventGroupsDemo.c \
	$(APP_SOURCE_DIR)/flop.c \
	$(APP_SOURCE_DIR)/GenQTest.c \
	$(APP_SOURCE_DIR)/IntSemTest.c \
	$(APP_SOURCE_DIR)/MessageBufferAMP.c \
	$(APP_SOURCE_DIR)/QueueOverwrite.c \
	$(APP_SOURCE_DIR)/recmutex.c \
	$(APP_SOURCE_DIR)/semtest.c \
	$(APP_SOURCE_DIR)/StaticAllocation.c \
	$(APP_SOURCE_DIR)/MessageBufferDemo.c \
	$(APP_SOURCE_DIR)/StreamBufferInterrupt.c \
	$(APP_SOURCE_DIR)/TaskNotify.c \
	$(APP_SOURCE_DIR)/TimerDemo.c


# Define all object files.
SRCS = $(STARTUP_SRCS) $(BSP_SRCS) $(LIBC_SRCS) $(RTOS_SRCS) $(APP_SRCS) $(DEMO_SRCS)
OBJS = $(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,${SRCS})))

### Verbosity control. Use 'make V=1' to get verbose builds.

ifeq ($(V),1)
TRACE_CC  =
TRACE_C++ =
TRACE_LD  =
TRACE_AR  =
TRACE_AS  =
Q=
else
TRACE_CC  = @echo "  CC       " $<
TRACE_C++ = @echo "  C++      " $<
TRACE_LD  = @echo "  LD       " $@
TRACE_AR  = @echo "  AR       " $@
TRACE_AS  = @echo "  AS       " $<
Q=@
endif

# Compilation options
CMODEL = -mcmodel=large

INCLUDES += \
	-I. \
	-I$(DEMO_SOURCE_DIR)/common/include \
	-I$(APP_SOURCE_DIR) \
	-I$(DEMO_SOURCE_DIR)/$(PROG) \
	-I$(BSP_SOURCE_DIR)/common \
	-I$(FREERTOS_SOURCE_DIR)/include


CFLAGS = \
	-Wall $(OPTIM) $(INCLUDES) $(DEFINES) \
	-fomit-frame-pointer -fno-strict-aliasing -fno-builtin -funroll-loops \
	-ffunction-sections -fdata-sections \
	-include $(PLATFORM).h \
	$(CMODEL)

ASFLAGS = -D__ASSEMBLY__ $(CFLAGS) -c

LDFLAGS = -T$(LDSCRIPT) $(OPTIM) -static -nostartfiles -Wl,--gc-sections,--undefine=__rtos_signature_freertos_v10_1_1 \
	$(CMODEL)

GCCVER 	= $(shell $(GCC) --version | grep gcc | cut -d" " -f9)

# Add `-fno-delete-null-pointer-checks` flag if the compiler supports it.
# GCC assumes that programs cannot safely dereference null pointers,
# and that no code or data element resides there.
# However, 0x0 is the vector table memory location, so the test must not be removed.
ifeq ($(shell $(CC) -fno-delete-null-pointer-checks -E - 2>/dev/null >/dev/null </dev/null ; echo $$?),0)
	CFLAGS += -fno-delete-null-pointer-checks
	LDFLAGS += -fno-delete-null-pointer-checks
endif

### Make variables

DEFINES = -DCFG_$(ADDR)

ifneq ($(shell echo | $(CC) -E -dM - | grep __NDS32_ISA_V3M__ > /dev/null && echo V3M),V3M)
	ifeq ($(USE_CACHEWB),1)
		DEFINES += -DCFG_CACHE_ENABLE -DCFG_CACHE_WRITEBACK
	endif

	ifeq ($(USE_CACHEWT),1)
		DEFINES += -DCFG_CACHE_ENABLE -DCFG_CACHE_WRITETHROUGH
	endif
endif

# Compilation rules
.SUFFIXES : %.o %.c %.cpp %.S

%.o: %.c
	$(TRACE_CC)
	$(Q)$(CC) -c -MMD $(CFLAGS) -o $@ $<

%.o: %.S
	$(TRACE_CC)
	$(Q)$(CC) -c -MMD $(CFLAGS) -o $@ $<

all: $(PROG).elf

$(PROG).elf  : $(OBJS) Makefile
	$(TRACE_LD)
	$(Q)$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)
	$(OBJCOPY) -O binary -S $(PROG).elf $(PROG).bin
	$(OBJDUMP) -S $(PROG).elf > $(PROG).dis
	@echo Completed

clean :
	@rm -f $(OBJS) $(OBJS:.o=.d)
	@rm -f $(PROG).elf
	@rm -f $(PROG).bin
	@rm -f $(PROG).dis

# Automatic dependency generation
ifneq ($(MAKECMDGOALS),clean)
-include $(OBJS:.o=.d)
endif
