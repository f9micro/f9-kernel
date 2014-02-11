/*
 * memory area definitions
 */

INCLUDE "loader_mem.ld"

ENTRY (__loader_start)

SYSTEM_CALL_BASE	= 0x00000800;
TEXT_BASE		= 0x00001000;
KERNEL_START_OFFSET	= 0xC000;
STACK_SIZE = 0x00000300;

SECTIONS {
	.text 0x08000000:
	{
		KEEP(*(.isr_vector))
		. = TEXT_BASE;
		text_start = .;
		*(.text*)
		*(.rodata*)
		.init_hook_start = .;
		KEEP(*(.init_hook))
		init_hook_end = .;
		text_end = .;
	} > MFlash

	kernel_flash_start = ADDR(.text) + 0xc000;

	.kernel :
	AT(kernel_flash_start)
	{
		kernel_start = .;
		*(.kernel)
		kernel_end = .;
	} > RamLoc

	.data :
	AT( kernel_flash_start + SIZEOF(.kernel))
	{
		data_start = .;
		*(.data*)
		data_end = .;
	} > RamLoc

	.stack (NOLOAD):
	{
		. += STACK_SIZE;
		stack_end = .;
	} > RamLoc

	/* zero initialized data */
	.bss (NOLOAD) :
	{
		bss_start = .;
		*(.bss*)
		*(COMMON)
		bss_end = .;
	} > RamLoc
}
