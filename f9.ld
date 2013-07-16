/*
 * memory area definitions 
 */

INCLUDE "f9_mem.ld"

ENTRY (__l4_start)

/*
 * Memory Relocation 
 * 
 * +-------------------+	0x000000000 
 * +  isr_vectors      + 
 * +--------------------	0x000001000 (PROC_TABLE_BASE)
 * +  proc_table       |
 * +--------------------	0x000002000 (KERNEL_BASE)
 * +  text / data      + 30k
 * +--------------------	0x000008000 (USERSPACE_BASE)
 * |....Userspace......|
 * ~~~~~~~~~~~~~~~~~~~~~
 *
 * +-------------------+	0x100000000 
 * +  stack            + 2k	
 * +-------------------+	0x100000800 (+KERNEL_STACK)
 * +  data / bss       + 10k
 * ~~~~~~~~~~~~~~~~~~~~~	0x100003000 (+KERNEL_DATA)
 * |....Userspace......|
 * ~~~~~~~~~~~~~~~~~~~~~	0x10007c000 (+USER_DATA) 
 * +  KIP              + 1k
 * +-------------------+
 *
 * 
 * +-------------------+	0x2007C000
 * +  AHB mem          +   
 * +-------------------+
 */

SYSTEM_CALL_BASE	= 0x00000800;
KERNEL_BASE		= 0x00001000;
USERSPACE_BASE		= 0x00000C000;

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
		kernel_text_end = .;
	} > MFlash
	
	.user_text 0x0800C000 : 
	{
		user_text_start = .;
		*root_thread.o(.user_text*)
		. = 0x00001000;
		user_text_end = .;
	} > MFlash
	
	.stack :
	{
		idle_stack_start = .;
		. += IDLE_STACK_SIZE;
		idle_stack_end = .;
		. += KERNEL_STACK_SIZE;
		kernel_stack_end = .;
	} > RamLoc
	
	.data :
	AT( ADDR(.text) + SIZEOF(.text) )
	ALIGN (128) 
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
	{
		kernel_bss_start = .;
		*(.bss*)
	    *(.ktable*)			/* Statically allocated kernel table */
		*(COMMON)
		kernel_bss_end = .;
	} > RamLoc
	
	.user_data : 
	AT( ADDR(.user_text) + SIZEOF(.user_text) )
	ALIGN (128)
	{
		user_data_start = .;
		root_stack_start = .;
		. += ROOT_THREAD_STACK_SIZE;
		root_stack_end = .;
		*(.user_data*)
		user_data_end = .;
	} > RamLoc 
	
	.user_bss (NOLOAD) :
	ALIGN (128)
	{
		user_bss_start = .;
		*(.user_bss*)
		user_bss_end = .;
	} > RamLoc

	.data_AHB (NOLOAD) :
	{
		kernel_ahb_start = .;
		*(.bitmap*)
		kernel_ahb_end = .;
	} > RamCCM
}
