/*
 * Startup code for MIK32 Amur SoC with SCR1 RV32 core
 *
 * Copyright (c) 2023 Mikron
 * Copyright (c) 2024 geomatsi@gmail.com
 */

	.section .init, "ax", @progbits
	.globl _start
	.globl main
	.align 1

_start:
	/* startup loop delay */
	li t0, 128000
1:
	nop
	addi t0, t0, -1
	bnez t0, 1b

	/* setup sp and fp */
	la sp, _estack
	la s0, _estack
	
	/* clear bss section */
	la a0, _sbss
	la a1, _ebss
	bgeu a0, a1, 2f
1:
	sw zero, (a0)
	addi a0, a0, 4
	bltu a0, a1, 1b
2:

	/* fill stack region with 0xaa canaries */
	la a0, _sstack
	la a1, _estack
	bgeu a0, a1, 2f
	li t0, 0xaaaaaaaa
1:
	sw t0, (a0)
	addi a0, a0, 4
	bltu a0, a1, 1b
2:
	/* main */
	jal main

	/* unreachable */
1:
	wfi
	j 1b

	.section .trap, "ax", @progbits
	.weak trap_handler
	.align 1

trap_entry:
	/* FIXME:
	 *  - step over needed only for some traps such as ebreak/ecall
	 *  - now feeling lucky not saving t0 on stack
	 */
	csrr t0, mepc
	addi t0, t0, 4
	csrw mepc, t0

	j trap_handler

trap_handler:
1:
	wfi
	j 1b
