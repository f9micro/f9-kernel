/*
 * memory area definitions
 */

INCLUDE "stm32f1/f9_mem.ld"

ENTRY (__l4_start)

/*
 * Memory Layout
 *
 * +-------------------+	0x0000 0000
 * +  isr_vector       +
 * +--------------------	0x0000 1000 (PROC_TABLE_BASE)
 * +  proc_table       |
 * +--------------------	0x0000 2000 (KERNEL_BASE)
 * |  text / data      | 30k
 * ~~~~~~~~~~~~~~~~~~~~~
 *
 * +-------------------+	0x2000 0000
 * +  stack            + 1k
 * +-------------------+	0x2000 0400 (+KERNEL_STACK)
 * +  data / bss       +
 * +-------------------+
 * +..userspace........+
 * +-------------------+
 * + Bitmap /          +
 * + kernel AHB        +
 * +-------------------+
 *
 * +-------------------+	0x4000 0000
 * +  APB mem          +
 * +-------------------+        0x4002 0000
 * +  AHB mem          +
 * +-------------------+
 */

SYSTEM_CALL_BASE	= 0x00000800;
KERNEL_BASE		= 0x00001000;
USERSPACE_BASE		= 0x0000C000;

IDLE_STACK_SIZE		= 0x00000100;
KERNEL_STACK_SIZE	= 0x00000300;
ROOT_THREAD_STACK_SIZE	= 0x00000200;

SECTIONS {
	.text 0x08000000:
	{
		KEEP(*(.isr_vector))
		. = KERNEL_BASE;
		kernel_text_start = .;
		*(.text*)
		*(.rodata*)
		init_hook_start = .;
		KEEP(*(.init_hook))
		init_hook_end = .;
		kernel_text_end = .;
	} > MFlash

	.stack :
	ALIGN (256)
	{
		idle_stack_start = .;
		. += IDLE_STACK_SIZE;
		idle_stack_end = .;
		. += KERNEL_STACK_SIZE;
		kernel_stack_end = .;
		stack_end = .;
	} > RamLoc

	.data :
	AT( ADDR(.text) + SIZEOF(.text) )
	ALIGN (256)
	{
		kernel_data_start = .;
		kip_start = .;
		*(.kip*)
		kip_end = .;
		/* Only kernel data */
		*(.data*)
		kernel_data_end = .;
	} >RamLoc

	/* zero initialized data */
	.bss (NOLOAD) :
	ALIGN (256)
	{
		bss_start = .;
		kernel_bss_start = .;
		*(.bss*)
		*(.ktable*)		/* Statically allocated kernel table */
		*(COMMON)
		kernel_bss_end = .;
		bitmap_bitband_start = .;
		*(.bitmap_bitband*)
		bitmap_bitband_end = .;
		bss_end = .;
	} > RamLoc

	user_text_flash_start = ADDR(.text) + 0xc000;
	.user_text :
	AT ( 0x0800c000 )
	ALIGN (256)
	{
		user_text_start = .;
		*(.user_text*)
		. = ALIGN (256);
		user_text_end = .;
	} > RamLoc
	user_text_flash_end = user_text_flash_start + SIZEOF(.user_text);
	end_of_text = .;

	.user_data :
	AT( 0x0800c000 + SIZEOF(.user_text) )
	ALIGN (256)
	{
		user_data_start = .;
		root_stack_start = .;
		. += ROOT_THREAD_STACK_SIZE;
		root_stack_end = .;
		*(.user_data*)
		user_runtime_start = .;
		KEEP(*(.user_runtime))
		user_runtime_end = .;
		. = ALIGN (256);
		user_data_end = .;
	} > RamLoc

	.user_bss (NOLOAD) :
	ALIGN (256)
	{
		user_bss_start = .;
		*(.user_bss*)
		. = ALIGN (256);
		user_bss_end = .;
	} > RamLoc

	.data_AHB (NOLOAD) :
	{
		kernel_ahb_start = .;
		bitmap_start = .;
		*(.bitmap*)
		bitmap_end = .;
		kernel_ahb_end = .;
		mem0_start = .;
	} > RamLoc

	.sym_tab ( 0x800c000 + SIZEOF(.user_text) + SIZEOF(.user_data) ) :
	{
		symtab_start = .;
		KEEP(*(.sym_tab*))
		symtab_end = .;
	} > MFlash

}
