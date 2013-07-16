/*
 * memory definitions
 */

MEMORY
{
	MFlash (rx) : ORIGIN = 0x08000000, LENGTH = 1024K	/* 1024k */
	RamLoc (rwx) : ORIGIN = 0x20000000, LENGTH = 112K	/* 32k */
	RamCCM (rwx) : ORIGIN = 0x10000000, LENGTH = 64K	/* 64k */	
}
