/*
 * memory definitions
 */

MEMORY
{
	MFlash (rx) : ORIGIN = 0x08000000, LENGTH = 1024K	/* 1024k */
	RamLoc (rwx) : ORIGIN = 0x20000000, LENGTH = 112K	/* 32k */
	RamAHB (rwx) : ORIGIN = 0x2001c000, LENGTH = 64K	/* 32k */	
}
