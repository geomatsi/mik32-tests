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

CCFLAGS += -Icommon/include \
	   -I$(SHARED)/libs \
	   -I$(SHARED)/include \
	   -I$(SHARED)/periphery \
	   -I$(HAL)/core/Include \
	   -I$(HAL)/peripherals/Include

LDFLAGS += -L $(SHARED)/ldscripts -L ld

VPATH += src crt common/src \
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
TEST1_CSRCS := test1.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c elbear_init.c
TEST1_OBJS := $(patsubst %, $(BUILD)/%, $(TEST1_ASRCS:.S=.o))
TEST1_OBJS += $(patsubst %, $(BUILD)/%, $(TEST1_CSRCS:.c=.o))
TEST1_LDS := $(SHARED)/ldscripts/ram.ld

test1: $(TEST1_LDS) $(TEST1_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	$(CP) -O binary $(BUILD)/$@.elf $(BUILD)/$@.bin

# test2: blink and uart test
#  - peripherals: mik32-hal
#  - xprintf: mik32v2-shared
#  - startup: minimalistic custom for RAM
#  - linker script: minimalistic custom for RAM

TEST2_ASRCS := crt_sram.s
TEST2_CSRCS := test2.c xprintf.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c mik32_hal_usart.c elbear_init.c
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
TEST3_CSRCS := test3.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c elbear_init.c
TEST3_OBJS := $(patsubst %, $(BUILD)/%, $(TEST3_ASRCS:.s=.o))
TEST3_OBJS += $(patsubst %, $(BUILD)/%, $(TEST3_CSRCS:.c=.o))
TEST3_LDS := ld/sram.ld

test3: $(TEST3_LDS) $(TEST3_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	$(CP) -O binary $(BUILD)/$@.elf $(BUILD)/$@.bin

# test4: enumerate debug triggers
#  - peripherals: mik32-hal
#  - xprintf: mik32v2-shared
#  - startup: minimalistic custom for RAM
#  - linker script: minimalistic custom for RAM

TEST4_ASRCS := crt_sram.s
TEST4_CSRCS := test4.c xprintf.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c mik32_hal_usart.c elbear_init.c
TEST4_OBJS := $(patsubst %, $(BUILD)/%, $(TEST4_ASRCS:.s=.o))
TEST4_OBJS += $(patsubst %, $(BUILD)/%, $(TEST4_CSRCS:.c=.o))
TEST4_LDS := ld/sram.ld

test4: $(TEST4_LDS) $(TEST4_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	$(CP) -O binary $(BUILD)/$@.elf $(BUILD)/$@.bin

# test5: h/w watchpoints
#  - peripherals: mik32-hal
#  - xprintf: mik32v2-shared
#  - startup: minimalistic custom for RAM
#  - linker script: minimalistic custom for RAM

TEST5_ASRCS := crt_sram.s
TEST5_CSRCS := test5.c xprintf.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c mik32_hal_usart.c elbear_init.c
TEST5_OBJS := $(patsubst %, $(BUILD)/%, $(TEST5_ASRCS:.s=.o))
TEST5_OBJS += $(patsubst %, $(BUILD)/%, $(TEST5_CSRCS:.c=.o))
TEST5_LDS := ld/sram.ld

test5: $(TEST5_LDS) $(TEST5_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	$(CP) -O binary $(BUILD)/$@.elf $(BUILD)/$@.bin

# test6: h/w breakpoints
#  - peripherals: mik32-hal
#  - xprintf: mik32v2-shared
#  - startup: minimalistic custom for RAM
#  - linker script: minimalistic custom for RAM

TEST6_ASRCS := crt_sram.s
TEST6_CSRCS := test6.c xprintf.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c mik32_hal_usart.c elbear_init.c
TEST6_OBJS := $(patsubst %, $(BUILD)/%, $(TEST6_ASRCS:.s=.o))
TEST6_OBJS += $(patsubst %, $(BUILD)/%, $(TEST6_CSRCS:.c=.o))
TEST6_LDS := ld/sram.ld

test6: $(TEST6_LDS) $(TEST6_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	$(CP) -O binary $(BUILD)/$@.elf $(BUILD)/$@.bin

# test7: icount h/w trigger
#  - peripherals: mik32-hal
#  - xprintf: mik32v2-shared
#  - startup: minimalistic custom for RAM
#  - linker script: minimalistic custom for RAM

TEST7_ASRCS := crt_sram.s
TEST7_CSRCS := test7.c xprintf.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c mik32_hal_usart.c elbear_init.c
TEST7_OBJS := $(patsubst %, $(BUILD)/%, $(TEST7_ASRCS:.s=.o))
TEST7_OBJS += $(patsubst %, $(BUILD)/%, $(TEST7_CSRCS:.c=.o))
TEST7_LDS := ld/sram.ld

test7: $(TEST7_LDS) $(TEST7_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	$(CP) -O binary $(BUILD)/$@.elf $(BUILD)/$@.bin

# test8: stack overflow for recursive trap handler
#  - peripherals: mik32-hal
#  - xprintf: mik32v2-shared
#  - startup: minimalistic custom for RAM
#  - linker script: minimalistic custom for RAM

TEST8_ASRCS := crt_sram.s
TEST8_CSRCS := test8.c xprintf.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c mik32_hal_usart.c elbear_init.c
TEST8_OBJS := $(patsubst %, $(BUILD)/%, $(TEST8_ASRCS:.s=.o))
TEST8_OBJS += $(patsubst %, $(BUILD)/%, $(TEST8_CSRCS:.c=.o))
TEST8_LDS := ld/sram.ld

test8: $(TEST8_LDS) $(TEST8_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	$(CP) -O binary $(BUILD)/$@.elf $(BUILD)/$@.bin

# test9: handling interrupts
#  - peripherals: mik32-hal
#  - xprintf: mik32v2-shared
#  - startup: minimalistic custom for RAM
#  - linker script: minimalistic custom for RAM

TEST9_ASRCS := crt_sram.s
TEST9_CSRCS := test9.c xprintf.c mik32_hal.c mik32_hal_pcc.c mik32_hal_gpio.c mik32_hal_usart.c mik32_hal_irq.c elbear_init.c
TEST9_OBJS := $(patsubst %, $(BUILD)/%, $(TEST9_ASRCS:.s=.o))
TEST9_OBJS += $(patsubst %, $(BUILD)/%, $(TEST9_CSRCS:.c=.o))
TEST9_LDS := ld/sram.ld

test9: $(TEST9_LDS) $(TEST9_OBJS)
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
