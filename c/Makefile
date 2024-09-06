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
	 $(SHARED)/runtime \
	 $(HAL)/core/Source \
	 $(HAL)/peripherals/Source

all:
	@echo "Examples: not yet"

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
