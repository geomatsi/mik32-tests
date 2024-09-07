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
CCFLAGS = -march=rv32imc_zicsr_zifencei -mabi=ilp32 -std=gnu99 -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra -Werror -fno-stack-protector
LDFLAGS = -nostdlib --gc-sections --fatal-warnings

# FIXME: suppress warning about RWX SRAM:
LDFLAGS += --no-warn-rwx-segment

CCFLAGS += \
	   -I$(SHARED)/include \
	   -I$(SHARED)/periphery \
	   -I$(HAL)/core/Include \
	   -I$(HAL)/peripherals/Include

LDFLAGS += -L $(SHARED)/ldscripts -L ld

VPATH += src crt \
	 $(HAL)/core/Source \
	 $(HAL)/peripherals/Source \
	 $(SHARED)/runtime

all:
	@echo "Examples: test1 test2"

# test1: blink test
#  - peripherals: mik32-hal
#  - startup: mik32-shared
#  - linker script: mik32-shared for RAM

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
TEST2_LDS := ld/flash_sram.ld

test2: $(TEST2_LDS) $(TEST2_OBJS)
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
	ctags -a -R . $(HAL)

clean:
	rm -rf $(BUILD)

.PHONY: all clean tags
