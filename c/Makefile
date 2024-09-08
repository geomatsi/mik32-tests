#
#
#

CROSS_COMPILE ?= riscv32-unknown-elf-
SHARED ?= deps/mik32v2-shared
HAL ?= deps/mik32-hal
BUILD ?= build

CP = $(CROSS_COMPILE)objcopy
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld

ASFLAGS = -march=rv32imc_zicsr_zifencei
CCFLAGS = -march=rv32imc_zicsr_zifencei -mabi=ilp32 -std=gnu99 -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra -Werror -fno-stack-protector -O2
LDFLAGS = -nostdlib --gc-sections --fatal-warnings

# FIXME: suppress warning about RWX SRAM:
LDFLAGS += --no-warn-rwx-segment

CCFLAGS += \
	   -I$(SHARED)/libs \
	   -I$(SHARED)/include \
	   -I$(SHARED)/periphery \
	   -I$(HAL)/core/Include \
	   -I$(HAL)/peripherals/Include

LDFLAGS += -L $(SHARED)/ldscripts -L ld

VPATH += src crt \
	 $(SHARED)/libs \
	 $(SHARED)/runtime \
	 $(HAL)/core/Source \
	 $(HAL)/peripherals/Source \

all:
	@echo "Examples: test1 test2 test3"

# test1: blink test
#  - peripherals: mik32-hal
#  - startup: mik32v2-shared
#  - linker script: mik32v2-shared for RAM

TEST1_ASRCS := crt0.S
TEST1_CSRCS := test1.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c
TEST1_OBJS := $(patsubst %, $(BUILD)/%, $(TEST1_ASRCS:.S=.o))
TEST1_OBJS += $(patsubst %, $(BUILD)/%, $(TEST1_CSRCS:.c=.o))
TEST1_LDS := $(SHARED)/ldscripts/ram.ld

test1: $(TEST1_LDS) $(TEST1_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	$(CP) -O binary $(BUILD)/$@.elf $(BUILD)/$@.bin

# test2: blink test
#  - peripherals: mik32-hal
#  - startup: minimalistic custom for RAM
#  - linker script: minimalistic custom for RAM

TEST2_ASRCS := crt_sram.s
TEST2_CSRCS := test1.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c
TEST2_OBJS := $(patsubst %, $(BUILD)/%, $(TEST2_ASRCS:.s=.o))
TEST2_OBJS += $(patsubst %, $(BUILD)/%, $(TEST2_CSRCS:.c=.o))
TEST2_LDS := ld/sram.ld

test2: $(TEST2_LDS) $(TEST2_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	$(CP) -O binary $(BUILD)/$@.elf $(BUILD)/$@.bin

# test3: trap test
#  - peripherals: mik32-hal
#  - startup: minimalistic custom for RAM
#  - linker script: minimalistic custom for RAM

TEST3_ASRCS := crt_sram.s
TEST3_CSRCS := test3.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c
TEST3_OBJS := $(patsubst %, $(BUILD)/%, $(TEST3_ASRCS:.s=.o))
TEST3_OBJS += $(patsubst %, $(BUILD)/%, $(TEST3_CSRCS:.c=.o))
TEST3_LDS := ld/sram.ld

test3: $(TEST3_LDS) $(TEST3_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	$(CP) -O binary $(BUILD)/$@.elf $(BUILD)/$@.bin

# common

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.s | $(BUILD)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(OPTS) $(CCFLAGS) -c $< -o $@

tags:
	ctags -a -R . $(HAL) $(SHARED)

clean:
	rm -rf $(BUILD)

.PHONY: all clean tags
