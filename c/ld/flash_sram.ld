OUTPUT_FORMAT("elf32-littleriscv")
OUTPUT_ARCH(riscv)

MEMORY
{
	RAM (rwx) : ORIGIN = 0x02000000, LENGTH = 16K
}

ENTRY(_start)

SECTIONS
{
	.init : {
		*(.init);
	} >RAM AT>RAM

	.trap : {
		. = ORIGIN(RAM) + 0xC0;
		KEEP(*(.trap));
	} >RAM AT>RAM

	.text : {
		*(.text);
		*(.text.*);
	} >RAM AT>RAM

	.data : {
		. = ALIGN(4);
		*(.data);
		*(.data.*);
		*(.sdata);
		*(.sdata.*);
		*(.rodata);
		*(.rodata.*);
		_edata = .;
	} >RAM AT>RAM

	.bss (NOLOAD) : {
		. = ALIGN(4);
		_sbss = .;
		*(.bss*);
		*(.sbss*);
		*(COMMON);
		_ebss = .;
	} >RAM AT>RAM

	.stack (NOLOAD) : {
		. = ALIGN(4);
		_sstack = .;
	} >RAM AT>RAM

	PROVIDE(_estack = ORIGIN(RAM) + LENGTH(RAM));
}
